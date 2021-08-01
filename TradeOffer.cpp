//
// Created by Max on 31. 7. 2021.
//

#include "TradeOffer.h"

bool TradeOffer::send() {
    return 0;
}

TradeOffer::TradeOffer(uint64_t partner, const std::string& token) {

}

void TradeOffer::addOurItem(TradeOffer::OfferAsset & item) {
    ourItems.push_back(item);
}

void TradeOffer::addTheirItem(TradeOffer::OfferAsset &item) {
    theirItems.push_back(item);
}
