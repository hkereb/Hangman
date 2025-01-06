cmake_minimum_required(VERSION 3.10)
project(serwer)

set(CMAKE_CXX_STANDARD 17)

# pliki zrodlowe
set(SOURCES
    hangman-server.cpp
    network-utils.cpp
    comunication-functions.cpp
    handle-client-message.cpp
    Game.cpp
    Lobby.cpp
)

# pliki naglowkowe
set(HEADERS
    network-utils.h
    comunication-functions.h
    handle-client-message.h
    Game.h
    Lobby.h
    Player.h
    Settings.h
)

# plik wykonywalny
add_executable(serwer ${SOURCES} ${HEADERS})

# do debugowania
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(serwer PRIVATE -Wall -Wextra -Wpedantic -g)
endif()
