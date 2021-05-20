#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include <Poco/URI.h>
#include <Poco/URIStreamOpener.h>

#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/SQLite.h>
#include <Poco/Data/SQLite/Connector.h>

#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <iomanip>

class Shipment
{
    std::string name;
    std::vector<long> ids;
    std::vector<float> drop_rates;
    std::vector<long> prices;

public:
    struct Profit
    {
        float buys;
        float sells;
    };

    static Shipment fromDB(Poco::Data::Session& db, std::string name)
    {
        using namespace Poco::Data::Keywords;
        Shipment result;
        result.name = std::move(name);
        db << (std::string("SELECT id, drop_rate FROM ") + result.name), into(result.ids), into(result.drop_rates), now;
        return result;
    }

    std::string id_str()
    {
        if (ids.empty())
            return "";
        return std::accumulate(std::begin(ids) + 1, std::end(ids), std::to_string(ids.front()),
            [](std::string lhs, int rhs) {
                std::string r = lhs + "," + std::to_string(rhs);
                return r;
            });
    }

    Profit avg_profit(Poco::JSON::Array::Ptr price_list)
    {
        Profit result = { 0, 0 };
        for (const auto& p : *price_list)
        {
            auto obj = p.extract<Poco::JSON::Object::Ptr>();
            int64_t id = obj->get("id").extract<int64_t>();
            auto it = std::find(ids.begin(), ids.end(), id);
            if (it != ids.end())
            {
                auto dp_id = drop_rates[it - ids.begin()];
                auto price_sell = obj->get("sells").extract<Poco::JSON::Object::Ptr>()->get("unit_price").extract<int64_t>();
                auto price_buy = obj->get("buys").extract<Poco::JSON::Object::Ptr>()->get("unit_price").extract<int64_t>();
                result.buys += dp_id * price_buy;
                result.sells += dp_id * price_sell;
            }
        }

        auto norm = [](auto v) -> decltype(v) {
            // 0.85 for the TP taxes
            // 10000 for the buying price
            // 0.004 to normalize from "per shipment" to "per volatile"
            return decltype(v)(((v * 0.85) - 10000) * 0.004);
        };
        result.buys = norm(result.buys);
        result.sells = norm(result.sells);

        return result;
    }
};

class SSLInitializer
{
public:
    SSLInitializer()
    {
        Poco::Net::initializeSSL();
    }

    ~SSLInitializer()
    {
        Poco::Net::uninitializeSSL();
    }
};

Poco::JSON::Array::Ptr recieve_prices(const std::string& ids_str)
{
    SSLInitializer ssl_init;
    Poco::Net::HTTPSStreamFactory::registerFactory();

    using Poco::Net::ConsoleCertificateHandler;
    using Poco::Net::Context;
    using Poco::Net::InvalidCertificateHandler;
    using Poco::Net::KeyConsoleHandler;
    using Poco::Net::PrivateKeyPassphraseHandler;
    using Poco::Net::SSLManager;

    Poco::SharedPtr<InvalidCertificateHandler> ptrCert = new ConsoleCertificateHandler(false);
    Context::Ptr ptrContext = new Context(Context::CLIENT_USE, "", Context::VERIFY_NONE, 9, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    SSLManager::instance().initializeClient(0, ptrCert, ptrContext);

    Poco::URI uri;
    uri.setScheme("https");
    uri.setHost("api.guildwars2.com");
    uri.setPath("/v2/commerce/prices");
    uri.setQuery("ids=" + ids_str);
    std::unique_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));
    Poco::JSON::Parser parser;
    auto idk = parser.parse(*pStr.get());
    auto r = parser.result();
    auto obj = r.extract<Poco::JSON::Array::Ptr>();
    return obj;
}

int main(int argc, char* argv[])
{

    bool write_into_db = false;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
        {
            std::cout << "v0.1\n";
            std::cout << "Recieves price lists from the gw2 server and calculates profits for volatile shipments.\n";
            std::cout << "It always requires \"volatile.db\" to be accessable.\n";
            std::cout << "USAGE:\n";
            std::cout << "  no parameters\t\tprints out the sells/buys profit per volatile in csv.\n";
            std::cout << "  -db\t\t\twrites the results in the \"volatile.db\" database.\n";
            return 0;
        }
        else if (!strcmp(argv[1], "-db"))
        {
            write_into_db = true;
        }
    }

    try
    {
        Poco::Data::SQLite::Connector::registerConnector();
        Poco::Data::Session db_session("SQLite", "volatile.db");

        auto leather_ship = Shipment::fromDB(db_session, "leather_shipment");
        auto trophy_ship = Shipment::fromDB(db_session, "trophy_shipment");

        auto ids_str = leather_ship.id_str() + "," + trophy_ship.id_str();

        Poco::SharedPtr<Poco::JSON::Array> obj = recieve_prices(ids_str);

        auto profit_leather = leather_ship.avg_profit(obj);
        auto profit_trophy = trophy_ship.avg_profit(obj);

        if (!write_into_db)
        {
            std::cout << "shipment,sells,buys\n"
                << std::setprecision(4);
            std::cout << "leather," << profit_leather.sells << "," << profit_leather.buys << "\n";
            std::cout << "trophy," << profit_trophy.sells << "," << profit_trophy.buys << "\n";
        }
        else
        {
            using namespace Poco::Data::Keywords;
            Poco::Data::Statement stm(db_session);

            stm << "INSERT INTO prices VALUES (strftime('%Y-%m-%d %H %M','now'), ROUND(?,2), ROUND(?,2), ROUND(?,2), ROUND(?,2));", use(profit_leather.sells), use(profit_leather.buys), use(profit_trophy.sells), use(profit_trophy.buys);
            stm.execute();
        }
    }
    catch (Poco::Exception& e)
    {
        std::cerr << e.displayText() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
