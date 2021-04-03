//
// Created by Max on 2. 4. 2021.
//

#include "SteamCommunity.h"
#include "utils.h"
SteamCommunity::SteamCommunity(const asio::any_io_executor& ex, ssl::context& ctx, std::string host)
    : resolver_(ex), ex(ex), ctx(ctx), host(std::move(host)) {
}

void SteamCommunity::request(std::string endpoint, bool post,
                             const std::unordered_map <std::string, std::string>& data,
                             const std::function<void(http::response < http::string_body >)>& callback) {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!data.empty() && !post) {
        endpoint += '?';
        auto iter = data.begin();
        while (true) {
            endpoint += urlEncode(iter->first);
            endpoint += '=';
            endpoint += urlEncode(iter->second);
            iter++;
            if (iter == data.end()) break;
            endpoint += '&';
        }
    } else if (!data.empty() && post){

    }
    // Set up an HTTP GET request message
    req_.method(post ? http::verb::post : http::verb::get);
    req_.target(endpoint.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(http::field::accept, "*/*");
    //req_.insert()
    req_.version(11);
    p_apiRequest = new WebRequest(ex, ctx, host, endpoint, req_, callback, [&]() {shutdown(); });
    // Look up the domain name
    resolver_.async_resolve(host, "443", [&](beast::error_code ec, const net::resolver::results_type& results) {p_apiRequest->on_resolve(ec, results); });
}
void SteamCommunity::shutdown() {
    delete this->p_apiRequest;
}
