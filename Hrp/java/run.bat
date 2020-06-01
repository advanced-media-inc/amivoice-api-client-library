@set /P AppKey="Please enter AppKey: "
java -cp .;Hrp.jar HrpSimpleTester https://acp-api.amivoice.com/v1/recognize ../../audio/test.wav 16K -a-general %AppKey%
@pause