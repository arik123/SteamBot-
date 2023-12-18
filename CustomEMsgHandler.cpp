#include "CustomEMsgHandler.h"

#include <steammessages_clientserver_login.pb.h>
#include <steammessages_clientserver_2.pb.h>
#include <steammessages_clientserver.pb.h>
//
// Created by Max on 31. 7. 2021.
//

bool emsgHandler(Steam::EMsg emsg, const unsigned char *data, size_t length, uint64_t job_id) {
    switch (emsg) {
        case Steam::EMsg::ClientEmailAddrInfo: {
            CMsgClientEmailAddrInfo info;
            info.ParseFromArray(data, length);

            std::cout << (info.email_is_validated() ? color(colorFG::Blue) : color(colorFG::Red))
                      << "Email:" << info.email_address()
                      << (info.email_is_validated() ? " is validated" : " is not validated") << color() << '\n';
            if (info.email_validation_changed())
                std::cout << color(colorFG::Yellow) << "Email validation changed\n" << color();
            if (info.credential_change_requires_code())
                std::cout << color(colorFG::Red) << "Credential change requires code\n" << color();
            if (info.password_or_secretqa_change_requires_code())
                std::cout << color(colorFG::Red) << "Password or secretqa change requires code\n" << color();
            return true;
        }
        case Steam::EMsg::ClientLicenseList: {
            CMsgClientLicenseList list;
            list.ParseFromArray(data, length);

            auto licenses = list.licenses();
            for (auto &license : licenses) {
                std::cout << color(colorFG::Green)
                          << "License:\n" << color(colorFG::Blue)
                          << "package_id " << license.package_id() << '\t'
                          << "time_created " << license.time_created() << '\t'
                          << "time_next_process " << license.time_next_process() << '\t'
                          << "minute_limit " << license.minute_limit() << '\n'
                          << "minutes_used " << license.minutes_used() << '\t'
                          << "payment_method " << license.payment_method() << '\t'
                          << "flags " << license.flags() << '\t'
                          << "purchase_country_code " << license.purchase_country_code() << '\n'
                          << "license_type " << license.license_type() << '\t'
                          << "territory_code " << license.territory_code() << '\t'
                          << "change_number " << license.change_number() << '\t'
                          << "owner_id " << license.owner_id() << '\n'
                          << "initial_period " << license.initial_period() << '\t'
                          << "initial_time_unit " << license.initial_time_unit() << '\t'
                          << "renewal_period " << license.renewal_period() << '\t'
                          << "renewal_time_unit " << license.renewal_time_unit() << '\n'
                          << "access_token " << license.access_token() << '\t'
                          << "master_package_id " << license.master_package_id() << '\n'
                          << color();
            }
            return true;
        }
        case Steam::EMsg::ClientIsLimitedAccount: {
            CMsgClientIsLimitedAccount limit;
            limit.ParseFromArray(data, length);
            std::cout << color(colorFG::Green) << "Account has following limitations:\n" << color(colorFG::Blue);
            printf("Limited Account %d\nCommunity Ban - %d\nLocked Account %d\nLimited Account Allowed to invite friends %d\n",
                   limit.bis_limited_account(), limit.bis_community_banned(), limit.bis_locked_account(),
                   limit.bis_limited_account_allowed_to_invite_friends());
            std::cout << color();
            return true;
        }
        case Steam::EMsg::ClientWalletInfoUpdate: {
            const std::unordered_map<int, std::string> ECurrencyCodeMap =
            {
                {0, "Invalid"},
                {1, "USD"},
                {2, "GBP"},
                {3, "EUR"},
                {4, "CHF"},
                {5, "RUB"},
                {6, "PLN"},
                {7, "BRL"},
                {8, "JPY"},
                {9, "NOK"},
                {10, "IDR"},
                {11, "MYR"},
                {12, "PHP"},
                {13, "SGD"},
                {14, "THB"},
                {15, "VND"},
                {16, "KRW"},
                {17, "TRY"},
                {18, "UAH"},
                {19, "MXN"},
                {20, "CAD"},
                {21, "AUD"},
                {22, "NZD"},
                {23, "CNY"},
                {24, "INR"},
                {25, "CLP"},
                {26, "PEN"},
                {27, "COP"},
                {28, "ZAR"},
                {29, "HKD"},
                {30, "TWD"},
                {31, "SAR"},
                {32, "AED"},
                {34, "ARS"},
                {35, "ILS"},
                {36, "BYN"},
                {37, "KZT"},
                {38, "KWD"},
                {39, "QAR"},
                {40, "CRC"},
                {41, "UYU"},
            };
            CMsgClientWalletInfoUpdate wallet;
            wallet.ParseFromArray(data, length);
            if (wallet.has_wallet()) {
                std::cout << color(colorFG::Green) << "Wallet:\n"
                          << color(colorFG::Blue) << "Balance: " << wallet.balance() << ' ' << wallet.balance64() << '\n'
                          << "Balance - Delayed: " << wallet.balance_delayed() << ' ' << wallet.balance64_delayed() << '\n'
                          << "Currency: " << ECurrencyCodeMap.at(wallet.currency()) << '\n'
                          << color();
            } else {
                std::cout << color(colorFG::Red) << "user has no wallet\n" << color();
            };
            return true;
        }
    }
    return false;
}
