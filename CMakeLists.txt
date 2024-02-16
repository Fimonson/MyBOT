cmake_minimum_required(VERSION 3.22)
project(K_BOT)

set(CMAKE_CXX_STANDARD 20)

find_package(Poco QUIET COMPONENTS Foundation Net JSON)

add_executable(KONT_BOT main.cpp AbstractBot.cpp IApiTelegram.cpp AbstractBot.h IApiTelegram.h)

target_link_libraries(KONT_BOT PocoNet PocoNetSSL PocoFoundation PocoJSON)