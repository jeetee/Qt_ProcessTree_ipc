cmake_minimum_required(VERSION 3.14)

project(ProcessTree LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Gui Widgets)

add_executable(ProcessTree
  main.cpp
  runmode.h
  commandlineparser.h commandlineparser.cpp

  ipc.h
  launcher.h launcher.cpp
  processmanager.h processmanager.cpp

  convertor.h convertor.cpp

  guiapp.h guiapp.cpp
  mainwindow.h mainwindow.cpp
)
target_link_libraries(ProcessTree Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets)

install(TARGETS ProcessTree
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
