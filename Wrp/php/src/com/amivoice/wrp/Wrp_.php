<?php

namespace com\amivoice\wrp;

require_once("com/amivoice/wrp/Wrp.php");

class Wrp_ extends Wrp {
	private static $_serverCertificateVerification = true;

	private static $_base64Table = array(
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	);

	private static function _basic($userName, $password) {
		return base64_encode($userName . ":" . $password);
	}

	private $socket_;
	private $outData_;
	private $inData_;
	private $inDataBytes_;
	private $result__;
	private $subState_;

	public function __construct() {
		parent::__construct();
		$this->socket_ = null;
		$this->outData_ = "";
		$this->inData_ = "";
		$this->inDataBytes_ = 0;
		$this->result__ = null;
		$this->subState_ = 0;
	}

	protected function connect_($serverURL, $proxyServerName, $connectTimeout, $receiveTimeout) {
		$colonDoubleSlashIndex = strpos($serverURL, "://");
		if ($colonDoubleSlashIndex === false) {
			throw new \UnexpectedValueException("Invalid server URL");
		}
		$scheme = substr($serverURL, 0, $colonDoubleSlashIndex);
		if ($scheme !== "ws" && $scheme !== "wss") {
			throw new \UnexpectedValueException("Invalid scheme: " . $scheme);
		}
		$slashIndex = strpos($serverURL, '/', $colonDoubleSlashIndex + 3);
		if ($slashIndex !== false) {
			$hostName = substr($serverURL, $colonDoubleSlashIndex + 3, $slashIndex - ($colonDoubleSlashIndex + 3));
			$resource = substr($serverURL, $slashIndex);
		} else {
			$hostName = substr($serverURL, $colonDoubleSlashIndex + 3);
			$resource = "/";
		}
		$colonIndex = strpos($hostName, ':', ((($bracketIndex = strpos($hostName, ']')) !== false) ? $bracketIndex : -1) + 1);
		if ($colonIndex !== false) {
			$port = intval(substr($hostName, $colonIndex + 1));
			if ($port === 0) {
				throw new \UnexpectedValueException("Invalid port: " . substr($hostName, $colonIndex + 1));
			}
			$hostName = substr($hostName, 0, $colonIndex);
		} else {
			if ($scheme === "wss") {
				$port = 443;
			} else {
				$port = 80;
			}
		}
		if ($port === 80) {
			$host = $hostName;
		} else {
			$host = $hostName . ":" . $port;
		}
		$proxyHostName = null;
		$proxyPort = 0;
		$proxyUserName = null;
		$proxyPassword = null;
		if ($proxyServerName !== null) {
			$proxyHostName = $proxyServerName;
			$atIndex = strrpos($proxyHostName, '@');
			if ($atIndex !== false) {
				$colonIndex = strpos($proxyHostName, ':');
				if ($colonIndex !== false && $colonIndex < $atIndex) {
					$proxyUserName = substr($proxyHostName, 0, $colonIndex);
					$proxyPassword = substr($proxyHostName, $colonIndex + 1, $atIndex - ($colonIndex + 1));
					$proxyHostName = substr($proxyHostName, $atIndex + 1);
				}
			}
			$colonIndex = strpos($proxyHostName, ':', ((($bracketIndex = strpos($proxyHostName, ']')) !== false) ? $bracketIndex : -1) + 1);
			if ($colonIndex !== false) {
				$proxyPort = intval(substr($proxyHostName, $colonIndex + 1));
				if ($proxyPort === 0) {
					throw new \UnexpectedValueException("Invalid port: " . substr($proxyHostName, $colonIndex + 1));
				}
				$proxyHostName = substr($proxyHostName, 0, $colonIndex);
			} else {
				$proxyPort = 80;
			}
		}
		if ($proxyServerName !== null) {
			$this->socket_ = @stream_socket_client("tcp://" . $proxyHostName . ":" . $proxyPort, $errorNumber, $errorString, ($connectTimeout === 0) ? 60.0 : $connectTimeout / 1000.0, STREAM_CLIENT_CONNECT);
			$this->resource_ = $serverURL;
		} else {
			if (self::$_serverCertificateVerification) {
				$this->socket_ = @stream_socket_client((($scheme === "wss") ? "tlsv1.2" : "tcp") . "://" . $hostName . ":" . $port, $errorNumber, $errorString, ($connectTimeout === 0) ? 60.0 : $connectTimeout / 1000.0, STREAM_CLIENT_CONNECT);
			} else {
				$this->socket_ = @stream_socket_client((($scheme === "wss") ? "tlsv1.2" : "tcp") . "://" . $hostName . ":" . $port, $errorNumber, $errorString, ($connectTimeout === 0) ? 60.0 : $connectTimeout / 1000.0, STREAM_CLIENT_CONNECT, stream_context_create(array("ssl" => array("verify_peer" => false, "verify_peer_name" => false))));
			}
		}
		if ($this->socket_ === false) {
			$this->socket_ = null;
			if (PATH_SEPARATOR !== ';') {
				$errorString = trim($errorString);
			} else {
				$errorString = trim(mb_convert_encoding($errorString, "UTF-8", "MS932"));
			}
			throw new \Exception($errorString);
		}
		stream_set_timeout($this->socket_, intval($receiveTimeout / 1000), ($receiveTimeout % 1000) * 1000);
		$this->inDataBytes_ = 0;
		$this->result__ = null;
		// -----------------------------------------------------------
		// GET <resource> HTTP/1.1
		// Host: <hostName>:<port>
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Key: ...
		// 
		// -----------------------------------------------------------
		$requestHeader = "";
		$requestHeader .= "GET ";
		$requestHeader .= $resource;
		$requestHeader .= " HTTP/1.1\r\n";
		$requestHeader .= "User-Agent: ";
		$requestHeader .= Wrp::getVersion();
		$requestHeader .= "\r\n";
		$requestHeader .= "Host: ";
		$requestHeader .= $host;
		$requestHeader .= "\r\n";
		$requestHeader .= "Upgrade: websocket\r\n";
		$requestHeader .= "Connection: upgrade\r\n";
		$requestHeader .= "Sec-WebSocket-Key: ";
		for ($i = 0; $i < 20; $i += 4) {
			$twentyFourBits = mt_rand(0, 0x7FFFFFFF) & 0x3F3F3F3F;
			$requestHeader .= self::$_base64Table[($twentyFourBits >> 24) & 0x3F];
			$requestHeader .= self::$_base64Table[($twentyFourBits >> 16) & 0x3F];
			$requestHeader .= self::$_base64Table[($twentyFourBits >>  8) & 0x3F];
			$requestHeader .= self::$_base64Table[($twentyFourBits      ) & 0x3F];
		}
		$eightBits = mt_rand(0, 0x7FFFFFFF) & 0x3F300000;
		$requestHeader .= self::$_base64Table[($eightBits >> 24) & 0x3F];
		$requestHeader .= self::$_base64Table[($eightBits >> 16) & 0x3F];
		$requestHeader .= "==\r\n";
		$requestHeader .= "\r\n";
		$this->write_($requestHeader);
		fflush($this->socket_);
		// -----------------------------------------------------------
		// HTTP/1.1 101 Switching Protocols
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Accept: ...
		// 
		// -----------------------------------------------------------
		$responseHeaderBytes = $this->read_CRLFCRLF_();
		if ($responseHeaderBytes === -1) {
			throw new \Exception("Unexpected end of stream");
		}
		$responseHeader = substr($this->inData_, 0, $responseHeaderBytes);
		if ($responseHeader[ 8] !== ' '
		 || $responseHeader[ 9] !== '1'
		 || $responseHeader[10] !== '0'
		 || $responseHeader[11] !== '1'
		 || $responseHeader[12] !== ' ') {
			throw new \Exception("Handshake error (invalld response code: " . substr($responseHeader, 9, 12 - 9) . ")");
		}
		$this->shift_($responseHeaderBytes);
	}

