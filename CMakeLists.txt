cmake_minimum_required(VERSION 3.21)

project(QViaggiaTreno
    VERSION 2026.10
    DESCRIPTION "Front-end grafico a ViaggiaTreno"
    HOMEPAGE_URL "https://github.com/M0Rf30/qviaggiatreno"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Xml)

# AUTOMOC/AUTOUIC, GNUInstallDirs e policy Qt raccomandate
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

# Impedisce la reintroduzione di API Qt deprecate e di costrutti superati
add_compile_definitions(
    QT_DISABLE_DEPRECATED_UP_TO=0x060500
    QT_NO_FOREACH
    QT_NO_CONTEXTLESS_CONNECT
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

include(CTest)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

install(FILES qviaggiatreno.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES src/img/logo.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME qviaggiatreno.svg)
