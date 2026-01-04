cmake_minimum_required(VERSION 3.16)

project(MahjongApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(MahjongCoreLib
    MahjongCore.h
    MahjongCore.cpp
)
target_link_libraries(MahjongCoreLib PRIVATE Qt6::Widgets)
target_include_directories(MahjongCoreLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(MahjongHintLib
    MahjongScorer.h
    MahjongScorer.cpp
    TileButton.h
    TileButton.cpp
    SquareButton.h
    SquareButton.cpp

)
target_link_libraries(MahjongHintLib PRIVATE Qt6::Widgets)
target_include_directories(MahjongHintLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(MahjongApp
    main.cpp
    StartWindow.h
    StartWindow.cpp
    mainwindow.h
    mainwindow.cpp
    BWindow.h
    BWindow.cpp
    SquareButton.h
    SquareButton.cpp
)

target_link_libraries(MahjongApp PRIVATE Qt6::Widgets MahjongCoreLib MahjongHintLib)
