#include "ArbitrageCalculator.h"
#include <iostream>
#include <iomanip>
#include <cmath>

/// @brief Determines if an arbitrage opportunity exists.
/// @param krakenData the latest Kraken market data
/// @param bybitData the latest Bybit market data
/// @param threshold the minimum profitable difference
void ArbitrageCalculator::calculateArbitrage(const std::shared_ptr<MarketData>& krakenData, const std::shared_ptr<MarketData>& bybitData, double threshold) {
    if (!krakenData || !bybitData) {
        std::cerr << "Null MarketData received. Skipping arbitrage calculation." << std::endl;
        return;
    }

    double krakenPrice = krakenData->price;
    double bybitPrice = bybitData->price;

    if (krakenPrice <= 0 || bybitPrice <= 0) {
        std::cerr << "[ARBITRAGE] Invalid price data. Skipping arbitrage calculation." << std::endl;
        return;
    }

    constexpr double krakenFeeRate = 0.001;  // 0.1% fee for spot trade
    constexpr double bybitFeeRate = 0.001;   // 0.1% fee for spot ttrade

    // Determine buy/sell
    double buyPrice, sellPrice, buyFeeRate, sellFeeRate;
    if (krakenPrice < bybitPrice) {
        buyPrice = krakenPrice;
        sellPrice = bybitPrice;
        buyFeeRate = krakenFeeRate;
        sellFeeRate = bybitFeeRate;
    } else {
        buyPrice = bybitPrice;
        sellPrice = krakenPrice;
        buyFeeRate = bybitFeeRate;
        sellFeeRate = krakenFeeRate;
    }

    double feeBuy = buyPrice * buyFeeRate;
    double feeSell = sellPrice * sellFeeRate;
    double netProfit = (sellPrice - buyPrice) - (feeBuy + feeSell);
    double adjustedDifferencePercentage = (netProfit / buyPrice);

    std::cout << "[ARBITRAGE] Buy at: " << buyPrice  << " | Sell at: " << sellPrice << std::endl;
    std::cout << "[ARBITRAGE] Price Difference: " << (sellPrice - buyPrice) << " | Fee Buy: " << feeBuy << " | Fee Sell: " << feeSell << std::endl;
    std::cout << "[ARBITRAGE] Adjusted Difference %: " << adjustedDifferencePercentage * 100  << " | Threshold: " << threshold * 100 << "%" << std::endl;

    if (adjustedDifferencePercentage > threshold) {
        std::cout << "\n******** Arbitrage Opportunity ********" << std::endl;
        std::cout << "Buy at $" << std::fixed << std::setprecision(2) << buyPrice << " | Sell at $" << sellPrice << std::endl;
        std::cout << "Net Profit: $" << netProfit << std::endl;
        std::cout << "Adjusted for Fees (percentage): " << adjustedDifferencePercentage * 100 << "%" << std::endl;
        std::cout << "****************************************\n" << std::endl;
    } else {
        std::cout << "[ARBITRAGE] No arbitrage opportunity found." << std::endl;
    }
}
