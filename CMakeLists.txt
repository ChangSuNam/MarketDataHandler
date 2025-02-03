cmake_minimum_required(VERSION 3.20)
project(MarketDataHandler)

set(CMAKE_CXX_STANDARD 20)

# Include the 'include' directory
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/websocketpp
    ${PROJECT_SOURCE_DIR}/third_party
)

# Add source files
add_executable(MarketDataHandler
    src/main.cpp
    src/MarketDataHandler.cpp
    src/ArbitrageCalculator.cpp
    src/MarketDataFeed.cpp
)

# Find and link jsoncpp
find_package(jsoncpp REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIRS})
target_link_libraries(MarketDataHandler PRIVATE jsoncpp_lib)

# Find and link Boost 
find_package(Boost REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS}) 
target_link_libraries(MarketDataHandler PRIVATE ${Boost_LIBRARIES})

# Find and link CURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
target_link_libraries(MarketDataHandler PRIVATE ${CURL_LIBRARIES})

# Find and link OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIRS})
target_link_libraries(MarketDataHandler PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Link ZLIB
find_package(ZLIB REQUIRED)
target_link_libraries(MarketDataHandler PRIVATE ZLIB::ZLIB)