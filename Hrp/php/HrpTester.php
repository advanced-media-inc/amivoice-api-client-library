<?php

require_once("com/amivoice/hrp/Hrp.php");
require_once("com/amivoice/hrp/HrpListener.php");

function u($s) {
	if (PHP_MAJOR_VERSION > 5 || PATH_SEPARATOR !== ';') {
		return $s;
	}
	if ($s === null) {
		return null;
	}
	return mb_convert_encoding($s, "UTF-8", "MS932");
}

function s($s) {
	if (PHP_MAJOR_VERSION > 5 || PATH_SEPARATOR !== ';') {
		return $s;
	}
	if ($s === null) {
		return null;
	}
	return mb_convert_encoding($s, "MS932", "UTF-8");
}

function p($s = "") {
	print s($s) . "\n";
}

class HrpTester implements com\amivoice\hrp\HrpListener {
	public static function main($args) {
		// HTTP 音声認識サーバ URL
		$serverURL = null;
		// プロキシサーバ名
		$proxyServerName = null;
		// 音声データファイル名
		$audioFileNames = [];
		// グラマファイル名
		$grammarFileNames = null;
		// プロファイル ID
		$profileId = null;
		// プロファイル登録単語
		$profileWords = null;
		// セグメンタプロパティ
		$segmenterProperties = null;
		// フィラー単語を保持するかどうか
		$keepFillerToken = null;
		// 認識中イベント発行間隔
		$resultUpdatedInterval = null;
		// 拡張情報
		$extension = null;
		// サービス認証キー文字列
		$authorization = null;
		// ドメイン ID (d=)
		$domainId = null;
		// 音声データ形式 (c=)
		$codec = null;
		// 認識結果タイプ (r=)
		$resultType = null;
		// 認識結果文字エンコーディング (e=)
		$resultEncoding = null;
		// サービス認証キー文字列 (u=)
		$serviceAuthorization = null;
		// 発話区間検出パラメータ文字列 (v=)
		$voiceDetection = null;
		// HTTP リクエストヘッダ群
		$acceptTopic = null;
		$contentType = null;
		$accept = null;
		// 処理タイプ ("s"／"sc"／"m"／"mc")
		$type = "mc";
		// 接続タイムアウト
		$connectTimeout = 5000;
		// 受信タイムアウト
		$receiveTimeout = 0;
		// 処理ループ (1～)
		$loop = 1;
		// スリープ時間
		$sleepTime = -2;
		// 詳細出力
		$verbose = false;
		// 実装タイプ
		$implementation = 1;

		// 引数のチェック
		foreach ($args as $arg) {
			if (strlen($arg) >= 2 && substr_compare($arg, "g=", 0, 2) === 0) {
				$grammarFileNames = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "i=", 0, 2) === 0) {
				$profileId = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "w=", 0, 2) === 0) {
				$profileWords = substr($arg, 2);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "op=", 0, 3) === 0) {
				$segmenterProperties = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "of=", 0, 3) === 0) {
				$keepFillerToken = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "oi=", 0, 3) === 0) {
				$resultUpdatedInterval = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "oe=", 0, 3) === 0) {
				$extension = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "ou=", 0, 3) === 0) {
				$authorization = substr($arg, 3);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "d=", 0, 2) === 0) {
				$domainId = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "c=", 0, 2) === 0) {
				$codec = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "r=", 0, 2) === 0) {
				$resultType = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "e=", 0, 2) === 0) {
				$resultEncoding = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "u=", 0, 2) === 0) {
				$serviceAuthorization = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "v=", 0, 2) === 0) {
				$voiceDetection = substr($arg, 2);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "hd=", 0, 3) === 0) {
				$acceptTopic = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "hc=", 0, 3) === 0) {
				$contentType = substr($arg, 3);
			} else
			if (strlen($arg) >= 3 && substr_compare($arg, "hr=", 0, 3) === 0) {
				$accept = substr($arg, 3);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-t", 0, 2) === 0) {
				$type = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-x", 0, 2) === 0) {
				$proxyServerName = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-c", 0, 2) === 0) {
				$connectTimeout = intval(substr($arg, 2));
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-r", 0, 2) === 0) {
				$receiveTimeout = intval(substr($arg, 2));
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-l", 0, 2) === 0) {
				$loop = intval(substr($arg, 2));
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-e", 0, 2) === 0) {
				if (strlen($arg) > 2) {
					$sleepTime = intval(substr($arg, 2));
				} else {
					$sleepTime = -1;
				}
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-v", 0, 2) === 0) {
				$verbose = true;
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "-2", 0, 2) === 0) {
				$implementation = 2;
			} else
			if ($serverURL === null) {
				$serverURL = $arg;
			} else {
				$audioFileNames[] = $arg;
			}
			if ($verbose) {
				p("DEBUG: " . $arg);
			}
		}
		if (count($audioFileNames) === 0) {
			p("Usage: php HrpTester.php [<parameters/options>]");
			p("                          <url>");
			p("                           <audioFileName>...");
			p("Parameters:");
			p("  g=<grammarFileNames>");
			p("  i=<profileId>");
			p("  w=<profileWords>");
			p("  op=<segmenterProperties>");
			p("  of=<keepFillerToken>");
			p("  oi=<resultUpdatedInterval>");
			p("  oe=<extension>");
			p("  ou=<authorization>");
			p("  d=<domainId>");
			p("  c=<codec>");
			p("  r=<resultType>");
			p("  e=<resultEncoding>");
			p("  u=<serviceAuthorization>");
			p("  v=<voiceDetection>");
			p("  hd=<acceptTopic>            HTTP 'Accept-Topic' header instead d=<domainId>");
			p("  hc=<contentType>            HTTP 'Content-Type' header instead c=<codec>");
			p("  hr=<accept>                 HTTP 'Accept' header instead r=<resultType>");
			p("Options:");
			p("  -t<type>                    (default: -tmc)");
			p("      <type>='s'              Single part & no chunked encoding");
			p("      <type>='sc'             Single part & chunked encoding");
			p("      <type>='m'              Multi parts & no chunked encoding");
			p("      <type>='mc'             Multi parts & chunked encoding");
			p("  -x<proxyServerName>         (default: -x)");
			p("  -c<connectionTimeout>       (default: -c5000)");
			p("  -r<receiveTimeout>          (default: -r0)");
			p("  -l<loop>                    (default: -l1)");
			p("  -e                          realtime simulation (default: -)");
			p("  -v                          verbose output (default: -)");
			return;
		}

		// 開始時間の取得
		$startTime = microtime(true);

		// スリープ時間の計算
		if ($sleepTime === -1) {
			if ($codec !== null) {
				if (strcasecmp($codec, "22K") === 0 || strcasecmp($codec, "MSB22K") === 0 || strcasecmp($codec, "LSB22K") === 0) {
					$sleepTime = 4096 * 1000 / 2 / 22050;
				} else
				if (strcasecmp($codec, "16K") === 0 || strcasecmp($codec, "MSB16K") === 0 || strcasecmp($codec, "LSB16K") === 0) {
					$sleepTime = 4096 * 1000 / 2 / 16000;
				} else
				if (strcasecmp($codec, "11K") === 0 || strcasecmp($codec, "MSB11K") === 0 || strcasecmp($codec, "LSB11K") === 0) {
					$sleepTime = 4096 * 1000 / 2 / 11025;
				} else
				if (strcasecmp($codec, "8K") === 0 || strcasecmp($codec, "MSB8K") === 0 || strcasecmp($codec, "LSB8K") === 0) {
					$sleepTime = 4096 * 1000 / 2 / 8000;
				} else
				if (strcasecmp($codec, "MULAW") === 0 || strcasecmp($codec, "ALAW") === 0) {
					$sleepTime = 4096 * 1000 / 1 / 8000;
				} else {
					$sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
				}
			} else {
				$sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
			}
		}

		// HTTP 音声認識サーバイベントリスナの作成
		$listener = new HrpTester($verbose);

		// HTTP 音声認識サーバの初期化
		$hrp = com\amivoice\hrp\Hrp::construct($implementation);
		$hrp->setListener($listener);
		$hrp->setServerURL($serverURL);
		$hrp->setProxyServerName($proxyServerName);
		$hrp->setConnectTimeout($connectTimeout);
		$hrp->setReceiveTimeout($receiveTimeout);
		$hrp->setGrammarFileNames($grammarFileNames);
		$hrp->setProfileId($profileId);
		$hrp->setProfileWords($profileWords);
		$hrp->setSegmenterProperties($segmenterProperties);
		$hrp->setKeepFillerToken($keepFillerToken);
		$hrp->setResultUpdatedInterval($resultUpdatedInterval);
		$hrp->setExtension($extension);
		$hrp->setAuthorization($authorization);
		$hrp->setDomainId($domainId);
		$hrp->setCodec($codec);
		$hrp->setResultType($resultType);
		$hrp->setResultEncoding($resultEncoding);
		$hrp->setServiceAuthorization($serviceAuthorization);
		$hrp->setVoiceDetection($voiceDetection);

		// HTTP 音声認識サーバへの接続
		if (!$hrp->connect()) {
			if (!$verbose) {
				p($hrp->getLastMessage());
			}
			p("HTTP 音声認識サーバ " . $serverURL . " への接続に失敗しました。");
			return;
		}

		try {
			// <!--
			if ($verbose) {
				p();
				p("======================= " . com\amivoice\hrp\Hrp::getVersion());
			}
			// -->
			for ($i = 0; $i < $loop; $i++) {
				foreach ($audioFileNames as $audioFileName) {
					// <!--
					if ($verbose) {
						p();
						if ($loop > 1) {
							p("----------------------- [" . ($i + 1) . "] " . $audioFileName);
						} else {
							p("----------------------- " . $audioFileName);
						}
						p();
					}
					// -->

					// <!-- HTTP リクエストヘッダ群
					if ($acceptTopic !== null) {
						$hrp->setAcceptTopic($acceptTopic);
					}
					if ($contentType !== null) {
						if ($contentType === "*") {
							if (strlen($audioFileName) >= 4 && substr_compare($audioFileName, ".a08", -4) === 0) {
								if (strpos($audioFileName, "msb") !== false) {
									$hrp->setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=big-endian");
								} else {
									$hrp->setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=little-endian");
								}
							} else
							if (strlen($audioFileName) >= 4 && substr_compare($audioFileName, ".a16", -4) === 0 || strlen($audioFileName) >= 4 && substr_compare($audioFileName, ".adc", -4) === 0) {
								if (strpos($audioFileName, "msb") !== false) {
									$hrp->setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=big-endian");
								} else {
									$hrp->setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=little-endian");
								}
							} else
							if (strlen($audioFileName) >= 3 && substr_compare($audioFileName, ".mu", -3) === 0) {
								$hrp->setContentType("audio/x-mulaw;rate=8000");
							} else
							if (strlen($audioFileName) >= 5 && substr_compare($audioFileName, ".alaw", -5) === 0) {
								$hrp->setContentType("audio/x-alaw;rate=8000");
							}
						} else {
							$hrp->setContentType($contentType);
						}
					}
					if ($accept !== null) {
						$hrp->setAccept($accept);
					}
					// -->

					// 音声データファイルの総バイト数の取得
					$audioDataBytes = @filesize($audioFileName);
					if ($audioDataBytes === false) {
						p("音声データファイル " . $audioFileName . " の読み込みに失敗しました。");
						break;
					}

					// HTTP 音声認識サーバへの音声データの送信の開始
					if (!$hrp->feedDataResume($type, $audioDataBytes)) {
						if (!$verbose) {
							p($hrp->getLastMessage());
						}
						p("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
						break;
					}

					$audioStream = false;
					try {
						// 音声データファイルのオープン
						$audioStream = @fopen($audioFileName, "rb");
						if ($audioStream === false) {
							throw new \Exception();
						}

						// 音声データファイルからの音声データの読み込み
						$audioData = @fread($audioStream, 4096);
						while ($audioData !== false && strlen($audioData) > 0) {
							// スリープ時間が計算されているかどうかのチェック
							if ($sleepTime >= 0) {
								// スリープ時間が計算されている場合...
								// 微小時間のスリープ
								$hrp->sleep($sleepTime);
							} else {
								// スリープ時間が計算されていない場合...
								// 微小時間のスリープ
								$hrp->sleep(1);
							}

							// HTTP 音声認識サーバへの音声データの送信
							if (!$hrp->feedData($audioData, 0, strlen($audioData))) {
								if (!$verbose) {
									p($hrp->getLastMessage());
								}
								p("HTTP 音声認識サーバへの音声データの送信に失敗しました。");
								break;
							}

							// 音声データファイルからの音声データの読み込み
							$audioData = @fread($audioStream, 4096);
						}
					} catch (\Exception $e) {
						p("音声データファイル " . $audioFileName . " の読み込みに失敗しました。");
					} finally {
						// 音声データファイルのクローズ
						if ($audioStream !== false) {
							@fclose($audioStream);
							$audioStream = false;
						}
					}

					// HTTP 音声認識サーバへの音声データの送信の完了
					if (!$hrp->feedDataPause()) {
						if (!$verbose) {
							p($hrp->getLastMessage());
						}
						p("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
						break;
					}
				}
			}
			// <!--
			if ($verbose) {
				p();
				p("=======================");
				p();
			}
			// -->
		} finally {
			// HTTP 音声認識サーバからの切断
			$hrp->disconnect();
		}

		// 終了時間の取得
		$endTime = microtime(true);

		// 経過時間の表示
		p("INFO: elapsed time: " . (($endTime - $startTime) * 1000.0) . " [msec.]");
	}

	private $verbose_;

	public function __construct($verbose) {
		$this->verbose_ = $verbose;
	}

	public function resultCreated($sessionId) {
		p("C " . $sessionId);
	}

	public function resultUpdated($result) {
		p("U " . $result);
		$text = $this->text_($result);
		if ($text !== null) {
			p("   -> " . $text);
		}
	}

	public function resultFinalized($result) {
		p("F " . $result);
		$text = $this->text_($result);
		if ($text !== null) {
			p("   -> " . $text);
		}
	}

	public function TRACE($message) {
		if ($this->verbose_) {
			p($message);
		}
	}

	private function text_($result) {
		return json_decode($result, true)["text"];
	}
}

$argv = array_slice($argv, 1);
for ($i = 0; $i < count($argv); $i++) {
	$argv[$i] = u($argv[$i]);
}
HrpTester::main($argv);

?>
