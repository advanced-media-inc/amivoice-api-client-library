<?php

require_once("com/amivoice/wrp/Wrp.php");
require_once("com/amivoice/wrp/WrpListener.php");

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

class WrpTester implements com\amivoice\wrp\WrpListener {
	public static function main($args) {
		// WebSocket 音声認識サーバ URL
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
		// 音声データ形式
		$codec = null;
		// 認識結果タイプ
		$resultType = null;
		// サービス認証キー文字列
		$serviceAuthorization = null;
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
			if (strlen($arg) >= 2 && substr_compare($arg, "c=", 0, 2) === 0) {
				$codec = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "r=", 0, 2) === 0) {
				$resultType = substr($arg, 2);
			} else
			if (strlen($arg) >= 2 && substr_compare($arg, "u=", 0, 2) === 0) {
				$serviceAuthorization = substr($arg, 2);
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
			if (strlen($arg) >= 2 && substr_compare($arg, "-3", 0, 2) === 0) {
				$implementation = 3;
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
			p("Usage: php WrpTester.php [<parameters/options>]");
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
			p("  c=<codec>");
			p("  r=<resultType>");
			p("  u=<serviceAuthorization>");
			p("Options:");
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

		// WebSocket 音声認識サーバイベントリスナの作成
		$listener = new WrpTester($verbose);

		// WebSocket 音声認識サーバの初期化
		$wrp = com\amivoice\wrp\Wrp::construct($implementation);
		$wrp->setListener($listener);
		$wrp->setServerURL($serverURL);
		$wrp->setProxyServerName($proxyServerName);
		$wrp->setConnectTimeout($connectTimeout);
		$wrp->setReceiveTimeout($receiveTimeout);
		$wrp->setGrammarFileNames($grammarFileNames);
		$wrp->setProfileId($profileId);
		$wrp->setProfileWords($profileWords);
		$wrp->setSegmenterProperties($segmenterProperties);
		$wrp->setKeepFillerToken($keepFillerToken);
		$wrp->setResultUpdatedInterval($resultUpdatedInterval);
		$wrp->setExtension($extension);
		$wrp->setAuthorization($authorization);
		$wrp->setCodec($codec);
		$wrp->setResultType($resultType);
		$wrp->setServiceAuthorization($serviceAuthorization);

		// WebSocket 音声認識サーバへの接続
		if (!$wrp->connect()) {
			if (!$verbose) {
				p($wrp->getLastMessage());
			}
			p("WebSocket 音声認識サーバ " . $serverURL . " への接続に失敗しました。");
			return;
		}

		try {
			// <!--
			if ($verbose) {
				p();
				p("======================= " . com\amivoice\wrp\Wrp::getVersion());
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

					// WebSocket 音声認識サーバへの音声データの送信の開始
					if (!$wrp->feedDataResume()) {
						if (!$verbose) {
							p($wrp->getLastMessage());
						}
						p("WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。");
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
								$wrp->sleep($sleepTime);
							} else {
								// スリープ時間が計算されていない場合...
								// 微小時間のスリープ
								$wrp->sleep(1);

								// 認識結果情報待機数が 1 以下になるまでスリープ
								$maxSleepTime = 50000;
								while ($wrp->getWaitingResults() > 1 && $maxSleepTime > 0) {
									$wrp->sleep(100);
									$maxSleepTime -= 100;
								}
							}

							// WebSocket 音声認識サーバへの音声データの送信
							if (!$wrp->feedData($audioData, 0, strlen($audioData))) {
								if (!$verbose) {
									p($wrp->getLastMessage());
								}
								p("WebSocket 音声認識サーバへの音声データの送信に失敗しました。");
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

					// WebSocket 音声認識サーバへの音声データの送信の完了
					if (!$wrp->feedDataPause()) {
						if (!$verbose) {
							p($wrp->getLastMessage());
						}
						p("WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。");
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
			// WebSocket 音声認識サーバからの切断
			$wrp->disconnect();
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

	public function utteranceStarted($startTime) {
		p("S " . $startTime);
	}

	public function utteranceEnded($endTime) {
		p("E " . $endTime);
	}

	public function resultCreated() {
		p("C");
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

	public function eventNotified($eventId, $eventMessage) {
		p($eventId . " " . $eventMessage);
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
WrpTester::main($argv);

?>
