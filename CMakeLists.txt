cmake_minimum_required(VERSION 3.25)
project(typr-tui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_TLS_VERIFY)
    set(CMAKE_TLS_VERIFY OFF CACHE BOOL "" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

# Generate version header from git tag
execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE TYPR_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT TYPR_VERSION)
    set(TYPR_VERSION "v0.1.0")
endif()
message(STATUS "typr version: ${TYPR_VERSION}")

set(FTXUI_VERSION "v7.0.1")
set(SQLITE_VERSION "v3.53.3")
message(STATUS "ftxui version: ${FTXUI_VERSION}")
message(STATUS "sqlite3 version: ${SQLITE_VERSION}")

configure_file(
    ${CMAKE_SOURCE_DIR}/src/version.hpp.in
    ${CMAKE_BINARY_DIR}/generated/version.hpp
    @ONLY
  )

set(CONTENT_DB_VERSION "v1.5.2")
set(CONTENT_DB_BASE_URL "https://github.com/AhmedowM/ctypr/releases/download/${CONTENT_DB_VERSION}")
set(CONTENT_DB_DIR ${CMAKE_BINARY_DIR}/content)

file(MAKE_DIRECTORY ${CONTENT_DB_DIR})

if(NOT EXISTS ${CONTENT_DB_DIR}/sentences.db)
  message(STATUS "Downloading sentences.db (1.7 MB)...")
  file(DOWNLOAD ${CONTENT_DB_BASE_URL}/sentences.db ${CONTENT_DB_DIR}/sentences.db
        SHOW_PROGRESS STATUS sdb_status)
  list(GET sdb_status 0 sdb_code)
  if(NOT sdb_code EQUAL 0)
    message(FATAL_ERROR "Failed to download sentences.db: ${sdb_status}")
  endif()
endif()

if(NOT EXISTS ${CONTENT_DB_DIR}/words.db)
  message(STATUS "Downloading words.db (5.7 MB)...")
  file(DOWNLOAD ${CONTENT_DB_BASE_URL}/words.db ${CONTENT_DB_DIR}/words.db
        SHOW_PROGRESS STATUS wdb_status)
  list(GET wdb_status 0 wdb_code)
  if(NOT wdb_code EQUAL 0)
    message(FATAL_ERROR "Failed to download words.db: ${wdb_status}")
  endif()
endif()

message(STATUS "Content databases ready in ${CONTENT_DB_DIR}")

add_executable(typr
    src/main.cpp
    src/app/ContentLocator.cpp
src/app/AppController.cpp
    src/ui/Theme.cpp
    src/ui/Chrome.cpp
    src/ui/BigText.cpp
    src/ui/components/StatBox.cpp
    src/ui/components/SettingsBar.cpp
    src/ui/screens/MainScreen.cpp
    src/ui/screens/TypingScreen.cpp
    src/ui/screens/ResultsScreen.cpp
    src/ui/screens/HistoryScreen.cpp
    src/ui/screens/StatsScreen.cpp
    src/ui/screens/InfoScreen.cpp
    src/engine/EngineBridge.cpp
    src/engine/ContentSource.cpp
    src/storage/Storage.cpp
)

target_include_directories(typr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(typr PRIVATE
  ftxui::component
  cpptypr
)

if(WIN32)
  target_link_libraries(typr PRIVATE urlmon)
endif()

target_compile_definitions(typr PRIVATE
  TYPR_CONTENT_VERSION="${CONTENT_DB_VERSION}"
)

install(TARGETS typr RUNTIME DESTINATION bin)
install(FILES
  ${CONTENT_DB_DIR}/sentences.db
  ${CONTENT_DB_DIR}/words.db
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/typr/content
)
