package com.amivoice.hrp;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Socket;
import java.net.URLEncoder;

class Hrp_ extends Hrp implements Runnable {
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
			try {
				final int hostName_domainIndex = hostName.indexOf('.') + 1;
				final int hostName_domainLength = hostName.length() - hostName_domainIndex;
				final java.security.cert.Certificate[] certificates = sslSession.getPeerCertificates();
				if (certificates.length > 0) {
					final java.security.cert.Certificate certificate = certificates[0];
					if (certificate instanceof java.security.cert.X509Certificate) {
						java.security.cert.X509Certificate x509Certificate = (java.security.cert.X509Certificate)certificate;
						final java.util.Collection<java.util.List<?>> subjectAlternativeNames = x509Certificate.getSubjectAlternativeNames();
						if (subjectAlternativeNames != null) {
							for (java.util.List<?> subjectAlternativeName: subjectAlternativeNames) {
								if (subjectAlternativeName.size() >= 2) {
									if ((Integer)subjectAlternativeName.get(0) == 2) {
										final String dnsName = (String)subjectAlternativeName.get(1);
										if (dnsName.startsWith("*.")) {
											if (dnsName.length() - 2 == hostName_domainLength) {
												if (hostName.regionMatches(false, hostName_domainIndex, dnsName, 2, hostName_domainLength)) {
													return true;
												}
											}
										} else {
											if (hostName.equalsIgnoreCase(dnsName)) {
												return true;
											}
										}
									}
								}
							}
						}
						final String subjectPrincipal = x509Certificate.getSubjectX500Principal().getName();
						final java.util.List<javax.naming.ldap.Rdn> subjectPrincipalEntries = new javax.naming.ldap.LdapName(subjectPrincipal).getRdns();
						for (javax.naming.ldap.Rdn subjectPrincipalEntry: subjectPrincipalEntries) {
							javax.naming.directory.Attribute cn = subjectPrincipalEntry.toAttributes().get("cn");
							if (cn != null) {
								String commonName = (String)cn.get();
								if (commonName.startsWith("*.")) {
									if (commonName.length() - 2 == hostName_domainLength) {
										if (hostName.regionMatches(false, hostName_domainIndex, commonName, 2, hostName_domainLength)) {
											return true;
										}
									}
								} else {
									if (hostName.equalsIgnoreCase(commonName)) {
										return true;
									}
								}
							}
						}
					}
				}
				return false;
			} catch (Exception e) {
				return false;
			}
		}
	};

	static {
		try {
			javax.net.ssl.SSLContext sslContext = javax.net.ssl.SSLContext.getInstance("TLSv1.2");
			sslContext.init(null, (!_serverCertificateVerification) ? _trustManagers : null, null);
			javax.net.ssl.SSLContext.setDefault(sslContext);
		} catch (Exception e) {}
	}

	private static final char[] _base64Table = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	static String _basic(String userName, String password) {
		String string = userName + ":" + password;
		StringBuilder buffer = new StringBuilder();
		int state = 0;
		int bits = 0;
		for (int i = 0; i < string.length(); i++) {
			char c = string.charAt(i);
			if (state == 0) {
				bits = (c >> 2) & 0x3F;
				buffer.append(_base64Table[bits]);
				bits = (c << 4) & 0x30;
				state = 1;
			} else
			if (state == 1) {
				bits |= (c >> 4) & 0x0F;
				buffer.append(_base64Table[bits]);
				bits = (c << 2) & 0x3C;
				state = 2;
			} else {
				bits |= (c >> 6) & 0x03;
				buffer.append(_base64Table[bits]);
				bits = c & 0x3F;
				buffer.append(_base64Table[bits]);
				bits = 0;
				state = 0;
			}
		}
		if (state != 0) {
			buffer.append(_base64Table[bits]);
			if (state == 1) {
				buffer.append('=');
			}
			buffer.append('=');
		}
		return buffer.toString();
	}

	private String host_;
	private String resource_;
	private String proxyUserName_;
	private String proxyPassword_;
	private Socket socket_;
	private OutputStream out_;
	private InputStream in_;
	private byte[] requestBodyChunked_;
	private byte[] requestBodyParted_;
	private int requestBodyPartedOffset_;
	private byte[] inData_;
	private int inDataBytes_;
	private int inDataBytes__;
	private String inDataEncoding__;
	private StringBuilder result__;
	private Thread thread_;

	public Hrp_() {
		host_ = null;
		resource_ = null;
		proxyUserName_ = null;
		proxyPassword_ = null;
		socket_ = null;
		out_ = null;
		in_ = null;
		requestBodyChunked_ = null;
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inData_ = new byte[4096];
		inDataBytes_ = 0;
		inDataBytes__ = 0;
		inDataEncoding__ = null;
		result__ = null;
		thread_ = null;
	}

	@Override
	protected void connect_(String serverURL, String proxyServerName, int connectTimeout, int receiveTimeout) throws IOException, MalformedURLException {
		int colonDoubleSlashIndex = serverURL.indexOf("://");
		if (colonDoubleSlashIndex == -1) {
			throw new MalformedURLException("Invalid server URL");
		}
		String scheme = serverURL.substring(0, colonDoubleSlashIndex);
		if (!scheme.equals("http") && !scheme.equals("https")) {
			throw new MalformedURLException("Invalid scheme: " + scheme);
		}
		String hostName;
		int slashIndex = serverURL.indexOf('/', colonDoubleSlashIndex + 3);
		if (slashIndex != -1) {
			hostName = serverURL.substring(colonDoubleSlashIndex + 3, slashIndex);
			resource_ = serverURL.substring(slashIndex);
		} else {
			hostName = serverURL.substring(colonDoubleSlashIndex + 3);
			resource_ = "/";
		}
		int port;
		int colonIndex = hostName.indexOf(':', hostName.indexOf(']') + 1);
		if (colonIndex != -1) {
			try {
				port = Integer.parseInt(hostName.substring(colonIndex + 1));
			} catch (NumberFormatException e) {
				throw new MalformedURLException("Invalid port: " + hostName.substring(colonIndex + 1));
			}
			hostName = hostName.substring(0, colonIndex);
		} else {
			if (scheme.equals("https")) {
				port = 443;
			} else {
				port = 80;
			}
		}
		if (port == 80) {
			host_ = hostName;
		} else {
			host_ = hostName + ":" + port;
		}
		String proxyHostName = null;
		int proxyPort = 0;
		proxyUserName_ = null;
		proxyPassword_ = null;
		if (proxyServerName != null) {
			proxyHostName = proxyServerName;
			int atIndex = proxyHostName.lastIndexOf('@');
			if (atIndex != -1) {
				/* int */ colonIndex = proxyHostName.indexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName_ = proxyHostName.substring(0, colonIndex);
					proxyPassword_ = proxyHostName.substring(colonIndex + 1, atIndex);
					proxyHostName = proxyHostName.substring(atIndex + 1);
				}
			}
			/* int */ colonIndex = proxyHostName.indexOf(':', proxyHostName.indexOf(']') + 1);
			if (colonIndex != -1) {
				try {
					proxyPort = Integer.parseInt(proxyHostName.substring(colonIndex + 1));
				} catch (NumberFormatException e) {
					throw new MalformedURLException("Invalid port: " + proxyHostName.substring(colonIndex + 1));
				}
				proxyHostName = proxyHostName.substring(0, colonIndex);
			} else {
				proxyPort = 80;
			}
		}
		Socket socket = javax.net.SocketFactory.getDefault().createSocket();
		socket.setSoTimeout(receiveTimeout);
		if (proxyServerName != null) {
			socket.connect(new InetSocketAddress(proxyHostName, proxyPort), connectTimeout);
		} else {
			socket.connect(new InetSocketAddress(hostName, port), connectTimeout);
		}
		if (proxyServerName != null) {
			if (scheme.equals("https")) {
				OutputStream out = socket.getOutputStream();
				InputStream in = socket.getInputStream();
				StringBuilder proxyRequestHeader = new StringBuilder();
				proxyRequestHeader.append("CONNECT ");
				proxyRequestHeader.append(hostName);
				proxyRequestHeader.append(":");
				proxyRequestHeader.append(port);
				proxyRequestHeader.append(" HTTP/1.1\r\n");
				proxyRequestHeader.append("Host: ");
				proxyRequestHeader.append(host_);
				proxyRequestHeader.append("\r\n");
				proxyRequestHeader.append("User-Agent: ");
				proxyRequestHeader.append(Hrp.getVersion());
				proxyRequestHeader.append("\r\n");
				if (proxyUserName_ != null && proxyPassword_ != null) {
					proxyRequestHeader.append("Proxy-Authorization: Basic ");
					proxyRequestHeader.append(_basic(proxyUserName_, proxyPassword_));
					proxyRequestHeader.append("\r\n");
					proxyUserName_ = null;
					proxyPassword_ = null;
				}
				proxyRequestHeader.append("\r\n");
				out.write(proxyRequestHeader.toString().getBytes());
				out.flush();
				StringBuilder proxyResponseHeader = new StringBuilder();
				int CRLFCRLF = 0;
				int c = in.read();
				while (c != -1) {
					proxyResponseHeader.append((char)c);
					CRLFCRLF <<= 8;
					CRLFCRLF |= c;
					if (CRLFCRLF == 0x0D0A0D0A) {
						break;
					}
					c = in.read();
				}
				if (proxyResponseHeader.charAt( 8) != ' '
				 || proxyResponseHeader.charAt( 9) != '2'
				 || proxyResponseHeader.charAt(10) != '0'
				 || proxyResponseHeader.charAt(11) != '0'
				 || proxyResponseHeader.charAt(12) != ' ') {
					throw new IOException("Invalid response code: " + proxyResponseHeader.substring(9, 12));
				}
			} else {
				resource_ = serverURL;
			}
		}
		if (scheme.equals("https")) {
			javax.net.ssl.SSLSocket sslSocket = (javax.net.ssl.SSLSocket)((javax.net.ssl.SSLSocketFactory)javax.net.ssl.SSLSocketFactory.getDefault()).createSocket(socket, hostName, port, true);
			sslSocket.startHandshake();
			if (_serverCertificateVerification) {
				if (!_hostNameVerifier.verify(hostName, sslSocket.getSession())) {
					throw new IOException("Can't verify host name: " + hostName);
				}
			}
			socket_ = sslSocket;
		} else {
			socket_ = socket;
		}
		out_ = new BufferedOutputStream(socket_.getOutputStream());
		in_ = socket_.getInputStream();
		thread_ = new Thread(this);
		thread_.start();
	}

	@Override
	protected void disconnect_() {
		if (socket_ != null) {
			try {
				socket_.close();
			} catch (IOException e) {}
			socket_ = null;
		}
		if (thread_ != null) {
			try {
				thread_.join();
			} catch (InterruptedException e) {}
			thread_ = null;
		}
	}

	@Override
	protected boolean isConnected_() {
		return (socket_ != null);
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
		requestBodyChunked_ = null;
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inDataBytes_ = 0;
		inDataBytes__ = 0;
		inDataEncoding__ = null;
		result__ = new StringBuilder();
		StringBuilder requestHeader = new StringBuilder();
		requestHeader.append("POST ");
		requestHeader.append(resource_);
		if (type.charAt(0) != 'm') {
			char delimiter = (resource_.indexOf('?') == -1) ? '?' : '&';
			if (domainId != null && domainId.length() > 0) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("d=");
				requestHeader.append(URLEncoder.encode(domainId, "UTF-8"));
			}
			if (codec != null && codec.length() > 0) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("c=");
				requestHeader.append(URLEncoder.encode(codec, "UTF-8"));
			}
			if (resultType != null && resultType.length() > 0) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("r=");
				requestHeader.append(URLEncoder.encode(resultType, "UTF-8"));
			}
			if (resultEncoding != null && resultEncoding.length() > 0) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("e=");
				requestHeader.append(URLEncoder.encode(resultEncoding, "UTF-8"));
			}
			if (serviceAuthorization != null && serviceAuthorization.length() > 0) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("u=");
				requestHeader.append(URLEncoder.encode(serviceAuthorization, "UTF-8"));
			}
			if (voiceDetection != null) {
				requestHeader.append(delimiter);
				delimiter = '&';
				requestHeader.append("v=");
				requestHeader.append(URLEncoder.encode(voiceDetection, "UTF-8"));
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
		requestHeader.append(" HTTP/1.1\r\n");
		requestHeader.append("Host: ");
		requestHeader.append(host_);
		requestHeader.append("\r\n");
		requestHeader.append("User-Agent: ");
		requestHeader.append(Hrp.getVersion());
		requestHeader.append("\r\n");
		if (proxyUserName_ != null && proxyPassword_ != null) {
			requestHeader.append("Proxy-Authorization: Basic ");
			requestHeader.append(_basic(proxyUserName_, proxyPassword_));
			requestHeader.append("\r\n");
		}
		if (type.charAt(1) != 'c') {
			requestHeader.append("Content-Length: ");
			if (requestBodyParted_ == null) {
				requestHeader.append(dataBytes);
			} else {
				requestHeader.append(dataBytes + requestBodyParted_.length);
			}
			requestHeader.append("\r\n");
		} else {
			requestBodyChunked_ = "0\r\n\r\n".getBytes();
			requestHeader.append("Transfer-Encoding: chunked\r\n");
		}
		if (requestBodyParted_ == null) {
			if (acceptTopic != null && acceptTopic.length() > 0) {
				requestHeader.append("Accept-Topic: ");
				requestHeader.append(acceptTopic);
				requestHeader.append("\r\n");
			}
			if (contentType != null && contentType.length() > 0) {
				requestHeader.append("Content-Type: ");
				requestHeader.append(contentType);
				requestHeader.append("\r\n");
			}
			if (accept != null && accept.length() > 0) {
				requestHeader.append("Accept: ");
				requestHeader.append(accept);
				requestHeader.append("\r\n");
			}
		} else {
			requestHeader.append("Content-Type: multipart/form-data; boundary=1234567890ABCDEFGHIJ\r\n");
		}
		requestHeader.append("\r\n");
		out_.write(requestHeader.toString().getBytes());
		out_.flush();
	}

	@Override
	protected void sendRequestBody_(byte[] data, int dataOffset, int dataBytes) throws IOException {
		if (data != null && dataBytes > 0) {
			if (requestBodyParted_ != null) {
				if (requestBodyPartedOffset_ == 0) {
					requestBodyPartedOffset_ = requestBodyParted_.length - 28;
					if (requestBodyChunked_ != null) {
						out_.write(Integer.toHexString(requestBodyPartedOffset_).getBytes());
						out_.write(requestBodyChunked_, 1, 2);
					}
					out_.write(requestBodyParted_, 0, requestBodyPartedOffset_);
					if (requestBodyChunked_ != null) {
						out_.write(requestBodyChunked_, 3, 2);
					}
				}
			}
			if (requestBodyChunked_ != null) {
				out_.write(Integer.toHexString(dataBytes).getBytes());
				out_.write(requestBodyChunked_, 1, 2);
			}
			out_.write(data, dataOffset, dataBytes);
			if (requestBodyChunked_ != null) {
				out_.write(requestBodyChunked_, 3, 2);
			}
			out_.flush();
		} else {
			if (requestBodyParted_ != null) {
				if (requestBodyChunked_ != null) {
					out_.write(Integer.toHexString(requestBodyParted_.length - requestBodyPartedOffset_).getBytes());
					out_.write(requestBodyChunked_, 1, 2);
				}
				out_.write(requestBodyParted_, requestBodyPartedOffset_, requestBodyParted_.length - requestBodyPartedOffset_);
				if (requestBodyChunked_ != null) {
					out_.write(requestBodyChunked_, 3, 2);
					out_.write(requestBodyChunked_, 0, 5);
				}
			} else {
				if (requestBodyChunked_ != null) {
					out_.write(requestBodyChunked_, 0, 5);
				}
			}
			out_.flush();
		}
	}

	@Override
	public void run() {
		try {
			while (true) {
				String sessionId = receiveResponseHeader_();
				onOpen_(sessionId);
				if (inDataBytes__ > 0) {
					receiveResponseBody_();
					int crlfIndex = result_CRLF_();
					while (crlfIndex != -1) {
						String result = result__.substring(0, crlfIndex);
						onMessage_(result);
						result__.delete(0, crlfIndex + 2);
						crlfIndex = result_CRLF_();
					}
				} else {
					while (receiveChunkedResponseBody_()) {
						int crlfIndex = result_CRLF_();
						while (crlfIndex != -1) {
							String result = result__.substring(0, crlfIndex);
							onMessage_(result);
							result__.delete(0, crlfIndex + 2);
							crlfIndex = result_CRLF_();
						}
					}
				}
				onClose_();
			}
		} catch (IOException e) {
			onError_(e);
		}
	}

	private String receiveResponseHeader_() throws IOException {
		int responseHeaderBytes = read_CRLFCRLF_();
		if (responseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		String responseHeader = new String(inData_, 0, responseHeaderBytes);
		if (responseHeader.charAt( 8) != ' '
		 || responseHeader.charAt( 9) != '2'
		 || responseHeader.charAt(10) != '0'
		 || responseHeader.charAt(11) != '0'
		 || responseHeader.charAt(12) != ' ') {
			throw new IOException("Invalid response code: " + responseHeader.substring(9, 12));
		}
		int index1 = responseHeader.indexOf("\r\nX-Session-ID: ");
		if (index1 == -1) {
			throw new IOException("Missing session id");
		}
		index1 += 16;
		int index2 = responseHeader.indexOf("\r\n", index1);
		if (index2 == -1) {
			throw new IOException("Missing session id");
		}
		String sessionId = responseHeader.substring(index1, index2);
		inDataEncoding__ = "UTF-8";
		index1 = responseHeader.indexOf("\r\nContent-Type: ");
		if (index1 != -1) {
			index1 += 16;
			index2 = responseHeader.indexOf("\r\n", index1);
			if (index2 != -1) {
				int index3 = responseHeader.indexOf("; charset=", index1);
				if (index3 != -1 && index3 < index2) {
					index3 += 10;
					inDataEncoding__ = responseHeader.substring(index3, index2).toUpperCase();
					if (inDataEncoding__.equals("SHIFT_JIS")) {
						inDataEncoding__ = "MS932";
					}
				}
			}
		}
		index1 = responseHeader.indexOf("\r\nContent-Length: ");
		if (index1 != -1) {
			index1 += 18;
			index2 = responseHeader.indexOf("\r\n", index1);
			if (index2 != -1) {
				try {
					inDataBytes__ = Integer.parseInt(responseHeader.substring(index1, index2));
				} catch (NumberFormatException e) {
					throw new IOException("Invalid content length: " + responseHeader.substring(index1, index2));
				}
			}
		}
		shift_(responseHeaderBytes);
		return sessionId;
	}

	private void receiveResponseBody_() throws IOException {
		if (!read_(inDataBytes__)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes__);
		shift_(inDataBytes__);
	}

	private boolean receiveChunkedResponseBody_() throws IOException {
		int chunkedResponseHeaderBytes = read_CRLF_();
		if (chunkedResponseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		inDataBytes__ = 0;
		for (int i = 0; i < chunkedResponseHeaderBytes; i++) {
			int c = inData_[i];
			if (c >= '0' && c <= '9') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - '0';
			} else
			if (c >= 'A' && c <= 'F') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - 'A' + 10;
			} else
			if (c >= 'a' && c <= 'f') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - 'a' + 10;
			} else {
				break;
			}
		}
		shift_(chunkedResponseHeaderBytes);
		if (inDataBytes__ == 0) {
			if (!read_(2)) {
				throw new IOException("Unexpected end of stream");
			}
			shift_(2);
			return false;
		}
		if (!read_(inDataBytes__ + 2)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes__);
		shift_(inDataBytes__ + 2);
		return true;
	}

	private int read_CRLFCRLF_() throws IOException {
		int inDataBytes = find_CRLFCRLF_();
		while (inDataBytes == -1) {
			ensure_(inDataBytes_ + 4096);
			int inDataReadBytes = in_.read(inData_, inDataBytes_, 4096);
			if (inDataReadBytes == -1) {
				return -1;
			}
			inDataBytes_ += inDataReadBytes;
			inDataBytes = find_CRLFCRLF_();
		}
		return inDataBytes + 4;
	}

	private int find_CRLFCRLF_() {
		for (int i = 0; i < inDataBytes_ - 3; i++) {
			if (inData_[i    ] == '\r'
			 && inData_[i + 1] == '\n'
			 && inData_[i + 2] == '\r'
			 && inData_[i + 3] == '\n') {
				return i;
			}
		}
		return -1;
	}

	private int read_CRLF_() throws IOException {
		int inDataBytes = find_CRLF_();
		while (inDataBytes == -1) {
			ensure_(inDataBytes_ + 4096);
			int inDataReadBytes = in_.read(inData_, inDataBytes_, 4096);
			if (inDataReadBytes == -1) {
				return -1;
			}
			inDataBytes_ += inDataReadBytes;
			inDataBytes = find_CRLF_();
		}
		return inDataBytes + 2;
	}

	private int find_CRLF_() {
		for (int i = 0; i < inDataBytes_ - 1; i++) {
			if (inData_[i    ] == '\r'
			 && inData_[i + 1] == '\n') {
				return i;
			}
		}
		return -1;
	}

	private boolean read_(int inDataBytes) throws IOException {
		ensure_(inDataBytes);
		while (inDataBytes_ < inDataBytes) {
			int inDataReadBytes = in_.read(inData_, inDataBytes_, inDataBytes - inDataBytes_);
			if (inDataReadBytes == -1) {
				return false;
			}
			inDataBytes_ += inDataReadBytes;
		}
		return true;
	}

	private void ensure_(int inDataBytes) {
		if (inData_.length < inDataBytes) {
			int inDataNewBytes = inData_.length;
			while (inDataNewBytes < inDataBytes) {
				inDataNewBytes *= 2;
			}
			byte[] inData = new byte[inDataNewBytes];
			System.arraycopy(inData_, 0, inData, 0, inDataBytes_);
			inData_ = inData;
		}
	}

	private void result_(int inDataBytes) {
		String result;
		try {
			result = new String(inData_, 0, inDataBytes, inDataEncoding__);
		} catch (UnsupportedEncodingException e) {
			result = new String(inData_, 0, inDataBytes);
		}
		result__.append(result);
	}

	private int result_CRLF_() {
		for (int i = 0; i < result__.length() - 1; i++) {
			if (result__.charAt(i    ) == '\r'
			 && result__.charAt(i + 1) == '\n') {
				return i;
			}
		}
		return -1;
	}

	private void shift_(int inDataBytes) {
		inDataBytes_ -= inDataBytes;
		System.arraycopy(inData_, inDataBytes, inData_, 0, inDataBytes_);
	}
}
