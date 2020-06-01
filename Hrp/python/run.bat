@set /P AppKey="Please enter AppKey: "
set PYTHONPATH=src
set SSL_CERT_FILE=../../curl-ca-bundle.crt
python HrpSimpleTester.py https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general %AppKey%
@pause