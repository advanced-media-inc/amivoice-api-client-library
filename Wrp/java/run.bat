@set /P AppKey="Please enter AppKey: "
java -cp .;Wrp.jar WrpSimpleTester wss://acp-api.amivoice.com/v1/ ../../audio/test.wav 16K -a-general %AppKey%
@pause