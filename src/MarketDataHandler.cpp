#include "MarketDataHandler.h"
#include <thread>
#include <chrono>
#include <iostream>

/// @brief Initializes data feeds.
/// @param logResponses set to true to enable response logging
/// @param arbitrageThreshold set the minimum profitable difference
/// @param timestampThreshold  set the maximum allowable timestamp difference (seconds)
MarketDataHandler::MarketDataHandler(bool logResponses, double arbitrageThreshold, int timestampThreshold)
    :logResponses(logResponses), arbitrageThreshold(arbitrageThreshold), timestampThreshold(timestampThreshold) {
    krakenFeed.setLogResponses(logResponses);
    bybitFeed.setLogResponses(logResponses);
}

/// @brief Continuously fetches and processes market data.
void MarketDataHandler::run() {
    krakenFeed.start("wss://ws.kraken.com");
    bybitFeed.start("wss://stream.bybit.com/v5/public/linear");

    while (true) {
        std::shared_ptr<MarketData> krakenData = krakenFeed.getLatestData();
        std::shared_ptr<MarketData> bybitData = bybitFeed.getLatestData();
        std::cout << "Kraken Data Address: " << krakenData.get() << ", Bybit Data Address: " << bybitData.get() << std::endl;
        if (!krakenData || !bybitData) {
            if (logResponses) {
                std::cerr << "Error: One or both market data sources are unavailable." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        long long timestampDiff = std::abs(krakenData->timestamp - bybitData->timestamp);
        if (logResponses) {
            std::cout << "Timestamp Difference: " << timestampDiff  << " | Threshold: " << timestampThreshold << std::endl;
            std::cout << "Kraken TS: " << krakenData->timestamp << ", Bybit TS: " << bybitData->timestamp << std::endl;
            std::cout << "Kraken Price: " << krakenData->price << ", Bybit Price: " << bybitData->price << std::endl;
        }
        if (timestampDiff <= timestampThreshold) {
            if (logResponses) {
                std::cout << "Starting Arbitrage Calculation"<< std::endl;
            }
            arbitrageCalculator.calculateArbitrage(krakenData, bybitData, arbitrageThreshold);
        } else if (logResponses) {
            std::cout << "Skipping arbitrage - timestamp difference (" << timestampDiff  << "s) exceeds threshold." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
