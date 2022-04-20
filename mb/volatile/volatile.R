#!/usr/bin/Rscript

######### config #########
db_path <- "volatile.db"

##########################

# returns the net profit per volatile magic for several shipments
# currently only supports leather and trophy shipments, as the others seems stupidly low on profit
volatile_magic_tp_win <- function(){

    library(dplyr)
    library(glue)
    library(DBI)
    library(RSQLite)
    library(jsonlite)

    db <- dbConnect(drv = RSQLite::SQLite(), dbname=db_path)
    leather_df <- tbl(db, "leather_shipment") %>% collect()
    trophy_df <- tbl(db, "trophy_shipment") %>% collect()
    dbDisconnect(db)
    
    df <- union(leather_df, trophy_df)
    
    # get current prices
    ids <- df %>% 
        glue_data("{id}") %>% 
        paste0(collapse=",")
    price_df <- paste0("https://api.guildwars2.com/v2/commerce/prices?ids=",ids) %>% 
        fromJSON() %>% 
        as_tibble()
        
    # calculates netto per shipment
    netto_per_volatile <- function(product_df){
        df <- product_df %>% inner_join(price_df, by="id")
    
        price_per_shipment = c(sum(df[["drop_rate"]] * df[["sells"]][["unit_price"]]), 
                               sum(df[["drop_rate"]] * df[["buys"]][["unit_price"]]))
        netto_per_shipment = price_per_shipment * 0.85
        profit_per_shipment = netto_per_shipment - 10000
        netto_profit_per_volatile = profit_per_shipment * 0.004
        names(netto_profit_per_volatile) <- c("sells", "buys")
        netto_profit_per_volatile
    }
    
    #enframe(shipment=c("leather"),
    leather=append(c(shipment="leather"), netto_per_volatile(leather_df))
    trophy=append(c(shipment="trophy"), netto_per_volatile(trophy_df))
    bind_rows(leather, trophy)
}

volatile_profit <- function(){
    profit <- volatile_magic_tp_win()
    result <- profit %>% arrange(desc(buys))
    result
}

write_to_db <- function(){
    profit <- volatile_magic_tp_win()

    # arrange values for DB
    # leather first, then trophy
    #leather_prices %>% filter(shipment="leather") %>% glue_data("{buys},{sells}")
    value_vec <- profit %>% glue_data("{buys},{sells}",) %>% paste0(collapse=",")

    db <- dbConnect(drv = RSQLite::SQLite(), dbname=db_path)
    dbExecute(db, paste0("INSERT INTO prices VALUES (strftime('%Y-%m-%d %H %M','now'),", value_vec, ");") )
    dbDisconnect(db)
}

write_to_db()
