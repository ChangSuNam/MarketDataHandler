#ifndef MARKET_DATA_FEED_H
#define MARKET_DATA_FEED_H

#include <string>
#include <optional>
#include <mutex>
#include "MarketData.h"
#include "ArbitrageCalculator.h"


class MarketDataFeed {
public:
    void start(const std::string& url);
    void setLogResponses(bool enable) ;
    std::shared_ptr<MarketData> getLatestData();
    void updateLatestData(std::shared_ptr<MarketData> newData) ;
    std::shared_ptr<MarketData> latestDataPtr;  

private:
    void connectToFeed(const std::string& url);
    void parseMessage(const std::string& message);
    std::string decompressMessage(const std::string& compressedData);
    bool logResponses = false; // Default not verbose
};

#endif