#!/bin/bash
read -p "Please enter AppKey: " AppKey
set -x
export PYTHONPATH=src
export SSL_CERT_FILE=../../curl-ca-bundle.crt
python WrpSimpleTester.py wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general $AppKey