package com.amivoice.hrp;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Proxy;
import java.net.URL;
import java.net.URLEncoder;

class Hrp__ extends Hrp {
	private static boolean _serverCertificateVerification = true;

	private static javax.net.ssl.TrustManager[] _trustManagers = {
		new javax.net.ssl.X509TrustManager() {
			@Override
			public void checkClientTrusted(java.security.cert.X509Certificate[] chain, String authType) throws java.security.cert.CertificateException {
			}
			@Override
			public void checkServerTrusted(java.security.cert.X509Certificate[] chain, String authType) throws java.security.cert.CertificateException {
			}
			@Override
			public java.security.cert.X509Certificate[] getAcceptedIssuers() {
				return null;
			}
		}
	};

	private static javax.net.ssl.HostnameVerifier _hostNameVerifier = new javax.net.ssl.HostnameVerifier() {
		@Override
		public boolean verify(String hostName, javax.net.ssl.SSLSession sslSession) {
			return true;
		}
	};

	static {
		try {
			javax.net.ssl.SSLContext sslContext = javax.net.ssl.SSLContext.getInstance("TLSv1.2");
			sslContext.init(null, (!_serverCertificateVerification) ? _trustManagers : null, null);
			javax.net.ssl.SSLContext.setDefault(sslContext);
			if (!_serverCertificateVerification) {
				javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(_hostNameVerifier);
			}
		} catch (Exception e) {}
	}

	private String serverURL__;
	private int connectTimeout__;
	private int receiveTimeout__;
	private String proxyHostName_;
	private int proxyPort_;
	private String proxyUserName_;
	private String proxyPassword_;
	private HttpURLConnection httpConnection_;
	private OutputStream out_;
	private InputStream in_;
	private byte[] requestBodyParted_;
	private int requestBodyPartedOffset_;
	private byte[] inData_;
	private int inDataBytes_;
	private String inDataEncoding_;

	public Hrp__() {
		serverURL__ = null;
		connectTimeout__ = 0;
		receiveTimeout__ = 0;
		proxyHostName_ = null;
		proxyPort_ = 0;
		proxyUserName_ = null;
		proxyPassword_ = null;
		httpConnection_ = null;
		out_ = null;
		in_ = null;
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inData_ = new byte[4096];
		inDataBytes_ = 0;
		inDataEncoding_ = null;
	}

