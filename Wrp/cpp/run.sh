#!/bin/bash
read -p "Please enter AppKey: " AppKey
set -x
export SSL_CERT_FILE=../../curl-ca-bundle.crt
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/linux64_release
bin/linux64_release/WrpSimpleTester wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general $AppKey
