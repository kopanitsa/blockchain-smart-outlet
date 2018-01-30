cmake_minimum_required(VERSION 3.8)
project(c_sample)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.c coin.c log.c http.c)
add_executable(c_sample ${SOURCE_FILES})

include_directories(/usr/local/Cellar/openssl/1.0.2n/include)
link_directories(/usr/local/Cellar/openssl/1.0.2n/bin/openssl)
#link_directories(/usr/local/php5/bin/openssl)
target_link_libraries(c_sample ssl)
target_link_libraries(c_sample crypto)

