#ifndef MARKET_DATA_HANDLER_H
#define MARKET_DATA_HANDLER_H

#include "MarketDataFeed.h"
#include "ArbitrageCalculator.h"

class MarketDataHandler {
public:
    MarketDataHandler(bool logResponses, double arbitrageThreshold, int timestampThreshold);
    void run();

private:
    MarketDataFeed krakenFeed;
    MarketDataFeed bybitFeed;
    ArbitrageCalculator arbitrageCalculator;
    
    bool logResponses;
    double arbitrageThreshold;
    int timestampThreshold;
};

#endif
