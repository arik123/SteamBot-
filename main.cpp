#include <iostream>
#include <fstream>
#include <functional>
#include <thread>

#include "lib/SteamPP/steam++.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/streambuf.hpp>

#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <openssl/hmac.h>

using net = boost::asio::ip::tcp;    // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;    // from <boost/asio.hpp>

std::string read_buffer;
std::string write_buffer;
net::socket * sock;
std::thread steamPoller;
asio::io_context ioc;

bool running = true;

extern Steam::SteamClient client;


void readHandle(boost::system::error_code error, std::size_t bytes_transferred){
    if (!error)
    read_buffer.resize(client.readable(reinterpret_cast<const unsigned char*>(read_buffer.data())));
    sock->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()), readHandle);
};
void writeHandle(boost::system::error_code error, std::size_t bytes_transferred){
};
Steam::SteamClient client(
        // write callback
        [](std::size_t length, std::function<void(unsigned char* buffer)> fill) {
            // TODO: check if previous write has finished
            write_buffer.resize(length);
            fill(reinterpret_cast<unsigned char*>(&write_buffer[0]));
            sock->async_write_some(asio::buffer(write_buffer.c_str(), write_buffer.size()), writeHandle);
        },
        // set_inverval callback
        [](std::function<void()> callback, int timeout) {
            auto callback_heap = new std::function<void()>(std::move(callback));
            steamPoller = std::thread([](std::function<void()> * callback, int timeout){;
                while (running) {
                    (*callback)();
                    std::this_thread::sleep_for(std::chrono::seconds (timeout));
                }
                delete callback;
            }, callback_heap, timeout);
        }
);
bool isLittleEndian()
{
    short int number = 0x1;
    char *numPtr = (char*)&number;
    return (numPtr[0] == 1);
}

std::string generateAuthCode(const std::string &secret) {
    using namespace boost::beast::detail;
    std::vector<uint8_t> decoded;
    decoded.resize(base64::decoded_size(secret.size()));
    base64::decode(decoded.data(), secret.c_str(), secret.size());
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::chrono::system_clock::duration dtn = tp.time_since_epoch();
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(dtn).count();
    now /= 1000;
    now /= 30;
    if(isLittleEndian()) {
        /* swap the bytes */
        now = ((now&0xFF)<<56) | (((now>>8)&0xFF)<<48) |   (((now>>16)&0xFF)<<40) | (((now>>24)&0xFF)<<32) | (((now>>32)&0xFF)<<24) | (((now>>40)&0xFF)<<16) |   (((now>>48)&0xFF)<<8) | (((now>>56)&0xFF));
    }
    std::vector<uint8_t> out (EVP_MAX_MD_SIZE);
    uint32_t md_size;
    HMAC(EVP_sha1(), decoded.data(), decoded.size()-1, reinterpret_cast<uint8_t*>(&now), sizeof (now), out.data(), &md_size);
    out.resize(md_size);
    uint32_t partBuff = *reinterpret_cast<uint32_t *>(out.data() + (out[19] & 0xf));
    int b = out[19] & 0xF;
    int codePoint = (out[b] & 0x7F) << 24 | (out[b + 1] & 0xFF) << 16 | (out[b + 2] & 0xFF) << 8 | (out[b + 3] & 0xFF);
    const char * chars = "23456789BCDFGHJKMNPQRTVWXY";
    std::string code = "";
    for (int i = 0; i < 5; i++) {
        code += chars[codePoint % strlen(chars)];
        codePoint /= strlen(chars);
    }
    return code;
}

