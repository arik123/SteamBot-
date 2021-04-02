//
// Created by Max on 23. 3. 2021.
//

#ifndef STEAMBOT_APIREQUEST_H
#define STEAMBOT_APIREQUEST_H
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <utility>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

namespace asio = boost::asio;    // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using net = boost::asio::ip::tcp;    // from <boost/asio.hpp>
void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
struct ApiRequest {
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    const std::string host;
    std::string endpoint;
    const std::function<void(http::response<http::string_body>)> callback;
    std::function<void()> shutdown_cb;
    ApiRequest(asio::any_io_executor ex, ssl::context& ctx, std::string host, std::string endpoint, http::request<http::empty_body> req, std::function<void(http::response<http::string_body>)>  callback, std::function<void()> shutdown_cb);

    void on_resolve(beast::error_code ec, const net::resolver::results_type& results);

    void on_connect(beast::error_code ec, const net::resolver::results_type::endpoint_type&);

    void on_handshake(beast::error_code ec);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_shutdown(beast::error_code ec);
};


#endif //STEAMBOT_APIREQUEST_H
