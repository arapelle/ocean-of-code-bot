TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        avatar.cpp \
        direction.cpp \
        game.cpp \
        main.cpp \
        map.cpp \
        opponent.cpp \
        player.cpp \
        random.cpp \
        tool.cpp \
        turn_info.cpp \
        vec2.cpp

DISTFILES += \
    catlist.txt

HEADERS += \
    avatar.hpp \
    direction.hpp \
    game.hpp \
    game_info.hpp \
    grid.hpp \
    grid_with_sectors.hpp \
    log.hpp \
    map.hpp \
    opponent.hpp \
    player.hpp \
    random.hpp \
    square.hpp \
    tool.hpp \
    turn_info.hpp \
    vec2.hpp
