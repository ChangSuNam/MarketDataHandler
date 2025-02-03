#include "MarketDataFeed.h"
#include "MarketData.h"
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <json/json.h>
#include <atomic>
#include <zlib.h> 
#include <sstream>
#include <json/json.h>
#include<memory>

using WebSocketClient = websocketpp::client<websocketpp::config::asio_tls_client>;

std::atomic<bool> isConnected(false);


size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}


std::string fetchTradableAssetPairs() {
    static bool curlInitialized = false;
    if (!curlInitialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        curlInitialized = true;
    }

    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "https://api.kraken.com/0/public/AssetPairs";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

std::string getWsnameForPair(const std::string& response, const std::string& pair) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;

    std::istringstream s(response);
    if (!Json::parseFromStream(reader, s, &root, &errors)) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        return "";
    }

    if (!root.isMember("result")) {
        std::cerr << "Unexpected Kraken API response. 'result' field missing" << std::endl;
        return "";
    }

    for (const auto& key : root["result"].getMemberNames()) {
        std::string wsname = root["result"][key]["wsname"].asString();
        if (wsname == pair) {
            return wsname;
        }
    }

    std::cerr << "Pair " << pair << " not found! Available pairs from Kraken:\n";
    for (const auto& key : root["result"].getMemberNames()) {
        std::cerr << " - " << root["result"][key]["wsname"].asString() << std::endl;
    }

    return "";
}

/// @brief Initiates a WebSocket connection.
/// @param url the  URL to connect to
void MarketDataFeed::start(const std::string& url) {
    // Run  websockets in separate thread
    std::thread([this, url]() { connectToFeed(url); }).detach();
}

/// @brief Manages WebSocket lifecycle.
/// @param url the URL to connect to
void MarketDataFeed::connectToFeed(const std::string& url) {
    WebSocketClient client;
    client.init_asio();

    client.set_tls_init_handler([](websocketpp::connection_hdl) {
        auto ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::tlsv12_client
        );
        return ctx;
    });

    client.set_open_handler([this, &client, url](websocketpp::connection_hdl hdl) {
    std::cout << "Connected to url: " << url << std::endl;
    isConnected = true;
    
    //Send subscription message
    if (url.find("kraken") != std::string::npos) {
        std::string response = fetchTradableAssetPairs();
        std::string wsname = getWsnameForPair(response, "XBT/USDT");
        if (!wsname.empty()) {
            std::string subscribe_msg = R"({"event":"subscribe","pair":[")" + wsname + R"("],"subscription":{"name":"trade"}})";
            client.send(hdl, subscribe_msg, websocketpp::frame::opcode::text);
            std::cout << "Sent Kraken subscription: " << subscribe_msg << std::endl;
        }
    } else if (url.find("bybit") != std::string::npos) {
        std::string subscribe_msg = R"({"op": "subscribe", "args": ["publicTrade.BTCUSDT"]})";
        client.send(hdl, subscribe_msg, websocketpp::frame::opcode::text);
        std::cout << "Sent Bybit subscription: " << subscribe_msg << std::endl;
    }

    // Start ping thread
    std::thread([&client, hdl, url]() {
        while (isConnected) {
            std::this_thread::sleep_for(std::chrono::seconds(18));
            if (client.get_con_from_hdl(hdl)->get_state() == websocketpp::session::state::open) {
                std::string ping_msg = (url.find("kraken.com") != std::string::npos) ?
                    R"({"event":"ping"})" : R"({"op": "ping"})";
                client.get_con_from_hdl(hdl)->send(ping_msg, websocketpp::frame::opcode::text);
                std::cout << "Sent ping to server. url: " + url << std::endl;
            }
        }
    }).detach();
    });
 
    client.set_message_handler([this, url](websocketpp::connection_hdl, WebSocketClient::message_ptr msg) {
        std::string payload = msg->get_payload();
        parseMessage(payload);
    });

    client.set_fail_handler([this, url](websocketpp::connection_hdl hdl) {
        std::cerr << "Connection failed. Retrying in 5 seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        connectToFeed(url);
    });

    client.set_close_handler([](websocketpp::connection_hdl) {
        std::cerr << "WebSocket connection closed" << std::endl;
        isConnected = false;
    });

    websocketpp::lib::error_code ec;
    auto connection = client.get_connection(url, ec);
    if (ec) {
        std::cerr << "Connection error: " << ec.message() << std::endl;
        return;
    }

    std::cout << "Connecting to: " << url << std::endl;
    client.connect(connection);
    client.run();
}

