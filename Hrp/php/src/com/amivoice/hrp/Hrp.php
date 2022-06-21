<?php

namespace com\amivoice\hrp;

require_once("com/amivoice/hrp/Hrp_.php");
require_once("com/amivoice/hrp/HrpListener.php");

abstract class Hrp {
	private static $VERSION = null;

	public static function getVersion() {
		if (self::$VERSION === null) {
			self::$VERSION = "Hrp/1.0.03 PHP/" . phpversion() . " (" . php_uname('s') . " " . php_uname('r') . " " . php_uname('v') . ")";
		}
		return self::$VERSION;
	}

	public static function construct($implementation = 1) {
		if ($implementation === 1) {
			return new Hrp_();
		} else {
			throw new \UnexpectedValueException("Unknown implementation: " . $implementation);
		}
	}

	private $listener_;
	private $serverURL_;
	private $proxyServerName_;
	private $connectTimeout_;
	private $receiveTimeout_;
	private $grammarFileNames_;
	private $profileId_;
	private $profileWords_;
	private $segmenterProperties_;
	private $keepFillerToken_;
	private $resultUpdatedInterval_;
	private $extension_;
	private $authorization_;
	private $domainId_;
	private $codec_;
	private $resultType_;
	private $resultEncoding_;
	private $serviceAuthorization_;
	private $voiceDetection_;
	private $acceptTopic_;
	private $contentType_;
	private $accept_;
	private $state_;
	private $lastMessage_;

	protected function __construct() {
		$this->listener_ = null;
		$this->serverURL_ = null;
		$this->proxyServerName_ = null;
		$this->connectTimeout_ = 0;
		$this->receiveTimeout_ = 0;
		$this->grammarFileNames_ = null;
		$this->profileId_ = null;
		$this->profileWords_ = null;
		$this->segmenterProperties_ = null;
		$this->keepFillerToken_ = null;
		$this->resultUpdatedInterval_ = null;
		$this->extension_ = null;
		$this->authorization_ = null;
		$this->domainId_ = null;
		$this->codec_ = null;
		$this->resultType_ = null;
		$this->resultEncoding_ = null;
		$this->serviceAuthorization_ = null;
		$this->voiceDetection_ = null;
		$this->acceptTopic_ = null;
		$this->contentType_ = null;
		$this->accept_ = null;
		$this->state_ = 0;
	}

	public function setListener($listener) {
		$this->listener_ = $listener;
	}

	public function setServerURL($serverURL) {
		$this->serverURL_ = $serverURL;
	}

	public function setProxyServerName($proxyServerName) {
		$this->proxyServerName_ = $proxyServerName;
	}

	public function setConnectTimeout($connectTimeout) {
		$this->connectTimeout_ = $connectTimeout;
	}

	public function setReceiveTimeout($receiveTimeout) {
		$this->receiveTimeout_ = $receiveTimeout;
	}

	public function setGrammarFileNames($grammarFileNames) {
		$this->grammarFileNames_ = $grammarFileNames;
	}

	public function setProfileId($profileId) {
		$this->profileId_ = $profileId;
	}

	public function setProfileWords($profileWords) {
		$this->profileWords_ = $profileWords;
	}

	public function setSegmenterProperties($segmenterProperties) {
		$this->segmenterProperties_ = $segmenterProperties;
	}

	public function setKeepFillerToken($keepFillerToken) {
		$this->keepFillerToken_ = $keepFillerToken;
	}

	public function setResultUpdatedInterval($resultUpdatedInterval) {
		$this->resultUpdatedInterval_ = $resultUpdatedInterval;
	}

	public function setExtension($extension) {
		$this->extension_ = $extension;
	}

	public function setAuthorization($authorization) {
		$this->authorization_ = $authorization;
	}

	public function setDomainId($domainId) {
		$this->domainId_ = $domainId;
	}

	public function setCodec($codec) {
		$this->codec_ = $codec;
	}

