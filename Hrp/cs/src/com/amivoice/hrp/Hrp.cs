using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Web;

#if !UNITY_2018_4_OR_NEWER
[assembly: AssemblyTitle("Hrp")]
[assembly: AssemblyProduct("Hrp")]
[assembly: AssemblyCopyright("Copyright (C) 2019-2021 Advanced Media, Inc.")]
[assembly: ComVisible(false)]
[assembly: Guid("d605a501-80ec-4f48-a8fb-2a70e627a257")]
[assembly: AssemblyVersion("1.0.03")]
[assembly: AssemblyFileVersion("1.0.03")]
#endif

namespace com.amivoice.hrp {

public abstract class Hrp {
	private static string VERSION = "Hrp/1.0.03 CLR/" + System.Environment.Version.ToString() + " (" + System.Environment.OSVersion.ToString() + ")";

	public static string getVersion() {
		return VERSION;
	}

	public static Hrp construct() {
		return new Hrp_();
	}

	public static Hrp construct(int implementation) {
		if (implementation == 1) {
			return new Hrp_();
		} else {
			throw new ArgumentException("Unknown implementation: " + implementation);
		}
	}

	private HrpListener listener_;
	private string serverURL_;
	private string proxyServerName_;
	private int connectTimeout_;
	private int receiveTimeout_;
	private string grammarFileNames_;
	private string profileId_;
	private string profileWords_;
	private string segmenterProperties_;
	private string keepFillerToken_;
	private string resultUpdatedInterval_;
	private string extension_;
	private string authorization_;
	private string domainId_;
	private string codec_;
	private string resultType_;
	private string resultEncoding_;
	private string serviceAuthorization_;
	private string voiceDetection_;
	private string acceptTopic_;
	private string contentType_;
	private string accept_;
	private int state_;
	private string lastMessage_;

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

	public void setServerURL(string serverURL) {
		serverURL_ = serverURL;
	}

	public void setProxyServerName(string proxyServerName) {
		proxyServerName_ = proxyServerName;
	}

	public void setConnectTimeout(int connectTimeout) {
		connectTimeout_ = connectTimeout;
	}

	public void setReceiveTimeout(int receiveTimeout) {
		receiveTimeout_ = receiveTimeout;
	}

	public void setGrammarFileNames(string grammarFileNames) {
		grammarFileNames_ = grammarFileNames;
	}

	public void setProfileId(string profileId) {
		profileId_ = profileId;
	}

	public void setProfileWords(string profileWords) {
		profileWords_ = profileWords;
	}

	public void setSegmenterProperties(string segmenterProperties) {
		segmenterProperties_ = segmenterProperties;
	}

	public void setKeepFillerToken(string keepFillerToken) {
		keepFillerToken_ = keepFillerToken;
	}

	public void setResultUpdatedInterval(string resultUpdatedInterval) {
		resultUpdatedInterval_ = resultUpdatedInterval;
	}

	public void setExtension(string extension) {
		extension_ = extension;
	}

	public void setAuthorization(string authorization) {
		authorization_ = authorization;
	}

	public void setDomainId(string domainId) {
		domainId_ = domainId;
	}

	public void setCodec(string codec) {
		codec_ = codec;
	}

	public void setResultType(string resultType) {
		resultType_ = resultType;
	}

	public void setResultEncoding(string resultEncoding) {
		resultEncoding_ = resultEncoding;
	}

	public void setServiceAuthorization(string serviceAuthorization) {
		serviceAuthorization_ = serviceAuthorization;
	}

	public void setVoiceDetection(string voiceDetection) {
		voiceDetection_ = voiceDetection;
	}

	public void setAcceptTopic(string acceptTopic) {
		acceptTopic_ = acceptTopic;
	}

	public void setContentType(string contentType) {
		contentType_ = contentType;
	}

	public void setAccept(string accept) {
		accept_ = accept;
	}

