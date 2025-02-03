# Overview

  Market data processing system built in C++. Retrieves real-time data via WebSocket, detects arbitrage opportunities with low-latency processing.

# Key Features

  Real-Time WebSocket Data Handling: Use websocketpp and boost::asio to manage connections to Kraken and Bybit exchange.
  
  Low-Latency Parsing: Optimized JSON parsing using jsoncpp with efficient memory allocation.
  
  Multithreaded Processing: Producer-consumer model for concurrent data processing.
  
  Lock-Free Data Management: Utilize std::atomic<std::shared_ptr<T>> for non-blocking updates to market data.
  
  Arbitrage Calculation: Compute arbitrage opportunities with fee considerations and configurable thresholds.

# Project Structure

1. main.cpp

  Initializes market data feed handler.

2. MarketDataHandler.cpp

  Manages data feeds, synchronizes market data updates, and invokes arbitrage calculations.

3. MarketDataFeed.cpp

  Handles WebSocket connections and processes/parse incoming trade messages. Atomically updates trade data.

4. ArbitrageCalculator.cpp

  Computes arbitrage opportunities based on price differences and trading fees.

# Design Patterns and Optimizations
  
  Singleton Pattern: Ensures a single instance of market data feeds per exchange.
  
  Observer Pattern: MarketDataHandler observes MarketDataFeed for updates.
  
  Lock-Free Design: Uses std::atomic<std::shared_ptr<T>> for non-blocking updates.
  
  Compression Handling: Supports gzip decompression for WebSocket messages.

# Future improvements

  -Memory Pool for JSON Parsing, reducing overheads
  -Orderbook including bid/ask

# How to Run

Requirements:
  
  C++, boost::asio, websocketpp, jsoncpp, ZLIb, CURL, OpenSSL, CMake
  
  Create a third_party folder and install websocketpp.

Run the following to create a build directory, compile, link and run the project:

  mkdir build
  cd build
  cmake ..
  make
  ./market_data_handler
