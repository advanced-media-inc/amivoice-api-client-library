package com.amivoice.wrp;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.nio.ByteBuffer;
import java.net.MalformedURLException;
import java.net.URI;
import javax.websocket.*;

class Wrp__ extends Wrp implements MessageHandler.Whole<String> {
	private Session session_;
	private byte[] outData_;

	public Wrp__() {
		session_ = null;
		outData_ = new byte[10 + 1 + 4096];
	}

	@Override
	protected void connect_(String serverURL, String proxyServerName, int connectTimeout, int receiveTimeout) throws Exception {
		String proxyHostName = null;
		int proxyPort = 0;
		String proxyUserName = null;
		String proxyPassword = null;
		if (proxyServerName != null) {
			proxyHostName = proxyServerName;
			int atIndex = proxyHostName.lastIndexOf('@');
			if (atIndex != -1) {
				int colonIndex = proxyHostName.indexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName = proxyHostName.substring(0, colonIndex);
					proxyPassword = proxyHostName.substring(colonIndex + 1, atIndex);
					proxyHostName = proxyHostName.substring(atIndex + 1);
				}
			}
			int colonIndex = proxyHostName.indexOf(':', proxyHostName.indexOf(']') + 1);
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
		WebSocketContainer client = ContainerProvider.getWebSocketContainer();
		if (client instanceof org.glassfish.tyrus.client.ClientManager) {
			Map<String,Object> clientProperties = ((org.glassfish.tyrus.client.ClientManager)client).getProperties();
			if (proxyServerName != null) {
				clientProperties.put(org.glassfish.tyrus.client.ClientProperties.PROXY_URI, "http://" + proxyHostName + ":" + proxyPort);
				if (proxyUserName != null && proxyPassword != null) {
					HashMap<String, String> proxyHeaders = new HashMap<String, String>();
					proxyHeaders.put("Proxy-Authorization", "Basic " + Wrp_._basic(proxyUserName, proxyPassword));
					clientProperties.put(org.glassfish.tyrus.client.ClientProperties.PROXY_HEADERS, proxyHeaders);
				}
			}
		}
		session_ = client.connectToServer(new Endpoint() {
			@Override
			public void onOpen(Session session, EndpointConfig endpointConfig) {
				onOpen_();
			}
			@Override
			public void onClose(Session session, CloseReason closeReason) {
				onClose_();
			}
			@Override
			public void onError(Session session, Throwable cause) {
				onError_(cause);
			}
		}, URI.create(serverURL));
		session_.addMessageHandler(this);
	}

	@Override
	protected void disconnect_() {
		if (session_ != null) {
			try {
				session_.close();
			} catch (IOException ee) {}
			session_ = null;
		}
	}

	@Override
	protected boolean isConnected_() {
		return (session_ != null);
	}

	@Override
	protected void sendRequest_(byte[] data, int dataOffset, int dataBytes, byte command) throws IOException {
		if (command == (byte)0) {
			session_.getBasicRemote().sendBinary(ByteBuffer.wrap(data, dataOffset, dataBytes));
		} else {
			if (outData_.length < 1 + dataBytes) {
				outData_ = new byte[1 + dataBytes];
			}
			outData_[0] = command;
			System.arraycopy(data, dataOffset, outData_, 1, dataBytes);
			session_.getBasicRemote().sendBinary(ByteBuffer.wrap(outData_, 0, 1 + dataBytes));
		}
	}

	@Override
	public void onMessage(String message) {
		onMessage_(message.trim());
	}
}
