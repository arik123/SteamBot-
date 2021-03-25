//
// Created by Max on 23. 3. 2021.
//

#ifndef STEAMBOT_STEAMAPI_H
#define STEAMBOT_STEAMAPI_H
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <utility>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include "ApiRequest.h"

namespace asio = boost::asio;    // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using net = boost::asio::ip::tcp;    // from <boost/asio.hpp>

class SteamApi {
    net::resolver resolver_;
    asio::any_io_executor ex;
    ssl::context &ctx;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    const std::string host;
    ApiRequest * p_apiRequest = nullptr;
public:
    SteamApi(const asio::any_io_executor& ex, ssl::context &ctx, std::string host)
    : resolver_(ex), ex(ex), ctx(ctx), host(std::move(host)){
    }
    static std::string urlEncode(const std::string &SRC) {
        std::string ret;
        ret.reserve(SRC.size());
        for (std::string::const_iterator iter = SRC.begin(); iter != SRC.end(); ++iter) {
            std::string::value_type c = (*iter);

            // Keep alphanumeric and other accepted characters intact
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                ret += c;
                continue;
            }

            // Any other characters are percent-encoded
            char buff[4];
            sprintf(buff, "%%%02X", c);
            ret += buff;
        }
        return ret;
    }
    // Start the asynchronous operation
    void request(char const *interface, char const *method, char const *version, bool post, const std::unordered_map<std::string, std::string>& data, const std::function<void(http::response<http::string_body>)>& callback ) {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        std::string endpoint = "/";
        endpoint += interface;
        endpoint += '/';
        endpoint += method;
        endpoint += '/';
        endpoint += version;
        if(!data.empty()) {
            endpoint += '?';
            auto iter = data.begin();
            while(true){
                endpoint += urlEncode(iter->first);
                endpoint += '=';
                endpoint += urlEncode(iter->second);
                iter++;
                if(iter == data.end()) break;
                endpoint += '&';
            }
        }
        // Set up an HTTP GET request message
        req_.method(post ? http::verb::post : http::verb::get);
        req_.target(endpoint.c_str());
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(http::field::accept, "*/*");
        req_.version(10);
        p_apiRequest = new ApiRequest(ex, ctx, host, endpoint, req_, callback, [&]() {shutdown(); });
        // Look up the domain name
        resolver_.async_resolve(host, "443", [&](beast::error_code ec, const net::resolver::results_type& results){p_apiRequest->on_resolve(ec, results);});
    }
	void shutdown()
    {
        delete this->p_apiRequest;
    }
};


#endif //STEAMBOT_STEAMAPI_H
