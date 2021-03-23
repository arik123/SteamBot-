//
// Created by Max on 23. 3. 2021.
//

#ifndef STEAMBOT_APIREQUEST_H
#define STEAMBOT_APIREQUEST_H
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

namespace asio = boost::asio;    // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using net = boost::asio::ip::tcp;    // from <boost/asio.hpp>

struct ApiRequest {
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    const char * host;
    const char * endpoint;
    const std::function<void(http::response<http::string_body>)>& callback;
    ApiRequest(asio::any_io_executor ex, ssl::context &ctx, const char * host, const char * endpoint, const std::function<void(http::response<http::string_body>)>& callback)
    : stream_(ex, ctx), host(host), endpoint(endpoint), callback(callback){
    }

    void on_resolve(beast::error_code ec, const net::resolver::results_type& results) {
        if (ec)
            return fail(ec, "resolve");

        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }
        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream_).async_connect(results,
                                                       [&](beast::error_code ec, net::resolver::results_type::endpoint_type et){on_connect(ec, std::move(et));});
    }

    void on_connect(beast::error_code ec, net::resolver::results_type::endpoint_type) {
        if (ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(ssl::stream_base::client,[&](beast::error_code ec){on_handshake(ec);});
    }

    void on_handshake(beast::error_code ec) {
        if (ec)
            return fail(ec, "handshake");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        http::async_write(stream_, req_, [&](beast::error_code ec, std::size_t bt){on_write(ec, bt);});
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Receive the HTTP response
        http::async_read(stream_, buffer_, res_, [&](beast::error_code ec, std::size_t bt){on_read(ec, bt);});
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "read");

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Gracefully close the stream
        stream_.async_shutdown([&](beast::error_code ec){on_shutdown(ec);});
    }

    void on_shutdown(beast::error_code ec) {
        if (ec == asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if (ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};


#endif //STEAMBOT_APIREQUEST_H
