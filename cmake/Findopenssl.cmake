include(FindPackageHandleStandardArgs)

set(openssl_ROOT_DIR ${PROJECT_SOURCE_DIR}/3rdparty/openssl)
set(openssl_INCLUDES ${openssl_ROOT_DIR}/include)
list(APPEND openssl_LIBS ${openssl_ROOT_DIR}/lib/libssl.a)
list(APPEND openssl_LIBS ${openssl_ROOT_DIR}/lib/libcrypto.a)

find_package_handle_standard_args(openssl DEFAULT_MSG openssl_INCLUDES openssl_LIBS)

