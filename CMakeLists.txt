cmake_minimum_required(VERSION 3.17)
project(SteamBot)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
    if(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -bigobj")
    endif()
endif ()

find_package(OpenSSL REQUIRED)
find_package(Boost 1.75.0 REQUIRED)

add_executable(SteamBot main.cpp SteamApi.cpp WebRequest.cpp utils.cpp SteamCommunity.cpp CustomEMsgHandler.cpp consoleColor.cpp)

add_subdirectory(lib/SteamPP)

target_include_directories(SteamBot PRIVATE ${Boost_INCLUDE_DIR} http_parser)
target_include_directories(SteamBot PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/certify/include>)
target_include_directories(SteamBot PRIVATE ${PROTOBUFS_DIR})
TARGET_LINK_LIBRARIES(SteamBot PUBLIC ${Boost_LIBRARIES})
target_link_libraries(SteamBot PRIVATE OpenSSL::SSL OpenSSL::Crypto steam++)
IF (WIN32)
    TARGET_LINK_LIBRARIES(SteamBot PRIVATE crypt32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.env ${CMAKE_CURRENT_BINARY_DIR}/.env COPYONLY)