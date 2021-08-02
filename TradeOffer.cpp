//
// Created by Max on 31. 7. 2021.
//

#include "TradeOffer.h"
#include "consoleColor.h"

#include <utility>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>
#include "consoleColor.h"

bool TradeOffer::send(SteamCommunity & community) {
    std::ostringstream offerParams;
    rapidjson::Document params;
    params.SetObject();
    if (!token.empty())
        params.AddMember("trade_offer_access_token", token, params.GetAllocator()); //TODO FIX - COMPILE ERROR
    rapidjson::OStreamWrapper offerStream (offerParams);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(offerStream);
    params.Accept(writer);
    std::cout << offerParams.str() << '\n';


    std::ostringstream referer;
    referer << "https://steamcommunity.com/tradeoffer/new/?partner=" << partner;
    if(!token.empty()){
        referer << "&token=" << token;
    };


    std::ostringstream offerData;
    rapidjson::Document data;
    data.SetObject();
    data.AddMember("newversion", true, data.GetAllocator());
    data.AddMember("version", ourItems.size() + theirItems.size() + 1, data.GetAllocator());
    std::array<const char *, 2> sides = {"me", "them"};
    for(const auto & side : sides) {
        rapidjson::Value someside;
        someside.SetObject();
        someside.AddMember("currency", rapidjson::Value().SetArray(), data.GetAllocator());
        someside.AddMember("ready", false, data.GetAllocator());
        rapidjson::Value assets;
        assets.SetArray();
        std::vector<OfferAsset> * items;
        if(strcmp(side, "me") == 0) {
            items = &ourItems;
        } else {
            items = &theirItems;
        }
        for(const auto & item : *items ) {
            rapidjson::Value asset;
            asset.SetObject();
            asset.AddMember("appid", std::to_string(item.appid), data.GetAllocator());
            asset.AddMember("contextid", std::to_string(item.contextid), data.GetAllocator());
            asset.AddMember("amount", std::to_string(item.amount), data.GetAllocator());
            asset.AddMember("assetid", std::to_string(item.assetid), data.GetAllocator());
            assets.PushBack(asset, data.GetAllocator());
        }
        someside.AddMember("assets", assets, data.GetAllocator());

        data.AddMember(rapidjson::Value(side, std::strlen(side)), someside, data.GetAllocator());
    }
    rapidjson::OStreamWrapper dataStream (offerData);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer2(dataStream);
    data.Accept(writer2);
    std::cout << offerData.str() << '\n';
    community.request("/tradeoffer/new/send", true, {
        {"sessionid", community.sessionToken},
        {"serverid", "1"},
        {"partner", std::to_string(partner)},
        {"tradeoffermessage", message},
        {"json_tradeoffer", offerData.str()},
        {"captcha", ""},
        {"trade_offer_create_params", offerParams.str()}
        //tradeofferid_countered
    }, referer.str(), [](http::response<http::string_body> &resp){
        std::cout << color(colorFG::Green) << resp.body().c_str() << color();
    });

    return false;
}

TradeOffer::TradeOffer(uint64_t partner, std::string token) : partner(partner), token(std::move(token)) {}

void TradeOffer::addOurItem(TradeOffer::OfferAsset & item) {
    ourItems.push_back(item);
}

void TradeOffer::addTheirItem(TradeOffer::OfferAsset &item) {
    theirItems.push_back(item);
}

void TradeOffer::addOurItem(SteamCommunity::InventoryItem &item, uint32_t count) {
    if(count < 1) count = 1;
    ourItems.push_back({item.appid, item.contextid, std::min(count, item.amount), item.assetid});
}

void TradeOffer::addTheirItem(SteamCommunity::InventoryItem &item, uint32_t count) {
    if(count < 1) count = 1;
    theirItems.push_back({item.appid, item.contextid, std::min(count, item.amount), item.assetid});
}
