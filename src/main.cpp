#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <span>

#include "../lib/SteamPP/include/steam++.h"
#include "../lib/SteamPP/include/utils.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include "../lib/SteamPP/include/SteamApi.h"
#include "../lib/SteamPP/include/SteamCommunity.h"

#include <boost/beast/http/message.hpp>

#include <utility>

#include "rapidjson/document.h"
#include "../lib/SteamPP/include/SteamApi.h"
#include "../lib/SteamPP/include/consoleColor.h"
#include "CustomEMsgHandler.h"
#include "TradeOffer.h"

#ifdef WIN32
#include <windows.h>
#endif

using net = boost::asio::ip::tcp;    // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;    // from <boost/asio.hpp>
namespace http = boost::beast::http;

std::string read_buffer;
std::string write_buffer;
net::socket * sock;
std::thread steamPoller;
asio::io_context ioc;
rapidjson::Document document;

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
        [](std::size_t length, const std::function<void(unsigned char* buffer)>& fill) {
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

SteamApi * api;
SteamCommunity * community;
int main(const int argc, const char** const argv, char* envp[]) {
    setupConsole();
    const auto env = loadenv(envp);
    std::cout << color(colorFG::Green) << "Starting" << color() << '\n';
    std::ifstream sentryIF("sentry.txt");
    std::string sentry;
    if (sentryIF.is_open())
    {
        sentryIF >> sentry;
        sentryIF.close();
    }

    try
    {
        api = new SteamApi(asio::make_strand(ioc), env.at("STEAM_API_KEY"));
        client.api = api;//api2; // set
        net::resolver resolver(ioc);
        sock = new net::socket(ioc);
        std::string cellid("0");
        std::ifstream ifile("cellid.txt");
        if (ifile.is_open())
        {
            ifile >> cellid;
            ifile.close();
        }
        api->GetCMList(cellid, [](std::vector<net::endpoint> serverList)
        {
            uint64_t min = std::numeric_limits<uint64_t>::max(), index, i=0;
            for (const auto& server : serverList)
            {
                try
                {
                    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
                    
                    sock->connect(server);
                	
                    std::chrono::system_clock::time_point tp2 = std::chrono::system_clock::now();
                	
					std::chrono::system_clock::duration dtn = tp2-tp;
                    uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dtn).count();
                	if(now < min)
                	{
                        min = now;
                		index = i;
                	}
                    sock->shutdown(net::socket::shutdown_both);
                    sock->close();
                }
                catch (const std::exception& e)
                {
                }
                i++;
            }
            std::cout << "Connecting to: " << serverList[index].address() << ':' << serverList[index].port() << " with ping " << min << " us\n";
            try
            {
                sock->connect(serverList[index]);
                read_buffer.resize(client.connected());
                return;
            }
            catch (const std::exception& e) {}
        	// in case something broke, go back to picking first in list
	        for(const auto& server : serverList)
	        {
                std::cout << "Connecting to: " << server.address() << ':' << server.port() << '\n';
		        try
		        {
                    sock->connect(server);
                    read_buffer.resize(client.connected());
                    break;
		        }
		        catch (const std::exception& e)
		        {
                    continue;
		        }
                std::cout << server << '\n';
	        }
            
        });
        
        

        sock->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()), readHandle);
    }
    catch (std::exception const& e)
    {
        delete api;
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    community = new SteamCommunity(asio::make_strand(ioc));

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
    uint64_t mySteamID;
    client.onLogOn = [&env, &mySteamID](Steam::EResult result, Steam::SteamID steamID, uint32_t cellid) {
        switch(result) {
            case Steam::EResult::OK:
				{
	                std::cout << "logged on!" << std::endl;
	                mySteamID = steamID.steamID64;
	                //client.webLogOn();
	                client.SetPersona(Steam::EPersonaState::Online, "Some new name");
		            
	                    std::ofstream myfile("cellid.txt");
	                    if (!myfile.is_open())
	                    {
	                        std::cout << "could not save sentry\n";
	                    }
	                    else
	                    {
	                        myfile << cellid;
	                        myfile.close();
	                    }
		            
	                client.SetGamePlayed(440);
	                client.SetGamePlayed("Hello, I'm alive, using C++");
	                client.SendPrivateMessage(Steam::SteamID(76561198162885342), "I shall live once again.");
                }
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

    client.onPrivateMsg = [&](Steam::SteamID user, const std::string& message) {
        std::cout << "message from: " << user.steamID64 << " " << message << '\n';
        if (message == "ping") {
            client.SendPrivateMessage(user, "pong");
        } else if(message == "fetch") {
            api->request("ISteamDirectory", "GetCMList", "v1", false,
            {{"cellid", "0"}},
            [&, user](http::response<http::string_body> resp){

		            std::cout << resp << std::endl;
		            client.SendPrivateMessage(user, "success ?");
            });
        } else if (message == "myinv") {
            community->getUserInventory(user.steamID64, 440, 2, [user](const std::vector<SteamCommunity::InventoryItem>& inv ){
                std::string message = "your inventory contains ";
                message += std::to_string(inv.size());
                message += " items";

                client.SendPrivateMessage(user, message.c_str());
            });
        } else if (message == "youinv") {
            community->getUserInventory(mySteamID, 440, 2, [](const std::vector<SteamCommunity::InventoryItem>& inv){
                std::cout << "callback called in main\n";
            });
        } else if (message == "getItem") {
            community->getUserInventory(mySteamID, 440, 2, [user](const std::vector<SteamCommunity::InventoryItem>& inv){
                SteamCommunity::InventoryItem chosenItem;
                for(const auto & item : inv) {
                    if(!item.tradable) continue;
                    chosenItem = item;
                    break;
                }
                TradeOffer offer(user.steamID64);
                offer.addOurItem(chosenItem);
                offer.send(*community);
            });
        }
    };

    client.onSessionToken = [](uint64_t tokenParam)
    {
        //TODO FIND IF USEFULL
#ifdef _DEBUG
        std::cout << "recieved session token " << tokenParam << '\n';
#endif

    };

    client.onWebSession = [](std::vector<std::string> & cookies, std::string & sessionID){
        community->cookies = cookies;
        community->sessionID = sessionID;
    };
	client.defaultHandler = emsgHandler;
    std::thread run([&]() {ioc.run(); });
    std::getchar();
    running = false;
    ioc.stop();
    if(steamPoller.joinable())
        steamPoller.join();
    run.join();
    delete api;
    delete sock;
}