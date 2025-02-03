#include "MarketDataHandler.h"
#include <iostream>

int main() {
    bool logResponses = true;  // Set to true to enable response logging
    double arbitrageThreshold = 0.002;  // Minimum profitable difference
    int timestampThreshold = 10;  // Maximum allowable timestamp difference (seconds)

    MarketDataHandler handler(logResponses, arbitrageThreshold, timestampThreshold);
    handler.run();

    return 0;
}
