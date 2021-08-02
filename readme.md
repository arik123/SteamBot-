#SteamBot++
This is a bot made using SteamPP library, it does not do much.\
It was created as a project to learn some networking in C++.\
I will hopefully implement more features at some point.
My end goal is to be able to trade.
## Features
It can:
- load .env files 
- store and use sentry file 
- generate auth codes using shared_secrets
- it responds to chat message `ping`
- if `_DEBUG` preprocessor definition is set, it will log all CMsg
- send requests to steam API
- pick lowest latency CM server and connect to it
- fetch and parse inventory
- **WIP** send tradeoffers
## Requirements
I recomend using vcpkg on windows
- CryptoPP
- Protobuf
- OpenSSL
- Boost ASIO, BEAST
- LibArchive
- RapidJSON
## Compilation
You need CMAKE and working compiler\
Tested with:
- gcc 11.1.0 on linux
- msvc 19.28 on windows