	public bool connect() {
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
			lastMessage_ = "ERROR: can't connect to HTTP server (" + e.Message + "): " + serverURL_;
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

	protected abstract void connect_(string serverURL, string proxyServerName, int connectTimeout, int receiveTimeout);

	public bool disconnect() {
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

	public bool isConnected() {
		return isConnected_();
	}

	protected abstract bool isConnected_();

	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public bool feedDataResume() {
		return feedDataResume(/* type */ "mc", /* dataBytes */ 0);
	}

	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public bool feedDataResume(string type) {
		return feedDataResume(type, /* dataBytes */ 0);
	}

	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public bool feedDataResume(long dataBytes) {
		return feedDataResume(/* type */ "mc", dataBytes);
	}

	// type="s"  dataBytes>0 - Single part & no chunked encoding
	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public bool feedDataResume(string type, long dataBytes) {
		if (type == null) {
			type = "mc";
		}
		if (dataBytes > 0) {
			if (type.IndexOf('m') == -1) {
				if (type.IndexOf('c') == -1) {
					type = "s ";
				} else {
					type = "sc";
				}
			} else {
				if (type.IndexOf('c') == -1) {
					type = "m ";
				} else {
					type = "mc";
				}
			}
		} else {
			if (type.IndexOf('m') == -1) {
				type = "sc";
			} else {
				type = "mc";
			}
		}
		lock (this) {
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
				domainId.Append(domainId_);
			} else {
				if (grammarFileNames_ != null) {
					domainId.Append("grammarFileNames=");
					domainId.Append(HttpUtility.UrlEncode(grammarFileNames_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (profileId_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("profileId=");
					domainId.Append(HttpUtility.UrlEncode(profileId_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (profileWords_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("profileWords=");
					domainId.Append(HttpUtility.UrlEncode(profileWords_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (segmenterProperties_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("segmenterProperties=");
					domainId.Append(HttpUtility.UrlEncode(segmenterProperties_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (keepFillerToken_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("keepFillerToken=");
					domainId.Append(HttpUtility.UrlEncode(keepFillerToken_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (resultUpdatedInterval_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("resultUpdatedInterval=");
					domainId.Append(HttpUtility.UrlEncode(resultUpdatedInterval_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (extension_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("extension=");
					domainId.Append(HttpUtility.UrlEncode(extension_, Encoding.UTF8).Replace("+", "%20"));
				}
				if (authorization_ != null) {
					if (domainId.Length > 0) {
						domainId.Append(' ');
					}
					domainId.Append("authorization=");
					domainId.Append(HttpUtility.UrlEncode(authorization_, Encoding.UTF8).Replace("+", "%20"));
				}
			}
			try {
				state_ = 1;
				sendRequestHeader_(domainId.ToString(),
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
				lastMessage_ = "ERROR: can't start feeding data to HTTP server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public bool feedData(byte[] data, int dataOffset, int dataBytes) {
		lock (this) {
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
				lastMessage_ = "ERROR: can't feed data to HTTP server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public bool feedDataPause() {
		lock (this) {
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
					Monitor.Wait(this);
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
			} catch (ThreadInterruptedException) {
				lastMessage_ = "WARNING: interrupted to stop feeding data to HTTP server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't stop feeding data to HTTP server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public string getLastMessage() {
		return lastMessage_;
	}

	public void sleep(int timeout) {
		try {
			Thread.Sleep(timeout);
		} catch (ThreadInterruptedException) {}
	}

	protected abstract void sendRequestHeader_(string domainId,
											   string codec,
											   string resultType,
											   string resultEncoding,
											   string serviceAuthorization,
											   string voiceDetection,
											   string acceptTopic,
											   string contentType,
											   string accept, string type, long dataBytes);

	protected abstract void sendRequestBody_(byte[] data, int dataOffset, int dataBytes);

	protected void onOpen_(string sessionId) {
		if (listener_ != null) {
			listener_.resultCreated(sessionId);
		}
	}

	protected void onClose_() {
		lock (this) {
			state_ = 0;
			Monitor.PulseAll(this);
		}
	}

	protected void onError_(Exception cause) {
		lock (this) {
			if (state_ == 0 || state_ == 5) {
				return;
			}
			lastMessage_ = "ERROR: caught exception (" + cause.Message + ")";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			state_ = 5;
			Monitor.PulseAll(this);
		}
	}

	protected void onMessage_(string resultData) {
		if (listener_ != null) { // {
			if (resultData.EndsWith("...") || resultData.EndsWith("...\"}")) {
				listener_.resultUpdated(resultData);
			} else {
				listener_.resultFinalized(resultData);
			}
		}
	}
}

} // namespace
