import sys
import threading
import time

try:
	from urllib import quote
	def u(s):
		if isinstance(s, str):
			return s.decode(sys.stdin.encoding)
		return s
	def q(s):
		return quote(s.encode("UTF-8"))
except:
	from urllib.parse import quote
	def u(s):
		return s
	def q(s):
		return quote(s, "UTF-8")

class Wrp(object):
	VERSION = "Wrp/1.0.03 Python/%d.%d.%d (%s)" % (sys.version_info[0], sys.version_info[1], sys.version_info[2], sys.platform)

	@staticmethod
	def getVersion():
		return Wrp.VERSION

	@staticmethod
	def construct(implementation = 1):
		if implementation == 1:
			from .Wrp_ import Wrp_
			return Wrp_()
		else:
			raise ValueError("Unknown implementation: " + implementation)

	def __init__(self):
		self.condition_ = threading.Condition()
		self.listener_ = None
		self.serverURL_ = None
		self.proxyServerName_ = None
		self.connectTimeout_ = 0
		self.receiveTimeout_ = 0
		self.grammarFileNames_ = None
		self.profileId_ = None
		self.profileWords_ = None
		self.segmenterProperties_ = None
		self.keepFillerToken_ = None
		self.resultUpdatedInterval_ = None
		self.extension_ = None
		self.authorization_ = None
		self.codec_ = None
		self.resultType_ = None
		self.state_ = 0
		self.waitingResults_ = 0

	def setListener(self, listener):
		self.listener_ = listener

	def setServerURL(self, serverURL):
		self.serverURL_ = u(serverURL)
		if self.serverURL_ != None:
			if self.serverURL_.startswith("http://"):
				self.serverURL_ = "ws://" + self.serverURL_[7:]
			elif self.serverURL_.startswith("https://"):
				self.serverURL_ = "wss://" + self.serverURL_[8:]

	def setProxyServerName(self, proxyServerName):
		self.proxyServerName_ = proxyServerName

	def setConnectTimeout(self, connectTimeout):
		self.connectTimeout_ = connectTimeout

	def setReceiveTimeout(self, receiveTimeout):
		self.receiveTimeout_ = receiveTimeout

	def setGrammarFileNames(self, grammarFileNames):
		self.grammarFileNames_ = u(grammarFileNames)

	def setProfileId(self, profileId):
		self.profileId_ = u(profileId)

	def setProfileWords(self, profileWords):
		self.profileWords_ = u(profileWords)

	def setSegmenterProperties(self, segmenterProperties):
		self.segmenterProperties_ = u(segmenterProperties)

	def setKeepFillerToken(self, keepFillerToken):
		self.keepFillerToken_ = keepFillerToken

	def setExtension(self, extension):
		self.extension_ = u(extension)

	def setAuthorization(self, authorization):
		self.authorization_ = u(authorization)

	def setCodec(self, codec):
		self.codec_ = u(codec)

	def setResultType(self, resultType):
		self.resultType_ = u(resultType)

	def setServiceAuthorization(self, serviceAuthorization):
		if serviceAuthorization != None:
			self.authorization_ = u(serviceAuthorization)

	def connect(self):
		if self.isConnected_():
			self.lastMessage_ = "WARNING: already connected to WebSocket server"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		if self.serverURL_ == None:
			self.lastMessage_ = "ERROR: can't connect to WebSocket server (Missing server URL)"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		try:
			self.state_ = 0
			self.waitingResults_ = 0
			self.connect_(self.serverURL_, self.proxyServerName_, self.connectTimeout_, self.receiveTimeout_)
		except Exception as e:
			self.lastMessage_ = "ERROR: can't connect to WebSocket server (" + u(str(e)) + "): " + self.serverURL_
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			self.disconnect_()
			return False
		self.lastMessage_ = "INFO: connected to WebSocket server: " + self.serverURL_
		if self.listener_ != None:
			self.listener_.TRACE(self.lastMessage_)
		return True

	def connect_(self, serverURL, proxyServerName, connectTimeout, receiveTimeout):
		raise NotImplementedError()

	def disconnect(self):
		if not self.isConnected_():
			self.lastMessage_ = "WARNING: already disconnected from WebSocket server"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		self.disconnect_()
		self.lastMessage_ = "INFO: disconnected from WebSocket server"
		if self.listener_ != None:
			self.listener_.TRACE(self.lastMessage_)
		return True

	def disconnect_(self):
		raise NotImplementedError()

	def isConnected(self):
		return self.isConnected_()

	def isConnected_(self):
		raise NotImplementedError()

	def feedDataResume(self):
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 0:
				self.lastMessage_ = "WARNING: already started feeding data to WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			command = ""
			command += "s "
			if self.codec_ != None and len(self.codec_) > 0:
				command += self.codec_
			else:
				command += "16K"
			if self.grammarFileNames_ != None and len(self.grammarFileNames_) > 0:
				command += ' '
				command += self.grammarFileNames_
				if self.grammarFileNames_.find('\001') != -1 and not self.grammarFileNames_.endswith("\001"):
					command += '\001'
			else:
				command += " \001"
			if self.profileId_ != None:
				command += " profileId="
				if self.profileId_.find(' ') != -1:
					command += '"'
					command += self.profileId_
					command += '"'
				else:
					command += self.profileId_
			if self.profileWords_ != None:
				command += " profileWords="
				if self.profileWords_.find(' ') != -1:
					command += '"'
					command += self.profileWords_
					command += '"'
				else:
					command += self.profileWords_
			if self.segmenterProperties_ != None:
				command += " segmenterProperties="
				if self.segmenterProperties_.find(' ') != -1:
					command += '"'
					command += self.segmenterProperties_
					command += '"'
				else:
					command += self.segmenterProperties_
			if self.keepFillerToken_ != None:
				command += " keepFillerToken="
				if self.keepFillerToken_.find(' ') != -1:
					command += '"'
					command += self.keepFillerToken_
					command += '"'
				else:
					command += self.keepFillerToken_
			if self.resultUpdatedInterval_ != None:
				command += " resultUpdatedInterval="
				if self.resultUpdatedInterval_.find(' ') != -1:
					command += '"'
					command += self.resultUpdatedInterval_
					command += '"'
				else:
					command += self.resultUpdatedInterval_
			if self.extension_ != None:
				command += " extension="
				if self.extension_.find(' ') != -1:
					command += '"'
					command += self.extension_.replace("\"", "\"\"")
					command += '"'
				else:
					command += self.extension_
			if self.authorization_ != None:
				command += " authorization="
				if self.authorization_.find(' ') != -1:
					command += '"'
					command += self.authorization_
					command += '"'
				else:
					command += self.authorization_
			if self.resultType_ != None:
				command += " resultType="
				if self.resultType_.find(' ') != -1:
					command += '"'
					command += self.resultType_
					command += '"'
				else:
					command += self.resultType_
			try:
				self.state_ = 1
				data = command.encode("UTF-8")
				self.sendRequest_(data, 0, len(data), 0)
				while self.state_ == 1:
					self.condition_.wait()
				if self.state_ == 5:
					return False
			except InterruptedError:
				self.lastMessage_ = "WARNING: interrupted to start feeding data to WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def feedData(self, data, dataOffset, dataBytes):
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 2 and self.state_ != 3:
				self.lastMessage_ = "WARNING: already stopped feeding data to WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			try:
				self.state_ = 3
				self.sendRequest_(data, dataOffset, dataBytes, 0x70) # 'p'
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't feed data to WebSocket server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def feedDataPause(self):
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 2 and self.state_ != 3:
				self.lastMessage_ = "WARNING: already stopped feeding data to WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			try:
				self.state_ = 4
				data = b"e"
				self.sendRequest_(data, 0, len(data), 0)
				while self.state_ == 4:
					self.condition_.wait()
				if self.state_ == 5:
					return False
			except InterruptedError:
				self.lastMessage_ = "WARNING: interrupted to stop feeding data to WebSocket server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def getLastMessage(self):
		return self.lastMessage_

	def getWaitingResults(self):
		return self.waitingResults_

	def sleep(self, timeout):
		try:
			time.sleep(timeout / 1000.0)
		except:
			pass

	def sendRequest_(self, data, dataOffset, dataBytes, command):
		raise NotImplementedError()

	def onOpen_(self):
		pass

	def onClose_(self):
		pass

	def onError_(self, cause):
		with self.condition_:
			if self.state_ == 0 or self.state_ == 5:
				return
			self.lastMessage_ = "ERROR: caught exception (" + u(str(cause)) + ")"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			self.state_ = 5
			self.condition_.notifyAll()

	def onMessage_(self, message):
		if len(message) == 0:
			return
		command = message[0]
		if command == 's':
			if len(message) > 1:
				with self.condition_:
					self.lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + message[2:] + ")"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					self.state_ = 5
					self.condition_.notifyAll()
			else:
				with self.condition_:
					self.lastMessage_ = "INFO: started feeding data to WebSocket server"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					self.state_ = 2
					self.condition_.notifyAll()
		elif command == 'p':
			if len(message) > 1:
				with self.condition_:
					self.lastMessage_ = "ERROR: can't feed data to WebSocket server (" + message[2:] + ")"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					self.state_ = 5
					self.condition_.notifyAll()
			else:
				pass
		elif command == 'e':
			if len(message) > 1:
				with self.condition_:
					self.lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + message[2:] + ")"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					self.state_ = 5
					self.condition_.notifyAll()
			else:
				with self.condition_:
					self.lastMessage_ = "INFO: stopped feeding data to WebSocket server"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					self.state_ = 0
					self.condition_.notifyAll()
		elif command == 'S':
			self.waitingResults_ += 1
			if self.listener_ != None:
				self.listener_.utteranceStarted(int(message[2:]))
		elif command == 'E':
			if self.listener_ != None:
				self.listener_.utteranceEnded(int(message[2:]))
		elif command == 'C':
			if self.listener_ != None:
				self.listener_.resultCreated()
		elif command == 'U':
			if self.listener_ != None:
				self.listener_.resultUpdated(message[2:])
		elif command == 'A':
			if self.listener_ != None:
				self.listener_.resultFinalized(message[2:])
			self.waitingResults_ -= 1
		elif command == 'R':
			if self.listener_ != None:
				self.listener_.resultFinalized("\001\001\001\001\001" + message[2:])
			self.waitingResults_ -= 1
		elif command == 'Q':
			if self.listener_ != None:
				self.listener_.eventNotified(command, message[2:])
		elif command == 'G':
			if self.listener_ != None:
				self.listener_.eventNotified(command, message[2:])
