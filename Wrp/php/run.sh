#!/bin/bash
read -p "Please enter AppKey: " AppKey
set -x
php -dinclude_path=src -dopenssl.cafile=../../curl-ca-bundle.crt -dextension=openssl WrpSimpleTester.php wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general $AppKey