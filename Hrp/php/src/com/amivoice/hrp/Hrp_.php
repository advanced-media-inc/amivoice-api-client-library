<?php

namespace com\amivoice\hrp;

require_once("com/amivoice/hrp/Hrp.php");

class Hrp_ extends Hrp {
	private static $_serverCertificateVerification = true;

	private static function _basic($userName, $password) {
		return base64_encode($userName . ":" . $password);
	}

	private $host_;
	private $resource_;
	private $proxyUserName_;
	private $proxyPassword_;
	private $socket_;
	private $requestBodyChunked_;
	private $requestBodyParted_;
	private $requestBodyPartedOffset_;
	private $inData_;
	private $inDataBytes_;
	private $inDataBytes__;
	private $inDataEncoding__;
	private $result__;
	private $subState_;

	public function __construct() {
		parent::__construct();
		$this->host_ = null;
		$this->resource_ = null;
		$this->proxyUserName_ = null;
		$this->proxyPassword_ = null;
		$this->socket_ = null;
		$this->requestBodyChunked_ = null;
		$this->requestBodyParted_ = null;
		$this->requestBodyPartedOffset_ = 0;
		$this->inData_ = "";
		$this->inDataBytes_ = 0;
		$this->inDataBytes__ = 0;
		$this->inDataEncoding__ = null;
		$this->result__ = null;
		$this->subState_ = 0;
	}

