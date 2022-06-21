package com.amivoice.hrp;

import java.io.*;
import java.net.URLEncoder;

public abstract class Hrp {
	private static final String VERSION = "Hrp/1.0.03 Java/" + System.getProperty("java.version") + " (" + System.getProperty("os.name") + " " + System.getProperty("os.version") + ")";

	public static String getVersion() {
		return VERSION;
	}

	public static Hrp construct() {
		return new Hrp_();
	}

	public static Hrp construct(int implementation) {
		try {
			if (implementation == 1) {
				return (Hrp)Class.forName("com.amivoice.hrp.Hrp_").getDeclaredConstructor().newInstance();
			} else
			if (implementation == 2) {
				return (Hrp)Class.forName("com.amivoice.hrp.Hrp__").getDeclaredConstructor().newInstance();
			} else {
				throw new IllegalArgumentException("Unknown implementation: " + implementation);
			}
		} catch (Exception e) {
			throw new IllegalArgumentException("Unknown implementation: " + implementation);
		}
	}

	private HrpListener listener_;
	private String serverURL_;
	private String proxyServerName_;
	private int connectTimeout_;
	private int receiveTimeout_;
	private String grammarFileNames_;
	private String profileId_;
	private String profileWords_;
	private String segmenterProperties_;
	private String keepFillerToken_;
	private String resultUpdatedInterval_;
	private String extension_;
	private String authorization_;
	private String domainId_;
	private String codec_;
	private String resultType_;
	private String resultEncoding_;
	private String serviceAuthorization_;
	private String voiceDetection_;
	private String acceptTopic_;
	private String contentType_;
	private String accept_;
	private int state_;
	private String lastMessage_;

	protected Hrp() {
		listener_ = null;
		serverURL_ = null;
		proxyServerName_ = null;
		connectTimeout_ = 0;
		receiveTimeout_ = 0;
		grammarFileNames_ = null;
		profileId_ = null;
		profileWords_ = null;
		segmenterProperties_ = null;
		keepFillerToken_ = null;
		resultUpdatedInterval_ = null;
		extension_ = null;
		authorization_ = null;
		domainId_ = null;
		codec_ = null;
		resultType_ = null;
		resultEncoding_ = null;
		serviceAuthorization_ = null;
		voiceDetection_ = null;
		acceptTopic_ = null;
		contentType_ = null;
		accept_ = null;
		state_ = 0;
	}

	public void setListener(HrpListener listener) {
		listener_ = listener;
	}

	public void setServerURL(String serverURL) {
		serverURL_ = serverURL;
	}

	public void setProxyServerName(String proxyServerName) {
		proxyServerName_ = proxyServerName;
	}

	public void setConnectTimeout(int connectTimeout) {
		connectTimeout_ = connectTimeout;
	}

	public void setReceiveTimeout(int receiveTimeout) {
		receiveTimeout_ = receiveTimeout;
	}

	public void setGrammarFileNames(String grammarFileNames) {
		grammarFileNames_ = grammarFileNames;
	}

	public void setProfileId(String profileId) {
		profileId_ = profileId;
	}

	public void setProfileWords(String profileWords) {
		profileWords_ = profileWords;
	}

	public void setSegmenterProperties(String segmenterProperties) {
		segmenterProperties_ = segmenterProperties;
	}

	public void setKeepFillerToken(String keepFillerToken) {
		keepFillerToken_ = keepFillerToken;
	}

	public void setResultUpdatedInterval(String resultUpdatedInterval) {
		resultUpdatedInterval_ = resultUpdatedInterval;
	}

	public void setExtension(String extension) {
		extension_ = extension;
	}

	public void setAuthorization(String authorization) {
		authorization_ = authorization;
	}

	public void setDomainId(String domainId) {
		domainId_ = domainId;
	}

	public void setCodec(String codec) {
		codec_ = codec;
	}

	public void setResultType(String resultType) {
		resultType_ = resultType;
	}

	public void setResultEncoding(String resultEncoding) {
		resultEncoding_ = resultEncoding;
	}

	public void setServiceAuthorization(String serviceAuthorization) {
		serviceAuthorization_ = serviceAuthorization;
	}

	public void setVoiceDetection(String voiceDetection) {
		voiceDetection_ = voiceDetection;
	}

	public void setAcceptTopic(String acceptTopic) {
		acceptTopic_ = acceptTopic;
	}

	public void setContentType(String contentType) {
		contentType_ = contentType;
	}

	public void setAccept(String accept) {
		accept_ = accept;
	}