/// @brief Parses market data messages.
/// @param message response message from the WebSocket server
void MarketDataFeed::parseMessage(const std::string& message) {
    std::string parsedMessage = message;

    if (static_cast<unsigned char>(message[0]) == 0x1F && static_cast<unsigned char>(message[1]) == 0x8B) { 
        parsedMessage = decompressMessage(message);
    }
    if (logResponses) {
        std::cout << "Received Message: " << parsedMessage << std::endl;
    }

    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream s(parsedMessage);

    if (!Json::parseFromStream(reader, s, &root, &errors)) {
        std::cerr << "Error parsing JSON: " << errors << std::endl;
        return;
    }

    auto data = std::make_shared<MarketData>();

    // Handle Bybit response
    if (root.isObject() && root.isMember("topic") && root["topic"].asString().find("publicTrade") != std::string::npos) {
        auto tradeData = root["data"];
        if (tradeData.isArray() && !tradeData.empty()) {
            try {
                data->price = std::stod(tradeData[0]["p"].asString());
                data->timestamp = tradeData[0]["T"].asLargestInt() / 1000;  // Convert ms →toseconds
            } catch (const std::exception& e) {
                std::cerr << "Error parsing Bybit trade data: " << e.what() << std::endl;
                return;
            }
        }
    }

    // Hande Kraken response
    else if (root.isArray() && root.size() >= 4 && root[2].asString() == "trade") {
        auto tradeList = root[1];  // The second array contains trades
        if (tradeList.isArray() && !tradeList.empty() && tradeList[0].isArray()) {
            try {
                data->price = std::stod(tradeList[0][0].asString());
                data->timestamp = static_cast<long long>(std::stod(tradeList[0][2].asString())); // Already in seconds
            } catch (const std::exception& e) {
                std::cerr << "Error parsing Kraken trade data: " << e.what() << std::endl;
                return;
            }
        }
    } else {
        std::cout << "Message does not contain response with subscribed information: " << message << std::endl;
        return;
    }

    updateLatestData(data);
}


std::string MarketDataFeed::decompressMessage(const std::string& compressedData) {
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
        std::cerr << "Failed to initialize zlib for decompression" << std::endl;
        return "";
    }

    strm.next_in = (Bytef*)compressedData.data();
    strm.avail_in = compressedData.size();

    char buffer[8192]; 
    std::string decompressedData;

    do {
        strm.next_out = (Bytef*)buffer;
        strm.avail_out = sizeof(buffer);

        int ret = inflate(&strm, Z_SYNC_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            std::cerr << "Zlib decompression error: " << ret << std::endl;
            inflateEnd(&strm);
            return "";
        }

        decompressedData.append(buffer, sizeof(buffer) - strm.avail_out);
    } while (strm.avail_out == 0);

    inflateEnd(&strm);
    return decompressedData;
}

void MarketDataFeed::setLogResponses(bool enable) {
    logResponses = enable;
}

/// @brief Retrieves the latest trade data.
/// @return the latest trade data
std::shared_ptr<MarketData> MarketDataFeed::getLatestData() {
        return std::atomic_load(&latestDataPtr);
}
/// @brief Atomically updates trade data.
/// @param newData the new trade data
void MarketDataFeed::updateLatestData(std::shared_ptr<MarketData> newData) {
        std::atomic_store(&latestDataPtr, newData);
    }

