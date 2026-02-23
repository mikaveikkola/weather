cmake_minimum_required(VERSION 3.16)
project(weather_backend_cpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon  REQUIRED)
find_package(libpqxx REQUIRED)   # finds the cmake config installed by our source build
find_package(pugixml REQUIRED)
find_package(CURL    REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(weather_backend ${SOURCES})

target_include_directories(weather_backend PRIVATE src)

target_link_libraries(weather_backend PRIVATE
    Drogon::Drogon
    libpqxx::pqxx
    pugixml::pugixml
    CURL::libcurl
)
