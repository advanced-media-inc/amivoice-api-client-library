package com.amivoice.wrp;

public interface WrpListener {
	void utteranceStarted(int startTime);
	void utteranceEnded(int endTime);
	void resultCreated();
	void resultUpdated(String result);
	void resultFinalized(String result);
	void TRACE(String message);
}
