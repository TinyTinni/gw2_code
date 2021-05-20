cmake_minimum_required(VERSION 3.0)

project(price_gatherer)

set (CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED) # required by Poco >=10  for NetSSL( see https://github.com/pocoproject/poco/issues/3245 )
find_package(Poco REQUIRED Net NetSSL JSON Data DataSQLite)

add_executable(list_prices main.cpp)

target_link_libraries(list_prices Poco::Net Poco::NetSSL Poco::JSON Poco::Data Poco::DataSQLite)

install(TARGETS list_prices RUNTIME DESTINATION bin)
