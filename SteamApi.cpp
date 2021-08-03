//
// Created by Max on 23. 3. 2021.
//

#include "SteamApi.h"
#include "utils.h"

void SteamApi::GetCMList(const std::string &cellid, const std::function<void(std::vector < net::endpoint > )> &callback) {
    request("ISteamDirectory", "GetCMList", "v1", false,
            { {"cellid", cellid} },
            [=](http::response<http::string_body>& resp) {
                if (resp[http::field::content_type].starts_with("application/json"))
                {
                    rapidjson::Document document;
                    document.ParseInsitu(resp.body().data());
                    std::vector<net::endpoint> serverList;
                    serverList.reserve(document["response"]["serverlist"].GetArray().Size());
                    for (auto& v : document["response"]["serverlist"].GetArray())
                    {
                        std::string server(v.GetString(), v.GetStringLength());
                        serverList.emplace_back(asio::ip::address::from_string(server.substr(0, server.find(':'))), std::stoi(server.substr(server.find(':')+1)));
                    }
                    callback(serverList);
                }
                else
                {
                    callback({});
                }
                //std::cout << resp.body() << std::endl;
            });
}

void SteamApi::request(const char *interface, const char *method, const char *version, bool post,
                       const std::unordered_map <std::string, std::variant<std::string, std::vector<uint8_t>>> &data,
                       const std::function<void(http::response < http::string_body >&)> &callback) {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    std::string endpoint = "/";
    endpoint += interface;
    endpoint += '/';
    endpoint += method;
    endpoint += '/';
    endpoint += version;
    if(!data.empty()){
        auto iter = data.begin();
        std::string formData;
        while (true) {
            formData += urlEncode(iter->first);
            formData += '=';
            if(std::get_if<std::string>(&(iter->second))) {
                formData += urlEncode(std::get<std::string>(iter->second));
            } else {
                formData += urlEncode(std::get<std::vector<uint8_t>>(iter->second));
            }
            iter++;
            if (iter == data.end()) break;
            formData += '&';
        }
        if(post) {
            req_.body() = formData;
            req_.set(http::field::content_type, "application/x-www-form-urlencoded");
        } else {
            endpoint += '?';
            endpoint += formData;
        }
    }
    // Set up an HTTP GET request message
    req_.method(post ? http::verb::post : http::verb::get);
    req_.target(endpoint.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(http::field::accept, "*/*");
    req_.version(11);
    req_.prepare_payload();
    auto p_apiRequest = new WebRequest(ex, ctx, host, endpoint, req_, callback, [](WebRequest* ptr) {shutdown(ptr); });
    // Look up the domain name
    resolver_.async_resolve(host, "443", [p_apiRequest](beast::error_code ec, const net::resolver::results_type& results) {p_apiRequest->on_resolve(ec, results); });
}

SteamApi::SteamApi(const asio::any_io_executor &ex, ssl::context &ctx, std::string host)
        : resolver_(ex), ex(ex), ctx(ctx), host(std::move(host)) {
}

void SteamApi::shutdown(WebRequest * ptr) {
    delete ptr;
}
