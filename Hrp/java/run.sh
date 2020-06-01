#!/bin/bash
read -p "Please enter AppKey: " AppKey
set -x
java -cp  java -cp .:Hrp.jar HrpSimpleTester https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general $AppKey
