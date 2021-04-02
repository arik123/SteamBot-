//
// Created by Max on 23. 3. 2021.
//

#include "SteamApi.h"

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
                       const std::unordered_map <std::string, std::string> &data,
                       const std::function<void(http::response < http::string_body > )> &callback) {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    std::string endpoint = "/";
    endpoint += interface;
    endpoint += '/';
    endpoint += method;
    endpoint += '/';
    endpoint += version;
    if (!data.empty()) {
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
    resolver_.async_resolve(host, "443", [&](beast::error_code ec, const net::resolver::results_type& results) {p_apiRequest->on_resolve(ec, results); });
}

std::string SteamApi::urlEncode(const std::string &SRC) {
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

SteamApi::SteamApi(const asio::any_io_executor &ex, ssl::context &ctx, std::string host)
        : resolver_(ex), ex(ex), ctx(ctx), host(std::move(host)) {
}

void SteamApi::shutdown() {
    delete this->p_apiRequest;
}