	@Override
	protected void connect_(String serverURL, String proxyServerName, int connectTimeout, int receiveTimeout) throws IOException {
		serverURL__ = serverURL;
		connectTimeout__ = connectTimeout;
		receiveTimeout__ = receiveTimeout;
		if (proxyServerName != null) {
			proxyHostName_ = proxyServerName;
			int atIndex = proxyHostName_.lastIndexOf('@');
			if (atIndex != -1) {
				int colonIndex = proxyHostName_.indexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName_ = proxyHostName_.substring(0, colonIndex);
					proxyPassword_ = proxyHostName_.substring(colonIndex + 1, atIndex);
					proxyHostName_ = proxyHostName_.substring(atIndex + 1);
				}
			}
			int colonIndex = proxyHostName_.indexOf(':', proxyHostName_.indexOf(']') + 1);
			if (colonIndex != -1) {
				try {
					proxyPort_ = Integer.parseInt(proxyHostName_.substring(colonIndex + 1));
				} catch (NumberFormatException e) {
					throw new MalformedURLException("Invalid port: " + proxyHostName_.substring(colonIndex + 1));
				}
				proxyHostName_ = proxyHostName_.substring(0, colonIndex);
			} else {
				proxyPort_ = 80;
			}
		}
	}

	@Override
	protected void disconnect_() {
		serverURL__ = null;
	}

	@Override
	protected boolean isConnected_() {
		return (serverURL__ != null);
	}

	@Override
	protected void sendRequestHeader_(String domainId,
									  String codec,
									  String resultType,
									  String resultEncoding,
									  String serviceAuthorization,
									  String voiceDetection,
									  String acceptTopic,
									  String contentType,
									  String accept, String type, long dataBytes) throws IOException {
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inDataBytes_ = 0;
		inDataEncoding_ = null;
		StringBuilder serverURL = new StringBuilder();
		serverURL.append(serverURL__);
		if (type.indexOf('m') == -1) {
			char delimiter = (serverURL__.indexOf('?') == -1) ? '?' : '&';
			if (domainId != null && domainId.length() > 0) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("d=");
				serverURL.append(URLEncoder.encode(domainId, "UTF-8"));
			}
			if (codec != null && codec.length() > 0) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("c=");
				serverURL.append(URLEncoder.encode(codec, "UTF-8"));
			}
			if (resultType != null && resultType.length() > 0) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("r=");
				serverURL.append(URLEncoder.encode(resultType, "UTF-8"));
			}
			if (resultEncoding != null && resultEncoding.length() > 0) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("e=");
				serverURL.append(URLEncoder.encode(resultEncoding, "UTF-8"));
			}
			if (serviceAuthorization != null && serviceAuthorization.length() > 0) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("u=");
				serverURL.append(URLEncoder.encode(serviceAuthorization, "UTF-8"));
			}
			if (voiceDetection != null) {
				serverURL.append(delimiter);
				delimiter = '&';
				serverURL.append("v=");
				serverURL.append(URLEncoder.encode(voiceDetection, "UTF-8"));
			}
		} else {
			StringBuilder requestBody = new StringBuilder();
			if (domainId != null && domainId.length() > 0) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"d\"\r\n\r\n");
				requestBody.append(domainId);
			}
			if (codec != null && codec.length() > 0) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\n");
				requestBody.append(codec);
			}
			if (resultType != null && resultType.length() > 0) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"r\"\r\n\r\n");
				requestBody.append(resultType);
			}
			if (resultEncoding != null && resultEncoding.length() > 0) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"e\"\r\n\r\n");
				requestBody.append(resultEncoding);
			}
			if (serviceAuthorization != null && serviceAuthorization.length() > 0) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"u\"\r\n\r\n");
				requestBody.append(serviceAuthorization);
			}
			if (voiceDetection != null) {
				requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"v\"\r\n\r\n");
				requestBody.append(voiceDetection);
			}
			requestBody.append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\n");
			requestBody.append("\r\n--1234567890ABCDEFGHIJ--\r\n");
			requestBodyParted_ = requestBody.toString().getBytes("UTF-8"); // [70+n+][70+n+][70+n+][70+n+][70+n+][70+n+]70+28 bytes
		}
		if (proxyHostName_ != null) {
			if (serverURL__.startsWith("https")) {
				if (proxyUserName_ != null) {
					final java.net.PasswordAuthentication passwordAuthentication = new java.net.PasswordAuthentication(proxyUserName_, proxyPassword_.toCharArray());
					java.net.Authenticator authenticator = new java.net.Authenticator() {
						@Override
						protected java.net.PasswordAuthentication getPasswordAuthentication() {
							if (!getRequestingHost().equals(proxyHostName_) || getRequestingPort() != proxyPort_) {
								return null;
							}
							return passwordAuthentication;
						}
					};
					java.net.Authenticator.setDefault(authenticator);
					proxyUserName_ = null;
					proxyPassword_ = null;
				}
			}
			httpConnection_ = (HttpURLConnection)(new URL(serverURL.toString())).openConnection(new Proxy(Proxy.Type.HTTP, new InetSocketAddress(proxyHostName_, proxyPort_)));
		} else {
			httpConnection_ = (HttpURLConnection)(new URL(serverURL.toString())).openConnection();
		}
		httpConnection_.setConnectTimeout(connectTimeout__);
		httpConnection_.setReadTimeout(receiveTimeout__);
		httpConnection_.setRequestMethod("POST");
		httpConnection_.setDoOutput(true);
		httpConnection_.setRequestProperty("User-Agent", Hrp.getVersion());
		if (proxyUserName_ != null) {
			httpConnection_.setRequestProperty("Proxy-Authorization", "Basic " + Hrp_._basic(proxyUserName_, proxyPassword_));
		}
		if (type.indexOf('c') == -1) {
			if (requestBodyParted_ == null) {
				httpConnection_.setFixedLengthStreamingMode(dataBytes);
			} else {
				httpConnection_.setFixedLengthStreamingMode(dataBytes + requestBodyParted_.length);
			}
		} else {
			httpConnection_.setChunkedStreamingMode(4096);
		}
		if (requestBodyParted_ == null) {
			if (acceptTopic != null && acceptTopic.length() > 0) {
				httpConnection_.setRequestProperty("Accept-Topic", acceptTopic);
			}
			if (contentType != null && contentType.length() > 0) {
				httpConnection_.setRequestProperty("Content-Type", contentType);
			}
			if (accept != null && accept.length() > 0) {
				httpConnection_.setRequestProperty("Accept", accept);
			}
		} else {
			httpConnection_.setRequestProperty("Content-Type", "multipart/form-data; boundary=1234567890ABCDEFGHIJ");
		}
		httpConnection_.connect();
		out_ = httpConnection_.getOutputStream();
	}

	@Override
	protected void sendRequestBody_(byte[] data, int dataOffset, int dataBytes) throws IOException {
		if (data != null && dataBytes > 0) {
			if (requestBodyParted_ != null) {
				if (requestBodyPartedOffset_ == 0) {
					requestBodyPartedOffset_ = requestBodyParted_.length - 28;
					out_.write(requestBodyParted_, 0, requestBodyPartedOffset_);
				}
			}
			out_.write(data, dataOffset, dataBytes);
			out_.flush();
		} else {
			if (requestBodyParted_ != null) {
				out_.write(requestBodyParted_, requestBodyPartedOffset_, requestBodyParted_.length - requestBodyPartedOffset_);
			}
			out_.close();
			run();
		}
	}

	private void run() {
		try {
			String sessionId = receiveResponseHeader_();
			onOpen_(sessionId);
			if (inDataBytes_ > 0) {
				String result = receiveResponseBody_();
				onMessage_(result);
			} else {
				String result = receiveResponseBodyChunked_();
				while (result != null) {
					onMessage_(result);
					result = receiveResponseBodyChunked_();
				}
			}
			onClose_();
		} catch (IOException e) {
			onError_(e);
		}
	}

	private String receiveResponseHeader_() throws IOException {
		if (httpConnection_.getResponseCode() != 200) {
			throw new IOException("Invalid response code: " + httpConnection_.getResponseCode());
		}
		in_ = httpConnection_.getInputStream();
		String sessionId = httpConnection_.getHeaderField("X-Session-ID");
		if (sessionId == null) {
			throw new IOException("Missing session id");
		}
		inDataEncoding_ = "UTF-8";
		String contentType = httpConnection_.getContentType();
		if (contentType != null) {
			int index1 = contentType.indexOf("; charset=");
			if (index1 != -1) {
				index1 += 10;
				inDataEncoding_ = contentType.substring(index1).toUpperCase();
				if (inDataEncoding_.equals("SHIFT_JIS")) {
					inDataEncoding_ = "MS932";
				}
			}
		}
		int contentLength = httpConnection_.getContentLength();
		if (contentLength >= 0) {
			inDataBytes_ = contentLength;
		}
		return sessionId;
	}

	private String receiveResponseBody_() throws IOException {
		if (inData_.length < inDataBytes_) {
			inData_ = new byte[inDataBytes_];
		}
		int inDataReadBytes = 0;
		while (inDataReadBytes < inDataBytes_) {
			int inDataCurrentReadBytes = in_.read(inData_, inDataReadBytes, inDataBytes_ - inDataReadBytes);
			if (inDataCurrentReadBytes == -1) {
				throw new IOException("Unexpected end of stream");
			}
			inDataReadBytes += inDataCurrentReadBytes;
		}
		int inDataTrimmedBytes = inDataBytes_;
		if (inDataTrimmedBytes > 0 && inData_[inDataTrimmedBytes - 1] == '\n') {
			inDataTrimmedBytes--;
			if (inDataTrimmedBytes > 0 && inData_[inDataTrimmedBytes - 1] == '\r') {
				inDataTrimmedBytes--;
			}
		}
		String text = new String(inData_, 0, inDataTrimmedBytes, inDataEncoding_);
		return text;
	}

	private String receiveResponseBodyChunked_() throws IOException {
		int lastTwoBytesIndex = -1;
		for (int i = 0; i < inDataBytes_; i++) {
			if (inData_[i] == '\r') {
				if (i + 1 < inDataBytes_ && inData_[i + 1] == '\n') {
					lastTwoBytesIndex = i;
					break;
				}
			}
		}
		while (lastTwoBytesIndex == -1) {
			if (inData_.length < inDataBytes_ + 4096) {
				byte[] inData = new byte[inData_.length * 2];
				System.arraycopy(inData_, 0, inData, 0, inDataBytes_);
				inData_ = inData;
			}
			int inDataCurrentReadBytes = in_.read(inData_, inDataBytes_, 4096);
			if (inDataCurrentReadBytes == -1) {
				if (inDataBytes_ == 0) {
					return null;
				}
				throw new IOException("Unexpected end of stream");
			}
			inDataBytes_ += inDataCurrentReadBytes;
			for (int i = 0; i < inDataBytes_; i++) {
				if (inData_[i] == '\r') {
					if (i + 1 < inDataBytes_ && inData_[i + 1] == '\n') {
						lastTwoBytesIndex = i;
						break;
					}
				}
			}
		}
		String text = new String(inData_, 0, lastTwoBytesIndex, inDataEncoding_);
		inDataBytes_ -= lastTwoBytesIndex + 2;
		if (inDataBytes_ > 0) {
			System.arraycopy(inData_, lastTwoBytesIndex + 2, inData_, 0, inDataBytes_);
		}
		return text;
	}
}
