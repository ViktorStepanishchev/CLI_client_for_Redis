#cmake_minimum_required(VERSION 3.28)
#project(CLI_client_REDIS)
#
#set(CMAKE_CXX_STANDARD 17)
#
#add_executable(CLI_client_REDIS my_redis_cli.cpp
#)

cmake_minimum_required(VERSION 3.28)
project(CLI_client_REDIS)

# Установите компилятор C++11 или более поздней версии
set(CMAKE_CXX_STANDARD 17)

# Найдите OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_definitions(-DOPENSSL_NO_SSL2 -DOPENSSL_NO_SSL3)
endif()

# Добавьте исполняемый файл вашего проекта
add_executable(CLI_client_REDIS my_redis_cli.cpp)

# Свяжите OpenSSL с вашим исполняемым файлом
target_link_libraries(CLI_client_REDIS ${OPENSSL_LIBRARIES})

#cmake_minimum_required(VERSION 3.28)
#project(CLI_client_REDIS)
#
#set(CMAKE_CXX_STANDARD 17)
#
#find_package(OpenSSL REQUIRED)
#
#add_executable(CLI_client_REDIS my_redis_cli.cpp
#)
#include_directories(${OPENSSL_INCLUDE_DIR})
#target_link_libraries(CLI_client_REDIS ${OPENSSL_LIBRARIES})
