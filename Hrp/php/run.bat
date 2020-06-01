@set /P AppKey="Please enter AppKey: "
php -dinclude_path=src -dopenssl.cafile=../../curl-ca-bundle.crt -dextension=openssl HrpSimpleTester.php https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general %AppKey%
@pause