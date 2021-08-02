//
// Created by Max on 23. 3. 2021.
//

#include "WebRequest.h"

#include <utility>
#include "utils.h"

void WebRequest::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "read");

    callback(res_);

    // Set a timeout on the operation
    beast::get_lowest_layer(sslStream_).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream
    sslStream_.async_shutdown([&](beast::error_code ec){on_shutdown(ec);});
}

void WebRequest::on_shutdown(beast::error_code ec) {
    if (ec == asio::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if (ec)
        return fail(ec, "shutdown");

    // If we get here then the connection is closed gracefully
    shutdown_cb(this);
}

void WebRequest::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Receive the HTTP response
    http::async_read(sslStream_, buffer_, res_, [&](beast::error_code ec, std::size_t bt){on_read(ec, bt);});
}

void WebRequest::on_handshake(beast::error_code ec) {
    if (ec)
        return fail(ec, "handshake");

    // Set a timeout on the operation
    beast::get_lowest_layer(sslStream_).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(sslStream_, req_, [&](beast::error_code ec, std::size_t bt){on_write(ec, bt);});
}

void WebRequest::on_connect(beast::error_code ec, const asio::ip::basic_endpoint<asio::ip::tcp> &) {
    if (ec)
        return fail(ec, "connect");

    // Perform the SSL handshake
    if(ssl) sslStream_.async_handshake(ssl::stream_base::client, [&](beast::error_code ec){on_handshake(ec);});
    else on_handshake(ec);
}

void WebRequest::on_resolve(beast::error_code ec,
                            const asio::ip::basic_resolver<asio::ip::tcp, asio::any_io_executor>::results_type &results) {
    if (ec)
        return fail(ec, "resolve");
    if(ssl){
        if (!SSL_set_tlsext_host_name(sslStream_.native_handle(), host.c_str())) {
            beast::error_code ec2{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
            std::cerr << ec2.message() << "\n";
            return;
        }
        usedStream_ = &beast::get_lowest_layer(sslStream_);
    } else {
        usedStream_ = &stream_;
    }
    // Set a timeout on the operation
    beast::get_lowest_layer(sslStream_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(sslStream_).async_connect(results,
                                                      [&](beast::error_code ec, const net::resolver::results_type::endpoint_type& et){on_connect(ec, et);});
}

WebRequest::WebRequest(asio::any_io_executor ex, ssl::context &ctx, std::string host, std::string endpoint,
                       http::request<http::string_body> req,
                       std::function<void(http::response<http::string_body>&)>  callback,
                       std::function<void(WebRequest*)> shutdown_cb,
                       bool ssl)
        : sslStream_(ex, ctx), stream_(ex), host(std::move(host)), endpoint(std::move(endpoint)), callback(std::move(callback)), shutdown_cb(std::move(shutdown_cb)), ssl(ssl){
    req_ = std::move(req);
}
