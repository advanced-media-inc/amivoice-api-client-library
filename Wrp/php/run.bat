@set /P AppKey="Please enter AppKey: "
php -dinclude_path=src -dopenssl.cafile=../../curl-ca-bundle.crt -dextension=openssl WrpSimpleTester.php wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general %AppKey%
@pause