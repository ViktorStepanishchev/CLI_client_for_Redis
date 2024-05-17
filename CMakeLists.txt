cmake_minimum_required(VERSION 3.28)
project(CLI_client_REDIS)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_definitions(-DOPENSSL_NO_SSL2 -DOPENSSL_NO_SSL3)
endif()

add_executable(CLI_client_REDIS my_redis_cli.cpp)

target_link_libraries(CLI_client_REDIS ${OPENSSL_LIBRARIES})