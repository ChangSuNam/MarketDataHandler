#ifndef ARBITRAGE_CALCULATOR_H
#define ARBITRAGE_CALCULATOR_H

#include "MarketData.h"

class ArbitrageCalculator {
public:
    static void calculateArbitrage(const std::shared_ptr<MarketData>& krakenData, const std::shared_ptr<MarketData>& bybitData,  double threshold);
};

#endif
