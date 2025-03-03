cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 17)

# pliki zrodlowe
set(SOURCES
    hangman-server.cpp
        network.cpp
        helpers.cpp
    handle-client-message.cpp
    Game.cpp
    Lobby.cpp
        sendToClient.cpp
        sendToClient.cpp
)

# pliki naglowkowe
set(HEADERS
        network.h
        helpers.h
    handle-client-message.h
    Game.h
    Lobby.h
    Player.h
    Settings.h
        sendToClient.h
        sendToClient.cpp
)

# plik wykonywalny
add_executable(server ${SOURCES} ${HEADERS})

# do debugowania
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -g)
endif()
