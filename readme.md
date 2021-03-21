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
## Requirements
I recomend using vcpkg on windows
- CryptoPP
- Protobuf
- OpenSSL
- Boost ASIO, BEAST
- LibArchive\
## Compilation
You need CMAKE and working compiler\
Tested with:
- gcc 10.0.2 on linux
- msvc 19.28 on windows


