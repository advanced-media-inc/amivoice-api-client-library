@set /P AppKey="Please enter AppKey: "
set SSL_CERT_FILE=../../curl-ca-bundle.crt
bin\Release\WrpSimpleTester.exe wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general %AppKey%
@pause