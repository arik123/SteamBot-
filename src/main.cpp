#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <span>

#include "../lib/SteamPP/include/steam++.h"
#include "../lib/SteamPP/include/utils.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../lib/SteamPP/include/SteamCommunity.h"

#include <boost/beast/http/message.hpp>

#include <utility>

#include "rapidjson/document.h"
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
std::thread steamPoller;
asio::io_context ioc;
rapidjson::Document document;

bool running = true;

/*
void readHandle(boost::system::error_code error, std::size_t bytes_transferred){
    if (!error)
        read_buffer.resize(client.readable(reinterpret_cast<const unsigned char*>(read_buffer.data())));
    sock->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()), readHandle);
};
void writeHandle(boost::system::error_code error, std::size_t bytes_transferred){
}
*/

SteamCommunity * community;
int main(const int argc, const char** const argv, char* envp[]) {
    const auto env = Environment(envp);
    Steam::SteamClient client(ioc, env);
    setupConsole();
    std::cout << color(colorFG::Green) << "Starting" << color() << '\n';


    std::ifstream sentryIF("sentry.txt");
    if (sentryIF.is_open())
    {
        std::string sentry;
        sentryIF >> sentry;
        sentryIF.close();
        client.setSentry(std::move(sentry));
    }
    {
        std::string cellid = "0";
        std::ifstream ifile("cellid.txt");
        if (ifile.is_open())
        {
            ifile >> cellid;
            ifile.close();
        }
        client.setCellid(std::move(cellid));
    }



    try
    {
        /*sock = new net::socket(ioc);
        sock->async_read_some(asio::buffer(read_buffer.data(), read_buffer.size()), readHandle);*/
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    community = new SteamCommunity(asio::make_strand(ioc));

    client.onHandshake = [&] {
        std::cout << "logging in \n";
        std::string code = generateAuthCode(env.get("STEAM_SHARED_SECRET"));
        client.LogOn(env.get("STEAM_USERNAME").c_str(), env.get("STEAM_PASSWORD").c_str(), code.data());
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
    client.onLogOn = [&](Steam::EResult result, Steam::SteamID steamID, uint32_t cellid) {
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
            client.api.request("ISteamDirectory", "GetCMList", "v1", false,
            {{"cellid", "0"}},
            [&, user](http::response<http::string_body> resp){

		            std::cout << resp << std::endl;
		            client.SendPrivateMessage(user, "success ?");
            });
        } else if (message == "myinv") {
            community->getUserInventory(user.steamID64, 440, 2, [&, user](const std::vector<SteamCommunity::InventoryItem>& inv ){
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
}