package com.amivoice.wrp;

import java.io.*;
import java.util.Random;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Socket;

class Wrp_ extends Wrp implements Runnable {
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

	private Socket socket_;
	private OutputStream out_;
	private InputStream in_;
	private byte[] outData_;
	private byte[] inData_;
	private int inDataBytes_;
	private String result__;
	private Thread thread_;

	public Wrp_() {
		socket_ = null;
		out_ = null;
		in_ = null;
		outData_ = new byte[10 + 1 + 4096];
		inData_ = new byte[4096];
		inDataBytes_ = 0;
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
		if (!scheme.equals("ws") && !scheme.equals("wss")) {
			throw new MalformedURLException("Invalid scheme: " + scheme);
		}
		String hostName;
		String resource;
		int slashIndex = serverURL.indexOf('/', colonDoubleSlashIndex + 3);
		if (slashIndex != -1) {
			hostName = serverURL.substring(colonDoubleSlashIndex + 3, slashIndex);
			resource = serverURL.substring(slashIndex);
		} else {
			hostName = serverURL.substring(colonDoubleSlashIndex + 3);
			resource = "/";
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
			if (scheme.equals("wss")) {
				port = 443;
			} else {
				port = 80;
			}
		}
		String host;
		if (port == 80) {
			host = hostName;
		} else {
			host = hostName + ":" + port;
		}
		String proxyHostName = null;
		int proxyPort = 0;
		String proxyUserName = null;
		String proxyPassword = null;
		if (proxyServerName != null) {
			proxyHostName = proxyServerName;
			int atIndex = proxyHostName.lastIndexOf('@');
			if (atIndex != -1) {
				/* int */ colonIndex = proxyHostName.indexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName = proxyHostName.substring(0, colonIndex);
					proxyPassword = proxyHostName.substring(colonIndex + 1, atIndex);
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
			if (scheme.equals("ws") || scheme.equals("wss")) {
				OutputStream out = socket.getOutputStream();
				InputStream in = socket.getInputStream();
				StringBuilder proxyRequestHeader = new StringBuilder();
				proxyRequestHeader.append("CONNECT ");
				proxyRequestHeader.append(hostName);
				proxyRequestHeader.append(":");
				proxyRequestHeader.append(port);
				proxyRequestHeader.append(" HTTP/1.1\r\n");
				proxyRequestHeader.append("Host: ");
				proxyRequestHeader.append(host);
				proxyRequestHeader.append("\r\n");
				proxyRequestHeader.append("User-Agent: ");
				proxyRequestHeader.append(Wrp.getVersion());
				proxyRequestHeader.append("\r\n");
				if (proxyUserName != null && proxyPassword != null) {
					proxyRequestHeader.append("Proxy-Authorization: Basic ");
					proxyRequestHeader.append(_basic(proxyUserName, proxyPassword));
					proxyRequestHeader.append("\r\n");
					proxyUserName = null;
					proxyPassword = null;
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
				resource = serverURL;
			}
		}
		if (scheme.equals("wss")) {
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
		out_ = socket_.getOutputStream();
		in_ = socket_.getInputStream();
		inDataBytes_ = 0;
		result__ = null;
		// -----------------------------------------------------------
		// GET <resource> HTTP/1.1
		// Host: <hostName>:<port>
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Key: ...
		// 
		// -----------------------------------------------------------
		StringBuilder requestHeader = new StringBuilder();
		requestHeader.append("GET ");
		requestHeader.append(resource);
		requestHeader.append(" HTTP/1.1\r\n");
		requestHeader.append("User-Agent: ");
		requestHeader.append(Wrp.getVersion());
		requestHeader.append("\r\n");
		requestHeader.append("Host: ");
		requestHeader.append(host);
		requestHeader.append("\r\n");
		requestHeader.append("Upgrade: websocket\r\n");
		requestHeader.append("Connection: upgrade\r\n");
		requestHeader.append("Sec-WebSocket-Key: ");
		Random random = new Random();
		for (int i = 0; i < 20; i += 4) {
			int twentyFourBits = random.nextInt() & 0x3F3F3F3F;
			requestHeader.append(_base64Table[(twentyFourBits >> 24) & 0x3F]);
			requestHeader.append(_base64Table[(twentyFourBits >> 16) & 0x3F]);
			requestHeader.append(_base64Table[(twentyFourBits >>  8) & 0x3F]);
			requestHeader.append(_base64Table[(twentyFourBits      ) & 0x3F]);
		}
		int eightBits = random.nextInt() & 0x3F300000;
		requestHeader.append(_base64Table[(eightBits >> 24) & 0x3F]);
		requestHeader.append(_base64Table[(eightBits >> 16) & 0x3F]);
		requestHeader.append("==\r\n");
		requestHeader.append("\r\n");
		out_.write(requestHeader.toString().getBytes());
		out_.flush();
		// -----------------------------------------------------------
		// HTTP/1.1 101 Switching Protocols
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Accept: ...
		// 
		// -----------------------------------------------------------
		int responseHeaderBytes = read_CRLFCRLF_();
		if (responseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		String responseHeader = new String(inData_, 0, responseHeaderBytes);
		if (responseHeader.charAt( 8) != ' '
		 || responseHeader.charAt( 9) != '1'
		 || responseHeader.charAt(10) != '0'
		 || responseHeader.charAt(11) != '1'
		 || responseHeader.charAt(12) != ' ') {
			throw new IOException("Handshake error (invalld response code: " + responseHeader.substring(9, 12) + ")");
		}
		shift_(responseHeaderBytes);
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
	protected void sendRequest_(byte[] data, int dataOffset, int dataBytes, byte command) throws IOException {
		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-------+-+-------------+-------------------------------+
		// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		// | |1|2|3|       |K|             |                               |
		// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		// |     Extended payload length continued, if payload len == 127  |
		// + - - - - - - - - - - - - - - - +-------------------------------+
		// |                               |Masking-key, if MASK set to 1  |
		// +-------------------------------+-------------------------------+
		// | Masking-key (continued)       |          Payload Data         |
		// +-------------------------------- - - - - - - - - - - - - - - - +
		// :                     Payload Data continued ...                :
		// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		// |                     Payload Data continued ...                |
		// +---------------------------------------------------------------+
		int realDataBytes = ((command == 0) ? 0 : 1) + dataBytes;
		if (outData_.length < 10 + realDataBytes) {
			outData_ = new byte[10 + realDataBytes];
		}
		int outDataBytes = 0;
		outData_[outDataBytes++] = (byte)(0x80 | ((command == 0) ? /* Text frame */ 0x01 : /* Binary frame */ 0x02));
		if (realDataBytes < 126) {
			outData_[outDataBytes++] = (byte)realDataBytes;
		} else
		if (realDataBytes < 65536) {
			outData_[outDataBytes++] = (byte)126;
			outData_[outDataBytes++] = (byte)((realDataBytes >>  8) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes      ) & 0xFF);
		} else {
			outData_[outDataBytes++] = (byte)127;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = (byte)((realDataBytes >> 24) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes >> 16) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes >>  8) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes      ) & 0xFF);
		}
		if (command == 0) {
			// pass
		} else {
			outData_[outDataBytes++] = command;
		}
		System.arraycopy(data, dataOffset, outData_, outDataBytes, dataBytes);
		outDataBytes += dataBytes;
		out_.write(outData_, 0, outDataBytes);
		out_.flush();
	}

	@Override
	public void run() {
		try {
			onOpen_();
			while (receiveResponse_()) {
				onMessage_(result__);
			}
			onClose_();
		} catch (IOException e) {
			onError_(e);
		}
	}

	private boolean receiveResponse_() throws IOException {
		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-------+-+-------------+-------------------------------+
		// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		// | |1|2|3|       |K|             |                               |
		// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		// |     Extended payload length continued, if payload len == 127  |
		// + - - - - - - - - - - - - - - - +-------------------------------+
		// |                               |Masking-key, if MASK set to 1  |
		// +-------------------------------+-------------------------------+
		// | Masking-key (continued)       |          Payload Data         |
		// +-------------------------------- - - - - - - - - - - - - - - - +
		// :                     Payload Data continued ...                :
		// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		// |                     Payload Data continued ...                |
		// +---------------------------------------------------------------+
		if (!read_(2)) {
			if (inDataBytes_ == 0) {
				return false;
			}
			throw new IOException("Unexpected end of stream");
		}
		int inDataBytes = (inData_[1] & 0x7F);
		shift_(2);
		if (inDataBytes == 126) {
			if (!read_(2)) {
				throw new IOException("Unexpected end of stream");
			}
			inDataBytes = ((inData_[0] & 0xFF) <<  8)
						| ((inData_[1] & 0xFF)      );
			shift_(2);
		} else
		if (inDataBytes == 127) {
			if (!read_(8)) {
				throw new IOException("Unexpected end of stream");
			}
			if (inData_[0] != 0
			 || inData_[1] != 0
			 || inData_[2] != 0
			 || inData_[3] != 0 || (inData_[4] & 0x80) != 0) {
				throw new IOException("Invalid payload length: " + inDataBytes);
			}
			inDataBytes = ((inData_[4] & 0xFF) << 24)
						| ((inData_[5] & 0xFF) << 16)
						| ((inData_[6] & 0xFF) <<  8)
						| ((inData_[7] & 0xFF)      );
			shift_(8);
		}
		if (inDataBytes < 0) {
			throw new IOException("Invalid payload length: " + inDataBytes);
		}
		if (!read_(inDataBytes)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes);
		shift_(inDataBytes);
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
		if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\n') {
			inDataBytes--;
			if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\r') {
				inDataBytes--;
			}
		}
		String result;
		try {
			result = new String(inData_, 0, inDataBytes, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			result = new String(inData_, 0, inDataBytes);
		}
		result__ = result;
	}

	private void shift_(int inDataBytes) {
		inDataBytes_ -= inDataBytes;
		System.arraycopy(inData_, inDataBytes, inData_, 0, inDataBytes_);
	}
}