	protected function disconnect_() {
		if ($this->socket_ !== null) {
			fclose($this->socket_);
			$this->socket_ = null;
		}
	}

	protected function isConnected_() {
		return ($this->socket_ !== null);
	}

	protected function sendRequest_($data, $dataOffset, $dataBytes, $command) {
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
		$realDataBytes = (($command === 0) ? 0 : 1) + $dataBytes;
		$this->outData_ = "";
		$this->outData_ .= chr(0x80 | (($command === 0) ? /* Text frame */ 0x01 : /* Binary frame */ 0x02));
		if ($realDataBytes < 126) {
			$this->outData_ .= chr($realDataBytes);
		} else
		if ($realDataBytes < 65536) {
			$this->outData_ .= chr(126);
			$this->outData_ .= chr(($realDataBytes >>  8) & 0xFF);
			$this->outData_ .= chr(($realDataBytes      ) & 0xFF);
		} else {
			$this->outData_ .= chr(127);
			$this->outData_ .= chr(0);
			$this->outData_ .= chr(0);
			$this->outData_ .= chr(0);
			$this->outData_ .= chr(0);
			$this->outData_ .= chr(($realDataBytes >> 24) & 0xFF);
			$this->outData_ .= chr(($realDataBytes >> 16) & 0xFF);
			$this->outData_ .= chr(($realDataBytes >>  8) & 0xFF);
			$this->outData_ .= chr(($realDataBytes      ) & 0xFF);
		}
		if ($command === 0) {
			// pass
		} else {
			$this->outData_ .= $command;
		}
		$this->outData_ .= substr($data, $dataOffset, $dataBytes);
		$this->write_($this->outData_);
		fflush($this->socket_);
	}

	public function run() {
		try {
			$this->onOpen_();
			while ($this->receiveResponse_()) {
				$this->onMessage_($this->result__);
			}
			$this->onClose_();
		} catch (\Exception $e) {
			$this->onError_($e);
		}
	}

