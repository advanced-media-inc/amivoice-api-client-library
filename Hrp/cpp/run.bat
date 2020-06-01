@set /P AppKey="Please enter AppKey: "
set SSL_CERT_FILE=../../curl-ca-bundle.crt
bin\winnt64_vc140_release\HrpSimpleTester.exe https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general %AppKey%
@pause