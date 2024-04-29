cmake_minimum_required(VERSION 3.17)
project(SteamBot)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
    if(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -bigobj")
    endif()
endif ()

add_compile_definitions(RAPIDJSON_HAS_STDSTRING=1)

find_package(OpenSSL REQUIRED)
find_package(Boost 1.84.0 REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(SteamBot src/main.cpp src/CustomEMsgHandler.cpp src/consoleColor.cpp src/TradeOffer.cpp)

add_subdirectory(lib/SteamPP)

target_include_directories(SteamBot PRIVATE ${Boost_INCLUDE_DIR} http_parser)
target_include_directories(SteamBot PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
target_include_directories(SteamBot PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/certify/include>)
target_include_directories(SteamBot PRIVATE ${PROTOBUFS_DIR})
TARGET_LINK_LIBRARIES(SteamBot PUBLIC ${Boost_LIBRARIES})
target_link_libraries(SteamBot PRIVATE OpenSSL::SSL OpenSSL::Crypto steam++)
IF (WIN32)
    TARGET_LINK_LIBRARIES(SteamBot PRIVATE crypt32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN )
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.env ${CMAKE_CURRENT_BINARY_DIR}/.env COPYONLY)