	public function setResultType($resultType) {
		$this->resultType_ = $resultType;
	}

	public function setResultEncoding($resultEncoding) {
		$this->resultEncoding_ = $resultEncoding;
	}

	public function setServiceAuthorization($serviceAuthorization) {
		$this->serviceAuthorization_ = $serviceAuthorization;
	}

	public function setVoiceDetection($voiceDetection) {
		$this->voiceDetection_ = $voiceDetection;
	}

	public function setAcceptTopic($acceptTopic) {
		$this->acceptTopic_ = $acceptTopic;
	}

	public function setContentType($contentType) {
		$this->contentType_ = $contentType;
	}

	public function setAccept($accept) {
		$this->accept_ = $accept;
	}

	public function connect() {
		if ($this->isConnected_()) {
			$this->lastMessage_ = "WARNING: already connected to HTTP server";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		if ($this->serverURL_ === null) {
			$this->lastMessage_ = "ERROR: can't connect to HTTP server (Missing server URL)";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		try {
			$this->state_ = 0;
			$this->connect_($this->serverURL_, $this->proxyServerName_, $this->connectTimeout_, $this->receiveTimeout_);
		} catch (\Exception $e) {
			$this->lastMessage_ = "ERROR: can't connect to HTTP server (" . $e->getMessage() . "): " . $this->serverURL_;
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			$this->disconnect_();
			return false;
		}
		$this->lastMessage_ = "INFO: connected to HTTP server: " . $this->serverURL_;
		if ($this->listener_ !== null) {
			$this->listener_->TRACE($this->lastMessage_);
		}
		return true;
	}

	protected abstract function connect_($serverURL, $proxyServerName, $connectTimeout, $receiveTimeout);

	public function disconnect() {
		if (!$this->isConnected_()) {
			$this->lastMessage_ = "WARNING: already disconnected from HTTP server";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		$this->disconnect_();
		$this->lastMessage_ = "INFO: disconnected from HTTP server";
		if ($this->listener_ !== null) {
			$this->listener_->TRACE($this->lastMessage_);
		}
		return true;
	}

	protected abstract function disconnect_();

	public function isConnected() {
		return $this->isConnected_();
	}

	protected abstract function isConnected_();

	// type="s"  dataBytes>0 - Single part & no chunked encoding
	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public function feedDataResume($type = "mc", $dataBytes = 0) {
		if ($type === null) {
			$type = "mc";
		}
		if ($dataBytes > 0) {
			if (strpos($type, 'm') === false) {
				if (strpos($type, 'c') === false) {
					$type = "s ";
				} else {
					$type = "sc";
				}
			} else {
				if (strpos($type, 'c') === false) {
					$type = "m ";
				} else {
					$type = "mc";
				}
			}
		} else {
			if (strpos($type, 'm') === false) {
				$type = "sc";
			} else {
				$type = "mc";
			}
		}
		/* synchronized ($this) */ {
			if (!$this->isConnected_()) {
				$this->lastMessage_ = "WARNING: already disconnected from HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 0) {
				$this->lastMessage_ = "WARNING: already started feeding data to HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			$domainId = "";
			if ($this->domainId_ !== null) {
				$domainId .= $this->domainId_;
			} else {
				if ($this->grammarFileNames_ !== null) {
					$domainId .= "grammarFileNames=";
					$domainId .= rawurlencode($this->grammarFileNames_);
				}
				if ($this->profileId_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "profileId=";
					$domainId .= rawurlencode($this->profileId_);
				}
				if ($this->profileWords_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "profileWords=";
					$domainId .= rawurlencode($this->profileWords_);
				}
				if ($this->segmenterProperties_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "segmenterProperties=";
					$domainId .= rawurlencode($this->segmenterProperties_);
				}
				if ($this->keepFillerToken_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "keepFillerToken=";
					$domainId .= rawurlencode($this->keepFillerToken_);
				}
				if ($this->resultUpdatedInterval_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "resultUpdatedInterval=";
					$domainId .= rawurlencode($this->resultUpdatedInterval_);
				}
				if ($this->extension_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "extension=";
					$domainId .= rawurlencode($this->extension_);
				}
				if ($this->authorization_ !== null) {
					if ($domainId !== "") {
						$domainId .= ' ';
					}
					$domainId .= "authorization=";
					$domainId .= rawurlencode($this->authorization_);
				}
			}
			try {
				$this->state_ = 1;
				$this->sendRequestHeader_($domainId,
										  $this->codec_,
										  $this->resultType_,
										  $this->resultEncoding_,
										  $this->serviceAuthorization_,
										  $this->voiceDetection_,
										  $this->acceptTopic_,
										  $this->contentType_,
										  $this->accept_, $type, $dataBytes);
				$this->state_ = 2;
				$this->lastMessage_ = "INFO: started feeding data to HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't start feeding data to HTTP server (" . $e->getMessage() . ")";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public function feedData($data, $dataOffset, $dataBytes) {
		/* synchronized ($this) */ {
			if (!$this->isConnected_()) {
				$this->lastMessage_ = "WARNING: already disconnected from HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 2 && $this->state_ !== 3) {
				$this->lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			try {
				$this->state_ = 3;
				$this->sendRequestBody_($data, $dataOffset, $dataBytes);
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't feed data to HTTP server (" . $e->getMessage() . ")";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public function feedDataPause() {
		/* synchronized ($this) */ {
			if (!$this->isConnected_()) {
				$this->lastMessage_ = "WARNING: already disconnected from HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 2 && $this->state_ !== 3) {
				$this->lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			try {
				$this->state_ = 4;
				$this->sendRequestBody_(null, 0, 0);
				while ($this->state_ === 4) {
					$this->checkResponse_(-1);
				}
				if ($this->state_ === 5) {
					$this->lastMessage_ = "ERROR: can't stop feeding data to HTTP server";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					return false;
				}
				$this->lastMessage_ = "INFO: stopped feeding data to HTTP server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't stop feeding data to HTTP server (" . $e->getMessage() . ")";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public function getLastMessage() {
		return $this->lastMessage_;
	}

	public function sleep($timeout) {
		$endTime = intval(microtime(true) * 1000.0) + $timeout;
		while (true) {
			$this->checkResponse_($timeout);
			$timeout = $endTime - intval(microtime(true) * 1000.0);
			if ($timeout <= 0) {
				break;
			}
		}
	}

	protected abstract function sendRequestHeader_($domainId,
												   $codec,
												   $resultType,
												   $resultEncoding,
												   $serviceAuthorization,
												   $voiceDetection,
												   $acceptTopic,
												   $contentType,
												   $accept, $type, $dataBytes);

	protected abstract function sendRequestBody_($data, $dataOffset, $dataBytes);

	protected abstract function checkResponse_($timeout);

	protected function onOpen_($sessionId) {
		if ($this->listener_ !== null) {
			$this->listener_->resultCreated($sessionId);
		}
	}

	protected function onClose_() {
		/* synchronized ($this) */ {
			$this->state_ = 0;
			//notifyAll();
		}
	}

	protected function onError_($cause) {
		/* synchronized ($this) */ {
			if ($this->state_ === 0 || $this->state_ === 5) {
				return;
			}
			$this->lastMessage_ = "ERROR: caught exception (" . $cause->getMessage() . ")";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			$this->state_ = 5;
			//notifyAll();
		}
	}

	protected function onMessage_($resultData) {
		if ($this->listener_ !== null) { // {
			if (strlen($resultData) >= 3 && substr_compare($resultData, "...", -3) === 0 || strlen($resultData) >= 5 && substr_compare($resultData, "...\"}", -5) === 0) {
				$this->listener_->resultUpdated($resultData);
			} else {
				$this->listener_->resultFinalized($resultData);
			}
		}
	}
}

?>
