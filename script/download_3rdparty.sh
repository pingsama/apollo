#!/bin/bash
set -e

download_url='https://ai-private-devtools.oss-cn-shanghai.aliyuncs.com/supremind/cpp/lib'

mkdir -p ./3rdparty
library_path=./3rdparty

download_library() {
    wget -P $library_path $download_url/$1
    tar -zxf $library_path/$1 -C $library_path
    rm -rf $library_path/$1
}

download_library jsoncpp-1.9.3-linux-amd64.tar.gz
download_library openssl-1.1.1-linux-amd64.tar.gz
download_library curl-7.74.0-linux-amd64.tar.gz