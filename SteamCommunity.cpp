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
							 std::string referer,
                             const std::function<void(http::response < http::string_body >&)>& callback) {
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
        std::string formData;
        auto iter = data.begin();
        while (true) {
            formData += urlEncode(iter->first);
            formData += '=';
            formData += urlEncode(iter->second);
            iter++;
            if (iter == data.end()) break;
            formData += '&';
        }
        req_.body() = formData;
    }
    // Set up an HTTP GET request message
    req_.method(post ? http::verb::post : http::verb::get);
    req_.target(endpoint.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(http::field::accept, "*/*");
	if(referer.length())
	{
        req_.set(http::field::referer, referer);
	}
    //req_.insert()
    req_.version(11);
    p_apiRequest = new WebRequest(ex, ctx, host, endpoint, req_, callback, [&]() {shutdown(); });
    // Look up the domain name
    resolver_.async_resolve(host, "443", [&](beast::error_code ec, const net::resolver::results_type& results) {p_apiRequest->on_resolve(ec, results); });
}
void SteamCommunity::shutdown() {
    delete this->p_apiRequest;
}
void SteamCommunity::getUserInventory(uint64_t steamid, uint32_t appID, uint32_t contextID, const std::function<void()>& callback, const std::string& language, const std::string& start)
{
    std::string endpoint("/inventory/");
    endpoint += std::to_string(steamid);
    endpoint += '/';
    endpoint += std::to_string(appID);
    endpoint += '/';
    endpoint += std::to_string(contextID);
    std::unordered_map <std::string, std::string> params = {{"l", language}, {"count", "5000"}};
    if(start.length()) {
        params.insert({"start_assetid", start});
    }
    request(endpoint,
            false, params,
            "",
            [callback](http::response < http::string_body >& resp) {
            std::cout << "inventory responded\n";
            if (resp[http::field::content_type].starts_with("application/json"))
            {
                rapidjson::Document document;
                document.Parse(resp.body().data());
                std::cout << "Success " << document["success"].GetInt();
                std::cout << ", Count " << document["total_inventory_count"].GetInt() << '\n';
                //TODO: finish parsing
            }
            else
            {
                callback();
            }
            callback();
        });
}