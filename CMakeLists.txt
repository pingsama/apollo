cmake_minimum_required(VERSION 3.5)
project(apollo CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_options("-Wall")

set(LIB_DOWNLOAD_URL "https://ai-private-devtools.oss-cn-shanghai.aliyuncs.com")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


find_package(jsoncpp REQUIRED)
if (jsoncpp_FOUND)
    include_directories(${jsoncpp_INCLUDES})
    list(APPEND LINK_LIB_LIST ${jsoncpp_LIBS})
else()
    message(FATAL_ERROR "jsoncpp not found")
endif ()

find_package(curl REQUIRED)
if (curl_FOUND)
    include_directories(${curl_INCLUDES})
    list(APPEND LINK_LIB_LIST ${curl_LIBS})
else()
    message(FATAL_ERROR "curl not found")
endif ()

find_package(openssl REQUIRED)
if (openssl_FOUND)
    include_directories(${openssl_INCLUDES})
    list(APPEND LINK_LIB_LIST ${openssl_LIBS})
else()
    message(FATAL_ERROR "openssl not found")
endif ()

execute_process(
    COMMAND ar -x ${curl_LIBS}
    COMMAND ar -x ${jsoncpp_LIBS}
    COMMAND ar -x ${openssl_ROOT_DIR}/lib/libssl.a
    COMMAND ar -x ${openssl_ROOT_DIR}/lib/libcrypto.a
)


file(GLOB SRC_APOLLO src/*.cpp src/apollo/*.h)
file(GLOB SRC_OBJ ${CMAKE_BINARY_DIR}/*.o)

add_library(apollo STATIC ${SRC_APOLLO} ${SRC_OBJ})
# target_link_libraries(apollo INTERFACE ${LINK_LIB_LIST})
