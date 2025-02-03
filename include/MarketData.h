#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <string>
#include <cstdint> 

/// @brief structure for storing market trade data.
struct MarketData {
    double price; //trade price
    long long timestamp; //trade timestamp
};

#endif