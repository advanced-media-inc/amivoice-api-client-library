var Hrp = function() {
	// public オブジェクト
	var hrp_ = {
		// public プロパティ
		version: "Hrp/1.0.04",
		serverURL: "",
		serverURLElement: undefined,
		grammarFileNames: "",
		grammarFileNamesElement: undefined,
		profileId: "",
		profileIdElement: undefined,
		profileWords: "",
		profileWordsElement: undefined,
		segmenterProperties: "",
		segmenterPropertiesElement: undefined,
		keepFillerToken: "",
		keepFillerTokenElement: undefined,
		resultUpdatedInterval: "",
		resultUpdatedIntervalElement: undefined,
		extension: "",
		extensionElement: undefined,
		authorization: "",
		authorizationElement: undefined,
		codec: "",
		codecElement: undefined,
		resultType: "",
		resultTypeElement: undefined,
		resultEncoding: "",
		resultEncodingElement: undefined,
		serviceAuthorization: "",
		serviceAuthorizationElement: undefined,
		voiceDetection: "",
		voiceDetectionElement: undefined,
		audio: null,
		audioElement: undefined,
		issuerURL: "",
		issuerURLElement: undefined,
		sid: null,
		sidElement: undefined,
		spw: null,
		spwElement: undefined,
		epi: null,
		epiElement: undefined,
		// public メソッド
		feedDataResume: feedDataResume_,
		feedDataPause: feedDataPause_,
		isActive: isActive_,
		issue: issue_,
		// イベントハンドラ
		feedDataResumeStarted: undefined,
		feedDataResumeEnded: undefined,
		feedDataPauseStarted: undefined,
		feedDataPauseEnded: undefined,
		resultCreated: undefined,
		resultUpdated: undefined,
		resultFinalized: undefined,
		issueStarted: undefined,
		issueEnded: undefined,
		TRACE: undefined
	};

	// 送信関連
	var state_ = 0;
	var inDataBytes_ = 0;
	var reason_;
	var recorder_ = window.Recorder || null;

	if (recorder_) {
		// 録音ライブラリのプロパティの設定
		recorder_.downSampling = true;

		// 録音の開始処理が完了した時に呼び出されます。
		recorder_.resumeEnded = function(samplesPerSec) {
			if (state_ === 1) {
				state_ = 2;
				if (hrp_.feedDataResumeEnded) hrp_.feedDataResumeEnded();
			}
		};

		// 録音の開始処理が失敗した時または録音の停止処理が完了した時に呼び出されます。
		recorder_.pauseEnded = function(reason, waveFile) {
			if (state_ === 1) {
				state_ = 0;
				if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded(reason);
			} else
			if (state_ === 2) {
				if (hrp_.feedDataPauseStarted) hrp_.feedDataPauseStarted();
				if (waveFile) {
					feedDataPause__(reason, waveFile);
				} else {
					state_ = 0;
					if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded(reason);
				}
			} else
			if (state_ === 3) {
				if (waveFile) {
					feedDataPause__(reason, waveFile);
				} else {
					state_ = 0;
					if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded(reason);
				}
			}
		};
	}

	// 音声データの供給の開始
	function feedDataResume_(waveFile) {
		if (state_ !== 0) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't start feeding data to HTTP server (Invalid state: " + state_ + ")");
			return false;
		}
		if (waveFile !== void 0) {
			if (hrp_.feedDataPauseStarted) hrp_.feedDataPauseStarted();
			if (!window.XMLHttpRequest) {
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't start feeding data to HTTP server (Unsupported XMLHttpRequest class)");
				if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded({code: 2, message: "Unsupported XMLHttpRequest class"});
				return true;
			}
			state_ = 4;
			feedDataPause__(null, waveFile);
			return true;
		}
		if (hrp_.feedDataResumeStarted) hrp_.feedDataResumeStarted();
		if (!window.XMLHttpRequest) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't start feeding data to HTTP server (Unsupported XMLHttpRequest class)");
			if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded({code: 2, message: "Unsupported XMLHttpRequest class"});
			return true;
		}
		state_ = 1;
		reason_ = null;
		if (recorder_) {
			recorder_.TRACE = hrp_.TRACE;
			recorder_.resume();
		} else {
			state_ = 2;
			if (hrp_.feedDataResumeEnded) hrp_.feedDataResumeEnded();
		}
		return true;
	}

	// 音声データの供給の停止
	function feedDataPause_(reason, waveFile) {
		if (state_ !== 2) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't stop feeding data to HTTP server (Invalid state: " + state_ + ")");
			return false;
		}
		if (hrp_.feedDataPauseStarted) hrp_.feedDataPauseStarted();
		state_ = 3;
		if (recorder_) {
			recorder_.TRACE = hrp_.TRACE;
			recorder_.pause();
		} else {
			state_ = 4;
			feedDataPause__(reason, waveFile);
		}
		return true;
	}
	function feedDataPause__(reason, waveFile) {
		reason_ = reason;
		if (waveFile) {
			hrp_.audio = waveFile;
			if (hrp_.audioElement) {
				if (hrp_.audioElement.update) {
					hrp_.audioElement.update(hrp_.audio);
				} else {
					hrp_.audioElement.value = hrp_.audio;
				}
			}
		}
		if (hrp_.serverURLElement) hrp_.serverURL = hrp_.serverURLElement.value;
		if (hrp_.grammarFileNamesElement) hrp_.grammarFileNames = hrp_.grammarFileNamesElement.value;
		if (hrp_.profileIdElement) hrp_.profileId = hrp_.profileIdElement.value;
		if (hrp_.profileWordsElement) hrp_.profileWords = hrp_.profileWordsElement.value;
		if (hrp_.segmenterPropertiesElement) hrp_.segmenterProperties = hrp_.segmenterPropertiesElement.value;
		if (hrp_.keepFillerTokenElement) hrp_.keepFillerToken = hrp_.keepFillerTokenElement.value;
		if (hrp_.resultUpdatedIntervalElement) hrp_.resultUpdatedInterval = hrp_.resultUpdatedIntervalElement.value;
		if (hrp_.extensionElement) hrp_.extension = hrp_.extensionElement.value;
		if (hrp_.authorizationElement) hrp_.authorization = hrp_.authorizationElement.value;
		if (hrp_.codecElement) hrp_.codec = hrp_.codecElement.value;
		if (hrp_.resultTypeElement) hrp_.resultType = hrp_.resultTypeElement.value;
		if (hrp_.resultEncodingElement) hrp_.resultEncoding = hrp_.resultEncodingElement.value;
		if (hrp_.serviceAuthorizationElement) hrp_.serviceAuthorization = hrp_.serviceAuthorizationElement.value;
		if (hrp_.voiceDetectionElement) hrp_.voiceDetection = hrp_.voiceDetectionElement.value;
		if (hrp_.audioElement) hrp_.audio = hrp_.audioElement.file;
		if (!hrp_.audio) {
			state_ = 0;
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't stop feeding data to HTTP server (Missing audio)");
			if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded({code: 3, message: "Missing audio"});
			return;
		}
		var formData = new FormData();
		var domainId = "";
		if (hrp_.grammarFileNames) {
			domainId += "grammarFileNames=";
			domainId += encodeURIComponent(hrp_.grammarFileNames);
		}
		if (hrp_.profileId) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "profileId=";
			domainId += encodeURIComponent(hrp_.profileId);
		}
		if (hrp_.profileWords) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "profileWords=";
			domainId += encodeURIComponent(hrp_.profileWords);
		}
		if (hrp_.segmenterProperties) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "segmenterProperties=";
			domainId += encodeURIComponent(hrp_.segmenterProperties);
		}
		if (hrp_.keepFillerToken) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "keepFillerToken=";
			domainId += encodeURIComponent(hrp_.keepFillerToken);
		}
		if (hrp_.resultUpdatedInterval) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "resultUpdatedInterval=";
			domainId += encodeURIComponent(hrp_.resultUpdatedInterval);
		}
		if (hrp_.extension) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "extension=";
			domainId += encodeURIComponent(hrp_.extension);
		}
		if (hrp_.authorization) {
			if (domainId.length > 0) {
				domainId += ' ';
			}
			domainId += "authorization=";
			domainId += encodeURIComponent(hrp_.authorization);
		}
		formData.append("d", domainId);
		if (hrp_.codec) {
			formData.append("c", hrp_.codec);
		}
		if (hrp_.resultType) {
			formData.append("r", hrp_.resultType);
		}
		if (hrp_.resultEncoding) {
			formData.append("e", hrp_.resultEncoding);
		}
		if (hrp_.serviceAuthorization) {
			formData.append("u", hrp_.serviceAuthorization);
		}
		if (hrp_.voiceDetection) {
			formData.append("v", hrp_.voiceDetection);
		}
		formData.append("a", hrp_.audio);
		state_ = 0;
		inDataBytes_ = 0;
		var httpRequest = new XMLHttpRequest();
		httpRequest.addEventListener("loadstart", onOpen_);
		httpRequest.addEventListener("progress", onMessage_);
		httpRequest.addEventListener("load", onMessage_);
		httpRequest.addEventListener("error", onError_);
		httpRequest.addEventListener("abort", onError_);
		httpRequest.addEventListener("timeout", onError_);
		httpRequest.addEventListener("loadend", onClose_);
		httpRequest.open("POST", hrp_.serverURL, true);
		httpRequest.send(formData);
	}

	function onOpen_(e) {
	}

	function onClose_(e) {
		state_ = 0;
		if (!reason_) {
			reason_ = {code: 0, message: ""};
		}
		if (hrp_.feedDataPauseEnded) hrp_.feedDataPauseEnded(reason_);
	}

	function onMessage_(e) {
		if (state_ === 0) {
			if (e.target.status !== 200) {
				state_ = 9;
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't stop feeding data to HTTP server (Invalid response code: " + e.target.status + ")");
				if (!reason_ || reason_.code < 2) {
					reason_ = {code: 3, message: "Invalid response code: " + e.target.status};
				}
				e.target.abort();
				return;
			}
			var sessionId = e.target.getResponseHeader("X-Session-ID");
			if (!sessionId) {
				state_ = 9;
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't stop feeding data to HTTP server (Missing session id)");
				if (!reason_ || reason_.code < 2) {
					reason_ = {code: 3, message: "Missing session id"};
				}
				e.target.abort();
				return;
			}
			if (hrp_.resultCreated) hrp_.resultCreated(sessionId);
			state_ = 1;
		}
		if (state_ === 1) {
			var inDataBytes = e.target.response.indexOf("\r\n", inDataBytes_);
			while (inDataBytes !== -1) {
				var inData = e.target.response.substring(inDataBytes_, inDataBytes);
				if (inData.indexOf("...", inData.length - 3) !== -1 || inData.indexOf("...\"}", inData.length - 5) !== -1) {
					if (hrp_.resultUpdated) hrp_.resultUpdated(inData);
				} else {
					if (hrp_.resultFinalized) hrp_.resultFinalized(inData);
				}
				inDataBytes_ = inDataBytes + 2;
				inDataBytes = e.target.response.indexOf("\r\n", inDataBytes_);
			}
		}
	}

	function onError_(e) {
		if (state_ !== 9) {
			state_ = 9;
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't stop feeding data to HTTP server (Caught '" + e.type + "' event)");
			if (!reason_ || reason_.code < 2) {
				reason_ = {code: 3, message: "Caught '" + e.type + "' event"};
			}
		}
	}

	// 音声データの供給中かどうかの取得
	function isActive_() {
		return (state_ === 2);
	}

	// サービス認証キー文字列の発行
	function issue_() {
		if (!window.XMLHttpRequest) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Unsupported XMLHttpRequest class)");
			return false;
		}
		if (hrp_.issuerURLElement) hrp_.issuerURL = hrp_.issuerURLElement.value;
		if (hrp_.sidElement) hrp_.sid = hrp_.sidElement.value;
		if (hrp_.spwElement) hrp_.spw = hrp_.spwElement.value;
		if (hrp_.epiElement) hrp_.epi = hrp_.epiElement.value;
		if (!hrp_.sid) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Missing service id)");
			alert("サービス ID が設定されていません。");
			if (hrp_.sidElement) hrp_.sidElement.focus();
			return false;
		}
		for (var i = 0; i < hrp_.sid.length; i++) {
			var c = hrp_.sid.charCodeAt(i);
			if (!(c >= 0x30 && c <= 0x39 || c >= 0x61 && c <= 0x7A || c >= 0x41 && c <= 0x5A || c === 0x2D || c === 0x5F)) {
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Illegal char in service id)");
				if (hrp_.sidElement) alert("サービス ID に許されていない文字が使用されています。");
				if (hrp_.sidElement) hrp_.sidElement.focus();
				return false;
			}
		}
		if (!hrp_.spw) {
			if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Missing service password)");
			alert("サービスパスワードが設定されていません。");
			if (hrp_.spwElement) hrp_.spwElement.focus();
			return false;
		}
		for (var i = 0; i < hrp_.spw.length; i++) {
			var c = hrp_.spw.charCodeAt(i);
			if (c < 0x20 || c > 0x7E) {
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Illegal char in service password)");
				if (hrp_.spwElement) alert("サービスパスワードに許されていない文字が使用されています。");
				if (hrp_.spwElement) hrp_.spwElement.focus();
				return false;
			}
		}
		for (var i = 0; i < hrp_.epi.length; i++) {
			var c = hrp_.epi.charCodeAt(i);
			if (c < 0x30 || c > 0x39) {
				if (hrp_.TRACE) hrp_.TRACE("ERROR: can't issue service authorization (Illegal char in pexires in)");
				if (hrp_.epiElement) alert("有効期限に許されていない文字が使用されています。");
				if (hrp_.epiElement) hrp_.epiElement.focus();
				return false;
			}
		}
		if (hrp_.issueStarted) hrp_.issueStarted();
		var searchParams = "sid=" + encodeURIComponent(hrp_.sid) + "&spw=" + encodeURIComponent(hrp_.spw);
		if (hrp_.epi) {
			searchParams += "&epi=" + encodeURIComponent(hrp_.epi);
		}
		var httpRequest = new XMLHttpRequest();
		httpRequest.addEventListener("load", function(e) {
			if (e.target.status === 200) {
				if (hrp_.serviceAuthorizationElement) {
					hrp_.serviceAuthorizationElement.value = e.target.response;
				} else
				if (hrp_.authorizationElement) {
					hrp_.authorizationElement.value = e.target.response;
				} else {
					hrp_.serviceAuthorization = e.target.response;
				}
				if (hrp_.issueEnded) hrp_.issueEnded(e.target.response);
			} else {
				if (hrp_.issueEnded) hrp_.issueEnded("");
			}
		});
		httpRequest.addEventListener("error", function(e) {
			if (hrp_.issueEnded) hrp_.issueEnded("");
		});
		httpRequest.addEventListener("abort", function(e) {
			if (hrp_.issueEnded) hrp_.issueEnded("");
		});
		httpRequest.addEventListener("timeout", function(e) {
			if (hrp_.issueEnded) hrp_.issueEnded("");
		});
		httpRequest.open("POST", hrp_.issuerURL, true);
		httpRequest.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		httpRequest.send(searchParams);
		return true;
	}

	// public プロパティの初期化
	if (recorder_) {
		hrp_.version += " " + recorder_.version;
	}
	hrp_.serverURL = window.location.protocol + "//" + window.location.host + window.location.pathname;
	hrp_.serverURL = hrp_.serverURL.substring(0, hrp_.serverURL.lastIndexOf('/') + 1);
	if (hrp_.serverURL.endsWith("/tool/")) {
		hrp_.serverURL = hrp_.serverURL.substring(0, hrp_.serverURL.length - 5);
	}
	hrp_.serverURL += "/recognize";
	hrp_.grammarFileNames = "-a-general";
	hrp_.issuerURL = window.location.protocol + "//" + window.location.host + window.location.pathname;
	hrp_.issuerURL = hrp_.issuerURL.substring(0, hrp_.issuerURL.lastIndexOf('/'));
	if (hrp_.issuerURL.indexOf("/tool", hrp_.issuerURL.length - 5) !== -1) {
		hrp_.issuerURL = hrp_.issuerURL.substring(0, hrp_.issuerURL.length - 5);
	}
	hrp_.issuerURL += "/issue_service_authorization";

	// public オブジェクトの返却
	return hrp_;
}();
