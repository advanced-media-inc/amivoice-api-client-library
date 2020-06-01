@set /P AppKey="Please enter AppKey: "
curl -X POST -F a=@../../audio/test.wav "https://acp-api.amivoice.com/v1/recognize?d=-a-general&u=%AppKey%"
@pause