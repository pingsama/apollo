include(FindPackageHandleStandardArgs)

set(jsoncpp_ROOT_DIR ${PROJECT_SOURCE_DIR}/3rdparty/jsoncpp)

if (NOT EXISTS ${jsoncpp_ROOT_DIR})
    file(DOWNLOAD "${LIB_DOWNLOAD_URL}/supremind/cpp/lib/jsoncpp-1.9.3-linux-amd64.tar.gz" "jsoncpp-1.9.3-linux-amd64.tar.gz")
    execute_process(COMMAND cmake -E tar -zxf "${CMAKE_CURRENT_SOURCE_DIR}/build/jsoncpp-1.9.3-linux-amd64.tar.gz" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty RESULT_VARIABLE rv)
    if(NOT rv EQUAL 0)
        message(FATAL_ERROR "jsoncpp download error!")
    endif()
endif ()

set(jsoncpp_INCLUDES ${jsoncpp_ROOT_DIR}/include)
list(APPEND jsoncpp_LIBS ${jsoncpp_ROOT_DIR}/lib/libjsoncpp.a)

find_package_handle_standard_args(jsoncpp DEFAULT_MSG jsoncpp_LIBS jsoncpp_INCLUDES)