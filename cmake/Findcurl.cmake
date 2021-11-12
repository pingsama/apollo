include(FindPackageHandleStandardArgs)

set(curl_ROOT_DIR ${PROJECT_SOURCE_DIR}/3rdparty/curl)

if (NOT EXISTS ${curl_ROOT_DIR})
    message(FATAL_ERROR "curl download error!")
endif ()

set(curl_INCLUDES ${curl_ROOT_DIR}/include)
list(APPEND curl_LIBS ${curl_ROOT_DIR}/lib/libcurl.a)

find_package_handle_standard_args(curl DEFAULT_MSG curl_LIBS curl_INCLUDES)