	protected function checkResponse_($timeout) {
		try {
			if ($this->subState_ === 0) {
				$this->onOpen_();
				$this->subState_ = 1;
			}
			if ($this->subState_ === 1) {
				if ($timeout >= 0) {
					$read = array($this->socket_);
					$write = null;
					$except = null;
					$selected = stream_select($read, $write, $except, 0, $timeout * 1000);
					if ($selected === false) {
						throw new \Exception("Can't read data");
					}
					if ($selected === 0) {
						return;
					}
				}
				if ($this->receiveResponse_()) {
					$this->onMessage_($this->result__);
				} else {
					$this->onClose_();
					$this->subState_ = 0;
				}
			}
		} catch (\Exception $e) {
			$this->onError_($e);
			$this->subState_ = 9;
		}
	}

	private function receiveResponse_() {
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
		if (!$this->read_(2)) {
			if ($this->inDataBytes_ === 0) {
				return false;
			}
			throw new \Exception("Unexpected end of stream");
		}
		$inDataBytes = (ord($this->inData_[1]) & 0x7F);
		$this->shift_(2);
		if ($inDataBytes === 126) {
			if (!$this->read_(2)) {
				throw new \Exception("Unexpected end of stream");
			}
			$inDataBytes = ((ord($this->inData_[0]) & 0xFF) <<  8)
						 | ((ord($this->inData_[1]) & 0xFF)      );
			$this->shift_(2);
		} else
		if ($inDataBytes === 127) {
			if (!$this->read_(8)) {
				throw new \Exception("Unexpected end of stream");
			}
			if (ord($this->inData_[0]) !== 0
			 || ord($this->inData_[1]) !== 0
			 || ord($this->inData_[2]) !== 0
			 || ord($this->inData_[3]) !== 0 || (ord($this->inData_[4]) & 0x80) !== 0) {
				throw new \Exception("Invalid payload length: " . $inDataBytes);
			}
			$inDataBytes = ((ord($this->inData_[4]) & 0xFF) << 24)
						 | ((ord($this->inData_[5]) & 0xFF) << 16)
						 | ((ord($this->inData_[6]) & 0xFF) <<  8)
						 | ((ord($this->inData_[7]) & 0xFF)      );
			$this->shift_(8);
		}
		if ($inDataBytes < 0) {
			throw new \Exception("Invalid payload length: " . $inDataBytes);
		}
		if (!$this->read_($inDataBytes)) {
			throw new \Exception("Unexpected end of stream");
		}
		$this->result_($inDataBytes);
		$this->shift_($inDataBytes);
		return true;
	}

	private function write_($data, $dataOffset = 0, $dataLength = 0) {
		if ($dataLength === 0) {
			$dataLength = strlen($data) - $dataOffset;
		}
		$dataWrittenBytes = 0;
		while ($dataLength > 0) {
			$dataCurrentWrittenBytes = @fwrite($this->socket_, substr($data, $dataOffset, $dataLength));
			if ($dataCurrentWrittenBytes === false) {
				throw new \Exception("Can't write data");
			}
			$dataOffset += $dataCurrentWrittenBytes;
			$dataLength -= $dataCurrentWrittenBytes;
			$dataWrittenBytes += $dataCurrentWrittenBytes;
		}
		return $dataWrittenBytes;
	}

	private function read_CRLFCRLF_() {
		$inDataBytes = $this->find_CRLFCRLF_();
		while ($inDataBytes === -1) {
			if (feof($this->socket_)) {
				return -1;
			}
			$inData = @fread($this->socket_, 4096);
			if ($inData === false) {
				return -1;
			}
			$this->inData_ .= $inData;
			$this->inDataBytes_ += strlen($inData);
			$inDataBytes = $this->find_CRLFCRLF_();
		}
		return $inDataBytes + 4;
	}

	private function find_CRLFCRLF_() {
		$inDataBytes = strpos($this->inData_, "\r\n\r\n");
		if ($inDataBytes === false) {
			return -1;
		}
		return $inDataBytes;
	}

	private function read_($inDataBytes) {
		while ($this->inDataBytes_ < $inDataBytes) {
			if (feof($this->socket_)) {
				return false;
			}
			$inData = @fread($this->socket_, $inDataBytes - $this->inDataBytes_);
			if ($inData === false) {
				return false;
			}
			$this->inData_ .= $inData;
			$this->inDataBytes_ += strlen($inData);
		}
		return true;
	}

	private function result_($inDataBytes) {
		if ($inDataBytes > 0 && $this->inData_[$inDataBytes - 1] === "\n") {
			$inDataBytes--;
			if ($inDataBytes > 0 && $this->inData_[$inDataBytes - 1] === "\r") {
				$inDataBytes--;
			}
		}
		$result = substr($this->inData_, 0, $inDataBytes);
		$this->result__ = $result;
	}

	private function shift_($inDataBytes) {
		$this->inDataBytes_ -= $inDataBytes;
		$this->inData_ = substr($this->inData_, $inDataBytes);
	}
}

?>