	protected function connect_($serverURL, $proxyServerName, $connectTimeout, $receiveTimeout) {
		$colonDoubleSlashIndex = strpos($serverURL, "://");
		if ($colonDoubleSlashIndex === false) {
			throw new \UnexpectedValueException("Invalid server URL");
		}
		$scheme = substr($serverURL, 0, $colonDoubleSlashIndex);
		if ($scheme !== "http" && $scheme !== "https") {
			throw new \UnexpectedValueException("Invalid scheme: " . $scheme);
		}
		$slashIndex = strpos($serverURL, '/', $colonDoubleSlashIndex + 3);
		if ($slashIndex !== false) {
			$hostName = substr($serverURL, $colonDoubleSlashIndex + 3, $slashIndex - ($colonDoubleSlashIndex + 3));
			$this->resource_ = substr($serverURL, $slashIndex);
		} else {
			$hostName = substr($serverURL, $colonDoubleSlashIndex + 3);
			$this->resource_ = "/";
		}
		$colonIndex = strpos($hostName, ':', ((($bracketIndex = strpos($hostName, ']')) !== false) ? $bracketIndex : -1) + 1);
		if ($colonIndex !== false) {
			$port = intval(substr($hostName, $colonIndex + 1));
			if ($port === 0) {
				throw new \UnexpectedValueException("Invalid port: " . substr($hostName, $colonIndex + 1));
			}
			$hostName = substr($hostName, 0, $colonIndex);
		} else {
			if ($scheme === "https") {
				$port = 443;
			} else {
				$port = 80;
			}
		}
		if ($port === 80) {
			$this->host_ = $hostName;
		} else {
			$this->host_ = $hostName . ":" . $port;
		}
		$proxyHostName = null;
		$proxyPort = 0;
		$this->proxyUserName_ = null;
		$this->proxyPassword_ = null;
		if ($proxyServerName !== null) {
			$proxyHostName = $proxyServerName;
			$atIndex = strrpos($proxyHostName, '@');
			if ($atIndex !== false) {
				$colonIndex = strpos($proxyHostName, ':');
				if ($colonIndex !== false && $colonIndex < $atIndex) {
					$this->proxyUserName_ = substr($proxyHostName, 0, $colonIndex);
					$this->proxyPassword_ = substr($proxyHostName, $colonIndex + 1, $atIndex - ($colonIndex + 1));
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
				$this->socket_ = @stream_socket_client((($scheme === "https") ? "tlsv1.2" : "tcp") . "://" . $hostName . ":" . $port, $errorNumber, $errorString, ($connectTimeout === 0) ? 60.0 : $connectTimeout / 1000.0, STREAM_CLIENT_CONNECT);
			} else {
				$this->socket_ = @stream_socket_client((($scheme === "https") ? "tlsv1.2" : "tcp") . "://" . $hostName . ":" . $port, $errorNumber, $errorString, ($connectTimeout === 0) ? 60.0 : $connectTimeout / 1000.0, STREAM_CLIENT_CONNECT, stream_context_create(array("ssl" => array("verify_peer" => false, "verify_peer_name" => false))));
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

	protected function sendRequestHeader_($domainId,
										  $codec,
										  $resultType,
										  $resultEncoding,
										  $serviceAuthorization,
										  $voiceDetection,
										  $acceptTopic,
										  $contentType,
										  $accept, $type, $dataBytes) {
		$this->requestBodyChunked_ = null;
		$this->requestBodyParted_ = null;
		$this->requestBodyPartedOffset_ = 0;
		$this->inDataBytes_ = 0;
		$this->inDataBytes__ = 0;
		$this->inDataEncoding__ = null;
		$this->result__ = "";
		$requestHeader = "";
		$requestHeader .= "POST ";
		$requestHeader .= $this->resource_;
		if ($type[0] !== 'm') {
			$delimiter = (strpos($this->resource_, '?') === false) ? '?' : '&';
			if ($domainId !== null && $domainId !== "") {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "d=";
				$requestHeader .= urlencode($domainId);
			}
			if ($codec !== null && $codec !== "") {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "c=";
				$requestHeader .= urlencode($codec);
			}
			if ($resultType !== null && $resultType !== "") {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "r=";
				$requestHeader .= urlencode($resultType);
			}
			if ($resultEncoding !== null && $resultEncoding !== "") {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "e=";
				$requestHeader .= urlencode($resultEncoding);
			}
			if ($serviceAuthorization !== null && $serviceAuthorization !== "") {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "u=";
				$requestHeader .= urlencode($serviceAuthorization);
			}
			if ($voiceDetection !== null) {
				$requestHeader .= $delimiter;
				$delimiter = '&';
				$requestHeader .= "v=";
				$requestHeader .= urlencode($voiceDetection);
			}
		} else {
			$requestBody = "";
			if ($domainId !== null && $domainId !== "") {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"d\"\r\n\r\n";
				$requestBody .= $domainId;
			}
			if ($codec !== null && $codec !== "") {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\n";
				$requestBody .= $codec;
			}
			if ($resultType !== null && $resultType !== "") {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"r\"\r\n\r\n";
				$requestBody .= $resultType;
			}
			if ($resultEncoding !== null && $resultEncoding !== "") {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"e\"\r\n\r\n";
				$requestBody .= $resultEncoding;
			}
			if ($serviceAuthorization !== null && $serviceAuthorization !== "") {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"u\"\r\n\r\n";
				$requestBody .= $serviceAuthorization;
			}
			if ($voiceDetection !== null) {
				$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"v\"\r\n\r\n";
				$requestBody .= $voiceDetection;
			}
			$requestBody .= "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\n";
			$requestBody .= "\r\n--1234567890ABCDEFGHIJ--\r\n";
			$this->requestBodyParted_ = $requestBody; // [70+n+][70+n+][70+n+][70+n+][70+n+][70+n+]70+28 bytes
		}
		$requestHeader .= " HTTP/1.1\r\n";
		$requestHeader .= "Host: ";
		$requestHeader .= $this->host_;
		$requestHeader .= "\r\n";
		$requestHeader .= "User-Agent: ";
		$requestHeader .= Hrp::getVersion();
		$requestHeader .= "\r\n";
		if ($this->proxyUserName_ !== null && $this->proxyPassword_ !== null) {
			$requestHeader .= "Proxy-Authorization: Basic ";
			$requestHeader .= self::_basic($this->proxyUserName_, $this->proxyPassword_);
			$requestHeader .= "\r\n";
		}
		if ($type[1] !== 'c') {
			$requestHeader .= "Content-Length: ";
			if ($this->requestBodyParted_ === null) {
				$requestHeader .= $dataBytes;
			} else {
				$requestHeader .= $dataBytes + strlen($this->requestBodyParted_);
			}
			$requestHeader .= "\r\n";
		} else {
			$this->requestBodyChunked_ = "0\r\n\r\n";
			$requestHeader .= "Transfer-Encoding: chunked\r\n";
		}
		if ($this->requestBodyParted_ === null) {
			if ($acceptTopic !== null && $acceptTopic !== "") {
				$requestHeader .= "Accept-Topic: ";
				$requestHeader .= $acceptTopic;
				$requestHeader .= "\r\n";
			}
			if ($contentType !== null && $contentType !== "") {
				$requestHeader .= "Content-Type: ";
				$requestHeader .= $contentType;
				$requestHeader .= "\r\n";
			}
			if ($accept !== null && $accept !== "") {
				$requestHeader .= "Accept: ";
				$requestHeader .= $accept;
				$requestHeader .= "\r\n";
			}
		} else {
			$requestHeader .= "Content-Type: multipart/form-data; boundary=1234567890ABCDEFGHIJ\r\n";
		}
		$requestHeader .= "\r\n";
		$this->write_($requestHeader);
		fflush($this->socket_);
	}

	protected function sendRequestBody_($data, $dataOffset, $dataBytes) {
		if ($data !== null && $dataBytes > 0) {
			if ($this->requestBodyParted_ !== null) {
				if ($this->requestBodyPartedOffset_ === 0) {
					$this->requestBodyPartedOffset_ = strlen($this->requestBodyParted_) - 28;
					if ($this->requestBodyChunked_ !== null) {
						$this->write_(dechex($this->requestBodyPartedOffset_));
						$this->write_($this->requestBodyChunked_, 1, 2);
					}
					$this->write_($this->requestBodyParted_, 0, $this->requestBodyPartedOffset_);
					if ($this->requestBodyChunked_ !== null) {
						$this->write_($this->requestBodyChunked_, 3, 2);
					}
				}
			}
			if ($this->requestBodyChunked_ !== null) {
				$this->write_(dechex($dataBytes));
				$this->write_($this->requestBodyChunked_, 1, 2);
			}
			$this->write_($data, $dataOffset, $dataBytes);
			if ($this->requestBodyChunked_ !== null) {
				$this->write_($this->requestBodyChunked_, 3, 2);
			}
			fflush($this->socket_);
		} else {
			if ($this->requestBodyParted_ !== null) {
				if ($this->requestBodyChunked_ !== null) {
					$this->write_(dechex(strlen($this->requestBodyParted_) - $this->requestBodyPartedOffset_));
					$this->write_($this->requestBodyChunked_, 1, 2);
				}
				$this->write_($this->requestBodyParted_, $this->requestBodyPartedOffset_, strlen($this->requestBodyParted_) - $this->requestBodyPartedOffset_);
				if ($this->requestBodyChunked_ !== null) {
					$this->write_($this->requestBodyChunked_, 3, 2);
					$this->write_($this->requestBodyChunked_, 0, 5);
				}
			} else {
				if ($this->requestBodyChunked_ !== null) {
					$this->write_($this->requestBodyChunked_, 0, 5);
				}
			}
			fflush($this->socket_);
		}
	}

	public function run() {
		try {
			while (true) {
				$sessionId = $this->receiveResponseHeader_();
				$this->onOpen_($sessionId);
				if ($this->inDataBytes__ > 0) {
					$this->receiveResponseBody_();
					$crlfIndex = strpos($this->result__, "\r\n");
					while ($crlfIndex !== false) {
						$result = substr($this->result__, 0, $crlfIndex);
						$this->onMessage_($result);
						$this->result__ = substr($this->result__, $crlfIndex + 2);
						$crlfIndex = strpos($this->result__, "\r\n");
					}
				} else {
					while ($this->receiveChunkedResponseBody_()) {
						$crlfIndex = strpos($this->result__, "\r\n");
						while ($crlfIndex !== false) {
							$result = substr($this->result__, 0, $crlfIndex);
							$this->onMessage_($result);
							$this->result__ = substr($this->result__, $crlfIndex + 2);
							$crlfIndex = strpos($this->result__, "\r\n");
						}
					}
				}
				$this->onClose_();
			}
		} catch (\Exception $e) {
			$this->onError_($e);
		}
	}

	protected function checkResponse_($timeout) {
		try {
			if ($this->subState_ === 0) {
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
				$sessionId = $this->receiveResponseHeader_();
				$this->onOpen_($sessionId);
				if ($this->inDataBytes__ > 0) {
					$this->subState_ = 1;
				} else {
					$this->subState_ = 2;
				}
			} else
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
				$this->receiveResponseBody_();
				$crlfIndex = strpos($this->result__, "\r\n");
				while ($crlfIndex !== false) {
					$result = substr($this->result__, 0, $crlfIndex);
					$this->onMessage_($result);
					$this->result__ = substr($this->result__, $crlfIndex + 2);
					$crlfIndex = strpos($this->result__, "\r\n");
				}
				$this->onClose_();
				$this->subState_ = 0;
			} else
			if ($this->subState_ === 2) {
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
				if ($this->receiveChunkedResponseBody_()) {
					$crlfIndex = strpos($this->result__, "\r\n");
					while ($crlfIndex !== false) {
						$result = substr($this->result__, 0, $crlfIndex);
						$this->onMessage_($result);
						$this->result__ = substr($this->result__, $crlfIndex + 2);
						$crlfIndex = strpos($this->result__, "\r\n");
					}
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

	private function receiveResponseHeader_() {
		$responseHeaderBytes = $this->read_CRLFCRLF_();
		if ($responseHeaderBytes === -1) {
			throw new \Exception("Unexpected end of stream");
		}
		$responseHeader = substr($this->inData_, 0, $responseHeaderBytes);
		if ($responseHeader[ 8] !== ' '
		 || $responseHeader[ 9] !== '2'
		 || $responseHeader[10] !== '0'
		 || $responseHeader[11] !== '0'
		 || $responseHeader[12] !== ' ') {
			throw new \Exception("Invalid response code: " . substr($responseHeader, 9, 12 - 9));
		}
		$index1 = strpos($responseHeader, "\r\nX-Session-ID: ");
		if ($index1 === false) {
			throw new \Exception("Missing session id");
		}
		$index1 += 16;
		$index2 = strpos($responseHeader, "\r\n", $index1);
		if ($index2 === false) {
			throw new \Exception("Missing session id");
		}
		$sessionId = substr($responseHeader, $index1, $index2 - $index1);
		$this->inDataEncoding__ = "UTF-8";
		$index1 = strpos($responseHeader, "\r\nContent-Type: ");
		if ($index1 !== false) {
			$index1 += 16;
			$index2 = strpos($responseHeader, "\r\n", $index1);
			if ($index2 !== false) {
				$index3 = strpos($responseHeader, "; charset=", $index1);
				if ($index3 !== false && $index3 < $index2) {
					$index3 += 10;
					$this->inDataEncoding__ = strtoupper(substr($responseHeader, $index3, $index2 - $index3));
					if ($this->inDataEncoding__ === "SHIFT_JIS") {
						$this->inDataEncoding__ = "MS932";
					}
				}
			}
		}
		$index1 = strpos($responseHeader, "\r\nContent-Length: ");
		if ($index1 !== false) {
			$index1 += 18;
			$index2 = strpos($responseHeader, "\r\n", $index1);
			if ($index2 !== false) {
				$this->inDataBytes__ = intval(substr($responseHeader, $index1, $index2 - $index1));
				if ($this->inDataBytes__ === 0) {
					throw new \Exception("Invalid content length: " . substr($responseHeader, $index1, $index2 - $index1));
				}
			}
		}
		$this->shift_($responseHeaderBytes);
		return $sessionId;
	}

	private function receiveResponseBody_() {
		if (!$this->read_($this->inDataBytes__)) {
			throw new \Exception("Unexpected end of stream");
		}
		$this->result_($this->inDataBytes__);
		$this->shift_($this->inDataBytes__);
	}

	private function receiveChunkedResponseBody_() {
		$chunkedResponseHeaderBytes = $this->read_CRLF_();
		if ($chunkedResponseHeaderBytes === -1) {
			throw new \Exception("Unexpected end of stream");
		}
		$this->inDataBytes__ = hexdec(substr($this->inData_, 0, $chunkedResponseHeaderBytes));
		$this->shift_($chunkedResponseHeaderBytes);
		if ($this->inDataBytes__ === 0) {
			if (!$this->read_(2)) {
				throw new \Exception("Unexpected end of stream");
			}
			$this->shift_(2);
			return false;
		}
		if (!$this->read_($this->inDataBytes__ + 2)) {
			throw new \Exception("Unexpected end of stream");
		}
		$this->result_($this->inDataBytes__);
		$this->shift_($this->inDataBytes__ + 2);
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

	private function read_CRLF_() {
		$inDataBytes = $this->find_CRLF_();
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
			$inDataBytes = $this->find_CRLF_();
		}
		return $inDataBytes + 2;
	}

	private function find_CRLF_() {
		$inDataBytes = strpos($this->inData_, "\r\n");
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
		$result = substr($this->inData_, 0, $inDataBytes);
		if ($this->inDataEncoding__ !== "UTF-8") {
			$result = mb_convert_encoding($result, "UTF-8", $this->inDataEncoding__);
		}
		$this->result__ .= $result;
	}

	private function shift_($inDataBytes) {
		$this->inDataBytes_ -= $inDataBytes;
		$this->inData_ = substr($this->inData_, $inDataBytes);
	}
}

?>
