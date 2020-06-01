#!/bin/bash
read -p "Please enter AppKey: " AppKey
set -x
java -cp  java -cp .:Wrp.jar WrpSimpleTester wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general $AppKey

