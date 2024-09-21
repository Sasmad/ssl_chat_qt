cmake_minimum_required(VERSION 3.5)

project(ssl_chat_qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Находит пакеты Boost и OpenSSL для Boost.Asio
find_package(Boost 1.65 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ssl_chat_qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ssl_chat_qt SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ssl_chat_qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Связывание с библиотеками Qt, Boost и OpenSSL
target_link_libraries(ssl_chat_qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ssl_chat_qt)
endif()
set_target_properties(ssl_chat_qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Находим Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Запрашиваем на создание Doxyfile с правильными путями на основе имеющегося Doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Добавляем цель для Doxygen в CMake
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else()
    message("Doxygen needs to be installed to generate the documentation.")
endif()

# Интеграция с процессом сборки
include(GNUInstallDirs)
install(TARGETS ssl_chat_qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ssl_chat_qt)
endif()

# Загрузка и компиляция Google Test и Google Mock
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)

FetchContent_MakeAvailable(googletest)

# Добавление тестовой цели
enable_testing()

add_executable(tests
    mainwindow_test.cpp
    #mainwindow_testPast.cpp
    mainwindow.h
    mainwindow.cpp  # Включение mainwindow.cpp для реализации HttpClient
)

target_link_libraries(tests
    gtest
    gmock
    Qt${QT_VERSION_MAJOR}::Widgets
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_test(NAME tests COMMAND tests)