	public boolean connect() {
		if (isConnected_()) {
			lastMessage_ = "WARNING: already connected to HTTP server";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		if (serverURL_ == null) {
			lastMessage_ = "ERROR: can't connect to HTTP server (Missing server URL)";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		try {
			state_ = 0;
			connect_(serverURL_, proxyServerName_, connectTimeout_, receiveTimeout_);
		} catch (Exception e) {
			lastMessage_ = "ERROR: can't connect to HTTP server (" + e.getMessage() + "): " + serverURL_;
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			disconnect_();
			return false;
		}
		lastMessage_ = "INFO: connected to HTTP server: " + serverURL_;
		if (listener_ != null) {
			listener_.TRACE(lastMessage_);
		}
		return true;
	}

	protected abstract void connect_(String serverURL, String proxyServerName, int connectTimeout, int receiveTimeout) throws Exception;

	public boolean disconnect() {
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from HTTP server";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		disconnect_();
		lastMessage_ = "INFO: disconnected from HTTP server";
		if (listener_ != null) {
			listener_.TRACE(lastMessage_);
		}
		return true;
	}

	protected abstract void disconnect_();

	public boolean isConnected() {
		return isConnected_();
	}

	protected abstract boolean isConnected_();

	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public boolean feedDataResume() {
		return feedDataResume(/* type */ "mc", /* dataBytes */ 0);
	}

	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public boolean feedDataResume(String type) {
		return feedDataResume(type, /* dataBytes */ 0);
	}

	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public boolean feedDataResume(long dataBytes) {
		return feedDataResume(/* type */ "mc", dataBytes);
	}

	// type="s"  dataBytes>0 - Single part & no chunked encoding
	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public boolean feedDataResume(String type, long dataBytes) {
		if (type == null) {
			type = "mc";
		}
		if (dataBytes > 0) {
			if (type.indexOf('m') == -1) {
				if (type.indexOf('c') == -1) {
					type = "s ";
				} else {
					type = "sc";
				}
			} else {
				if (type.indexOf('c') == -1) {
					type = "m ";
				} else {
					type = "mc";
				}
			}
		} else {
			if (type.indexOf('m') == -1) {
				type = "sc";
			} else {
				type = "mc";
			}
		}
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 0) {
				lastMessage_ = "WARNING: already started feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			StringBuilder domainId = new StringBuilder();
			if (domainId_ != null) {
				domainId.append(domainId_);
			} else {
				if (grammarFileNames_ != null) {
					domainId.append("grammarFileNames=");
					try {
						domainId.append(URLEncoder.encode(grammarFileNames_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (profileId_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("profileId=");
					try {
						domainId.append(URLEncoder.encode(profileId_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (profileWords_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("profileWords=");
					try {
						domainId.append(URLEncoder.encode(profileWords_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (segmenterProperties_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("segmenterProperties=");
					try {
						domainId.append(URLEncoder.encode(segmenterProperties_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (keepFillerToken_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("keepFillerToken=");
					try {
						domainId.append(URLEncoder.encode(keepFillerToken_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (resultUpdatedInterval_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("resultUpdatedInterval=");
					try {
						domainId.append(URLEncoder.encode(resultUpdatedInterval_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (extension_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("extension=");
					try {
						domainId.append(URLEncoder.encode(extension_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
				if (authorization_ != null) {
					if (domainId.length() > 0) {
						domainId.append(' ');
					}
					domainId.append("authorization=");
					try {
						domainId.append(URLEncoder.encode(authorization_, "UTF-8").replace("+", "%20"));
					} catch (UnsupportedEncodingException e) {}
				}
			}
			try {
				state_ = 1;
				sendRequestHeader_(domainId.toString(),
								   codec_,
								   resultType_,
								   resultEncoding_,
								   serviceAuthorization_,
								   voiceDetection_,
								   acceptTopic_,
								   contentType_,
								   accept_, type, dataBytes);
				state_ = 2;
				lastMessage_ = "INFO: started feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't start feeding data to HTTP server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public boolean feedData(byte[] data, int dataOffset, int dataBytes) {
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 2 && state_ != 3) {
				lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			try {
				state_ = 3;
				sendRequestBody_(data, dataOffset, dataBytes);
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't feed data to HTTP server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public boolean feedDataPause() {
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 2 && state_ != 3) {
				lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			try {
				state_ = 4;
				sendRequestBody_(null, 0, 0);
				while (state_ == 4) {
					wait();
				}
				if (state_ == 5) {
					lastMessage_ = "ERROR: can't stop feeding data to HTTP server";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					return false;
				}
				lastMessage_ = "INFO: stopped feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
			} catch (InterruptedException e) {
				lastMessage_ = "WARNING: interrupted to stop feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't stop feeding data to HTTP server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public String getLastMessage() {
		return lastMessage_;
	}

	public void sleep(int timeout) {
		try {
			Thread.sleep((long)timeout);
		} catch (InterruptedException e) {}
	}

	protected abstract void sendRequestHeader_(String domainId,
											   String codec,
											   String resultType,
											   String resultEncoding,
											   String serviceAuthorization,
											   String voiceDetection,
											   String acceptTopic,
											   String contentType,
											   String accept, String type, long dataBytes) throws IOException;

	protected abstract void sendRequestBody_(byte[] data, int dataOffset, int dataBytes) throws IOException;

	protected void onOpen_(String sessionId) {
		if (listener_ != null) {
			listener_.resultCreated(sessionId);
		}
	}

	protected void onClose_() {
		synchronized (this) {
			state_ = 0;
			notifyAll();
		}
	}

	protected void onError_(Throwable cause) {
		synchronized (this) {
			if (state_ == 0 || state_ == 5) {
				return;
			}
			lastMessage_ = "ERROR: caught exception (" + cause.getMessage() + ")";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			state_ = 5;
			notifyAll();
		}
	}

	protected void onMessage_(String resultData) {
		if (listener_ != null) { // {
			if (resultData.endsWith("...") || resultData.endsWith("...\"}")) {
				listener_.resultUpdated(resultData);
			} else {
				listener_.resultFinalized(resultData);
			}
		}
	}
}
