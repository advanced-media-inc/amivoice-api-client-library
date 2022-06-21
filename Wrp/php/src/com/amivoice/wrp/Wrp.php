<?php

namespace com\amivoice\wrp;

require_once("com/amivoice/wrp/Wrp_.php");
require_once("com/amivoice/wrp/WrpListener.php");

abstract class Wrp {
	private static $VERSION = null;

	public static function getVersion() {
		if (self::$VERSION === null) {
			self::$VERSION = "Wrp/1.0.03 PHP/" . phpversion() . " (" . php_uname('s') . " " . php_uname('r') . " " . php_uname('v') . ")";
		}
		return self::$VERSION;
	}

	public static function construct($implementation = 1) {
		if ($implementation === 1) {
			return new Wrp_();
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
	private $codec_;
	private $resultType_;
	private $state_;
	private $lastMessage_;
	private $waitingResults_;

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
		$this->codec_ = null;
		$this->resultType_ = null;
		$this->state_ = 0;
		$this->waitingResults_ = 0;
	}

	public function setListener($listener) {
		$this->listener_ = $listener;
	}

	public function setServerURL($serverURL) {
		$this->serverURL_ = $serverURL;
		if ($this->serverURL_ !== null) {
			if (strlen($this->serverURL_) >= 7 && substr_compare($this->serverURL_, "http://", 0, 7) === 0) {
				$this->serverURL_ = "ws://" . substr($this->serverURL_, 7);
			} else
			if (strlen($this->serverURL_) >= 8 && substr_compare($this->serverURL_, "https://", 0, 8) === 0) {
				$this->serverURL_ = "wss://" . substr($this->serverURL_, 8);
			}
		}
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

	public function setCodec($codec) {
		$this->codec_ = $codec;
	}

	public function setResultType($resultType) {
		$this->resultType_ = $resultType;
	}

	public function setServiceAuthorization($serviceAuthorization) {
		if ($serviceAuthorization !== null) {
			$this->authorization_ = $serviceAuthorization;
		}
	}

	public function connect() {
		if ($this->isConnected_()) {
			$this->lastMessage_ = "WARNING: already connected to WebSocket server";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		if ($this->serverURL_ === null) {
			$this->lastMessage_ = "ERROR: can't connect to WebSocket server (Missing server URL)";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		try {
			$this->state_ = 0;
			$this->waitingResults_ = 0;
			$this->connect_($this->serverURL_, $this->proxyServerName_, $this->connectTimeout_, $this->receiveTimeout_);
		} catch (\Exception $e) {
			$this->lastMessage_ = "ERROR: can't connect to WebSocket server (" . $e->getMessage() . "): " . $this->serverURL_;
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			$this->disconnect_();
			return false;
		}
		$this->lastMessage_ = "INFO: connected to WebSocket server: " . $this->serverURL_;
		if ($this->listener_ !== null) {
			$this->listener_->TRACE($this->lastMessage_);
		}
		return true;
	}

	protected abstract function connect_($serverURL, $proxyServerName, $connectTimeout, $receiveTimeout);

	public function disconnect() {
		if (!$this->isConnected_()) {
			$this->lastMessage_ = "WARNING: already disconnected from WebSocket server";
			if ($this->listener_ !== null) {
				$this->listener_->TRACE($this->lastMessage_);
			}
			return false;
		}
		$this->disconnect_();
		$this->lastMessage_ = "INFO: disconnected from WebSocket server";
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

	public function feedDataResume() {
		/* synchronized ($this) */ {
			if (!$this->isConnected_()) {
				$this->lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 0) {
				$this->lastMessage_ = "WARNING: already started feeding data to WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			$command = "";
			$command .= "s ";
			if ($this->codec_ !== null && $this->codec_ !== "") {
				$command .= $this->codec_;
			} else {
				$command .= "16K";
			}
			if ($this->grammarFileNames_ !== null && $this->grammarFileNames_ !== "") {
				$command .= ' ';
				$command .= $this->grammarFileNames_;
				if (strpos($this->grammarFileNames_, "\001") !== false && substr_compare($this->grammarFileNames_, "\001", -1) !== 0) {
					$command .= "\001";
				}
			} else {
				$command .= " \001";
			}
			if ($this->profileId_ !== null) {
				$command .= " profileId=";
				if (strpos($this->profileId_, ' ') !== false) {
					$command .= '"';
					$command .= $this->profileId_;
					$command .= '"';
				} else {
					$command .= $this->profileId_;
				}
			}
			if ($this->profileWords_ !== null) {
				$command .= " profileWords=";
				if (strpos($this->profileWords_, ' ') !== false) {
					$command .= '"';
					$command .= $this->profileWords_;
					$command .= '"';
				} else {
					$command .= $this->profileWords_;
				}
			}
			if ($this->segmenterProperties_ !== null) {
				$command .= " segmenterProperties=";
				if (strpos($this->segmenterProperties_, ' ') !== false) {
					$command .= '"';
					$command .= $this->segmenterProperties_;
					$command .= '"';
				} else {
					$command .= $this->segmenterProperties_;
				}
			}
			if ($this->keepFillerToken_ !== null) {
				$command .= " keepFillerToken=";
				if (strpos($this->keepFillerToken_, ' ') !== false) {
					$command .= '"';
					$command .= $this->keepFillerToken_;
					$command .= '"';
				} else {
					$command .= $this->keepFillerToken_;
				}
			}
			if ($this->resultUpdatedInterval_ !== null) {
				$command .= " resultUpdatedInterval=";
				if (strpos($this->resultUpdatedInterval_, ' ') !== false) {
					$command .= '"';
					$command .= $this->resultUpdatedInterval_;
					$command .= '"';
				} else {
					$command .= $this->resultUpdatedInterval_;
				}
			}
			if ($this->extension_ !== null) {
				$command .= " extension=";
				if (strpos($this->extension_, ' ') !== false) {
					$command .= '"';
					$command .= str_replace("\"", "\"\"", $this->extension_);
					$command .= '"';
				} else {
					$command .= $this->extension_;
				}
			}
			if ($this->authorization_ !== null) {
				$command .= " authorization=";
				if (strpos($this->authorization_, ' ') !== false) {
					$command .= '"';
					$command .= $this->authorization_;
					$command .= '"';
				} else {
					$command .= $this->authorization_;
				}
			}
			if ($this->resultType_ !== null) {
				$command .= " resultType=";
				if (strpos($this->resultType_, ' ') !== false) {
					$command .= '"';
					$command .= $this->resultType_;
					$command .= '"';
				} else {
					$command .= $this->resultType_;
				}
			}
			try {
				$this->state_ = 1;
				$data = $command;
				$this->sendRequest_($data, 0, strlen($data), 0);
				while ($this->state_ === 1) {
					$this->checkResponse_(-1);
				}
				if ($this->state_ === 5) {
					return false;
				}
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" . $e->getMessage() . ")";
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
				$this->lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 2 && $this->state_ !== 3) {
				$this->lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			try {
				$this->state_ = 3;
				$this->sendRequest_($data, $dataOffset, $dataBytes, 'p');
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't feed data to WebSocket server (" . $e->getMessage() . ")";
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
				$this->lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			if ($this->state_ !== 2 && $this->state_ !== 3) {
				$this->lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
				if ($this->listener_ !== null) {
					$this->listener_->TRACE($this->lastMessage_);
				}
				return false;
			}
			try {
				$this->state_ = 4;
				$data = "e";
				$this->sendRequest_($data, 0, strlen($data), 0);
				while ($this->state_ === 4) {
					$this->checkResponse_(-1);
				}
				if ($this->state_ === 5) {
					return false;
				}
			} catch (\Exception $e) {
				$this->lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" . $e->getMessage() . ")";
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

	public function getWaitingResults() {
		return $this->waitingResults_;
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

	protected abstract function sendRequest_($data, $dataOffset, $dataBytes, $command);

	protected abstract function checkResponse_($timeout);

	protected function onOpen_() {
	}

	protected function onClose_() {
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

	protected function onMessage_($message) {
		if (strlen($message) === 0) {
			return;
		}
		$command = $message[0];
		if ($command === 's') {
			if (strlen($message) > 1) {
				/* synchronized ($this) */ {
					$this->lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" . substr($message, 2) . ")";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					$this->state_ = 5;
					//notifyAll();
				}
			} else {
				/* synchronized ($this) */ {
					$this->lastMessage_ = "INFO: started feeding data to WebSocket server";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					$this->state_ = 2;
					//notifyAll();
				}
			}
		} else
		if ($command === 'p') {
			if (strlen($message) > 1) {
				/* synchronized ($this) */ {
					$this->lastMessage_ = "ERROR: can't feed data to WebSocket server (" . substr($message, 2) . ")";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					$this->state_ = 5;
					//notifyAll();
				}
			} else {
				// pass
			}
		} else
		if ($command === 'e') {
			if (strlen($message) > 1) {
				/* synchronized ($this) */ {
					$this->lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" . substr($message, 2) . ")";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					$this->state_ = 5;
					//notifyAll();
				}
			} else {
				/* synchronized ($this) */ {
					$this->lastMessage_ = "INFO: stopped feeding data to WebSocket server";
					if ($this->listener_ !== null) {
						$this->listener_->TRACE($this->lastMessage_);
					}
					$this->state_ = 0;
					//notifyAll();
				}
			}
		} else
		if ($command === 'S') {
			$this->waitingResults_++;
			if ($this->listener_ !== null) {
				$this->listener_->utteranceStarted(intval(substr($message, 2)));
			}
		} else
		if ($command === 'E') {
			if ($this->listener_ !== null) {
				$this->listener_->utteranceEnded(intval(substr($message, 2)));
			}
		} else
		if ($command === 'C') {
			if ($this->listener_ !== null) {
				$this->listener_->resultCreated();
			}
		} else
		if ($command === 'U') {
			if ($this->listener_ !== null) {
				$this->listener_->resultUpdated(substr($message, 2));
			}
		} else
		if ($command === 'A') {
			if ($this->listener_ !== null) {
				$this->listener_->resultFinalized(substr($message, 2));
			}
			$this->waitingResults_--;
		} else
		if ($command === 'R') {
			if ($this->listener_ !== null) {
				$this->listener_->resultFinalized("\001\001\001\001\001" . substr($message, 2));
			}
			$this->waitingResults_--;
		} else
		if ($command === 'Q') {
			if ($this->listener_ !== null) {
				$this->listener_->eventNotified($command, substr($message, 2));
			}
		} else
		if ($command === 'G') {
			if ($this->listener_ !== null) {
				$this->listener_->eventNotified($command, substr($message, 2));
			}
		}
	}
}

?>
