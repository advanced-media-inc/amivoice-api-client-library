@set /P AppKey="Please enter AppKey: "
bin\Release\HrpSimpleTester.exe https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general %AppKey%
@pause