std::unordered_map<std::string, std::string> loadenv(char * envp[]) {
    std::unordered_map<std::string, std::string> env;
    for (auto ienv = envp; *ienv != nullptr; ienv++)
    {
        std::string line = *ienv;
        int i = line.find('=');
        if( i >= 0) {
            env.insert({line.substr(0,i), std::string(line.c_str()+i+1)});
        }
    }
    std::ifstream dotEnv(".env");
    while(!dotEnv.eof()) {
        std::string line;
        char linePart [32];
        do {
            dotEnv.clear();
            dotEnv.getline(linePart, 32);
            line += linePart;
        } while (!dotEnv.eof() && !dotEnv.bad() && dotEnv.fail());
        int i = line.find('=');
        if( i >= 0) {
            if(line[line.length()-1] == '\r') line.resize(line.length()-1);
            env.insert({line.substr(0,i), std::string(line.c_str()+i+1)});
        }
    }
    dotEnv.close();
    return env;
}
int main(int argc, char** argv, char* envp[]) {
//    while(true) {
//        std::string code = generateAuthCode("");
//        std::cout << code << '\n';
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//    }
    auto env = loadenv(envp);
    std::ifstream sentryIF("sentry.txt");
    std::string sentry;
    if (sentryIF.is_open())
    {
        sentryIF >> sentry;
        sentryIF.close();
    }
	
    try
    {
        // The io_context is required for all I/O

        // The SSL context is required, and holds certificates
        ssl::context ctx(ssl::context::tlsv12_client);

        // This holds the root certificate used for verification
        ctx.set_verify_mode(ssl::context::verify_peer |
            ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);

        net::resolver resolver(ioc);
        net::endpoint endp(asio::ip::address_v4(0xA2FEC683), 27017); // TODO: https://api.steampowered.com/ISteamDirectory/GetCMList/v1/?cellid=0
        sock = new net::socket(ioc);
        sock->connect(endp);
        read_buffer.resize(client.connected());

        sock->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()), readHandle);
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    client.onHandshake = [&] {
        std::cout << "logging in \n";
        std::string code = generateAuthCode(env.at("STEAM_SHARED_SECRET"));
        const uint8_t* p_sentry = (sentry.length() > 0) ? reinterpret_cast<const uint8_t *>(sentry.c_str()) : nullptr;
        if(p_sentry != nullptr) std::cout << "using sentry\n";
        client.LogOn(env.at("STEAM_USERNAME").c_str(), env.at("STEAM_PASSWORD").c_str(), p_sentry, code.data());
    };
    client.onRelationships = [](bool incremental, std::map<Steam::SteamID, Steam::EFriendRelationship> &users, std::map<Steam::SteamID, Steam::EClanRelationship> &groups){
        std::cout << "a\n";
    };

    client.onSentry = [](const uint8_t sentry [20])
    {
        std::ofstream myfile("sentry.txt");
    	if(!myfile.is_open())
    	{
            std::cout << "could not save sentry\n";
            return;
    	}
        myfile << sentry;
        myfile.close();
    };

    client.onLogOn = [](Steam::EResult result, Steam::SteamID steamID) {
        switch(result) {
            case Steam::EResult::OK:
                std::cout << "logged on!" << std::endl;
                client.SetPersonaState(Steam::EPersonaState::Online);
                //client.SetGamePlayed(440);
                client.SetGamePlayed("Hello, I'm alive, using C++");
                client.SendPrivateMessage(Steam::SteamID(76561198162885342), "I shall live once again.");
                break;
            case Steam::EResult::InvalidPassword:
                std::cout << "Wrong password!\n";
                break;
            case Steam::EResult::AccountLoginDeniedNeedTwoFactor:
                std::cout << "NeedTwoFactorCode!\n";
                break;
            case Steam::EResult::TwoFactorCodeMismatch:
                //client.onHandshake();
                std::cout << "TwoFactorCodeMismatch!\n";
                break;
            case Steam::EResult::InvalidLoginAuthCode:
                std::cout << "Invalid Auth Code!\n";
                break;
            default:
                std::cout << "Error! number:" << static_cast<int>(result) <<"\n";
                break;
        }
        if (result != Steam::EResult::OK) {
            running = false;
            ioc.stop();
        }
    };

    client.onPrivateMsg = [](Steam::SteamID user, std::string message) {
        std::cout << "message from: " << user.steamID64 << " " << message << '\n';
        if (message == "ping") {
            client.SendPrivateMessage( user, "pong");
        }
    };

    std::thread run([&]() {ioc.run(); });
    std::getchar();
    running = false;
    ioc.stop();
    if(steamPoller.joinable())
        steamPoller.join();
    run.join();
    delete sock;
}