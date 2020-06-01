namespace com.amivoice.wrp {

public interface WrpListener {
	void utteranceStarted(int startTime);
	void utteranceEnded(int endTime);
	void resultCreated();
	void resultUpdated(string result);
	void resultFinalized(string result);
	void TRACE(string message);
}

} // namespace
