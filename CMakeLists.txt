cmake_minimum_required(VERSION 3.16)
project(FreeCADLauncher VERSION 1.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools QUIET)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/services/downloader.cpp
    src/services/appimagemanager.cpp
    src/services/configsync.cpp
    src/services/githubreleasesfetcher.cpp
    src/services/filedownloader.cpp
    src/services/traymanager.cpp
    src/utils/versionextractor.cpp
    src/utils/architecturedetector.cpp
    src/utils/filesizeformatter.cpp
    src/utils/githubreleasesparser.cpp
    src/helpers/filesystemchecker.cpp
    src/ui/appimageselectordialog.cpp
    src/ui/releasestablecontroller.cpp
    resources/resources.qrc
)

set(HEADERS
    src/mainwindow.h
    src/types.h
    src/constants.h
    src/interfaces/idownloader.h
    src/interfaces/iappimagemanager.h
    src/interfaces/iconfigsync.h
    src/interfaces/itraymanager.h
    src/services/downloader.h
    src/services/appimagemanager.h
    src/services/configsync.h
    src/services/githubreleasesfetcher.h
    src/services/filedownloader.h
    src/services/traymanager.h
    src/utils/versionextractor.h
    src/utils/architecturedetector.h
    src/utils/filesizeformatter.h
    src/utils/githubreleasesparser.h
    src/helpers/filesystemchecker.h
    src/ui/appimageselectordialog.h
    src/ui/releasestablecontroller.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${PROJECT_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

set(TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/FreeCADLauncher_ru.ts
    ${CMAKE_SOURCE_DIR}/translations/FreeCADLauncher_fr.ts
)
set(QM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/translations)
set(QM_RU ${QM_OUTPUT_DIR}/FreeCADLauncher_ru.qm)
set(QM_FR ${QM_OUTPUT_DIR}/FreeCADLauncher_fr.qm)
file(MAKE_DIRECTORY ${QM_OUTPUT_DIR})
find_program(LRELEASE_EXECUTABLE NAMES lrelease-qt6 lrelease lrelease-qt5
    PATHS /usr/lib64/qt6/bin /usr/lib/qt6/bin /usr/lib/x86_64-linux-gnu/qt6/bin
    PATH_SUFFIXES bin
    NO_DEFAULT_PATH)
if(NOT LRELEASE_EXECUTABLE)
    find_program(LRELEASE_EXECUTABLE NAMES lrelease-qt6 lrelease lrelease-qt5)
endif()
if(LRELEASE_EXECUTABLE)
    add_custom_command(OUTPUT ${QM_RU} ${QM_FR}
        COMMAND ${LRELEASE_EXECUTABLE} -qm ${QM_RU} ${CMAKE_SOURCE_DIR}/translations/FreeCADLauncher_ru.ts
        COMMAND ${LRELEASE_EXECUTABLE} -qm ${QM_FR} ${CMAKE_SOURCE_DIR}/translations/FreeCADLauncher_fr.ts
        COMMENT "Building translation .qm files"
        VERBATIM
    )
    add_custom_target(FreeCADLauncher_translations ALL DEPENDS ${QM_RU} ${QM_FR})
    add_dependencies(${PROJECT_NAME} FreeCADLauncher_translations)
endif()
