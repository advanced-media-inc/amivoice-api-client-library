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

class WrpSimpleTester implements com\amivoice\wrp\WrpListener {
	public static function main($args) {
		if (count($args) < 4) {
			p("Usage: php WrpSimpleTester.php <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]");
			return;
		}

		$serverURL = $args[0];
		$audioFileName = $args[1];
		$codec = $args[2];
		$grammarFileNames = $args[3];
		$authorization = (count($args) > 4) ? $args[4] : null;

		// WebSocket 音声認識サーバイベントリスナの作成
		$listener = new WrpSimpleTester();

		// WebSocket 音声認識サーバの初期化
		$wrp = com\amivoice\wrp\Wrp::construct();
		$wrp->setListener($listener);
		$wrp->setServerURL($serverURL);
		$wrp->setCodec($codec);
		$wrp->setGrammarFileNames($grammarFileNames);
		$wrp->setAuthorization($authorization);

		// WebSocket 音声認識サーバへの接続
		if (!$wrp->connect()) {
			p($wrp->getLastMessage());
			p("WebSocket 音声認識サーバ " . $serverURL . " への接続に失敗しました。");
			return;
		}

		try {
			// WebSocket 音声認識サーバへの音声データの送信の開始
			if (!$wrp->feedDataResume()) {
				p($wrp->getLastMessage());
				p("WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。");
				return;
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
					// 微小時間のスリープ
					$wrp->sleep(1);

					// 認識結果情報待機数が 1 以下になるまでスリープ
					$maxSleepTime = 50000;
					while ($wrp->getWaitingResults() > 1 && $maxSleepTime > 0) {
						$wrp->sleep(100);
						$maxSleepTime -= 100;
					}

					// WebSocket 音声認識サーバへの音声データの送信
					if (!$wrp->feedData($audioData, 0, strlen($audioData))) {
						p($wrp->getLastMessage());
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
				p($wrp->getLastMessage());
				p("WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。");
				return;
			}
		} finally {
			// WebSocket 音声認識サーバからの切断
			$wrp->disconnect();
		}
	}

	public function __construct() {
	}

	public function utteranceStarted($startTime) {
//		p("S " . $startTime);
	}

	public function utteranceEnded($endTime) {
//		p("E " . $endTime);
	}

	public function resultCreated() {
//		p("C");
	}

	public function resultUpdated($result) {
//		p("U " . $result);
	}

	public function resultFinalized($result) {
//		p("F " . $result);
		p($result);
		$text = $this->text_($result);
		if ($text !== null) {
			p(" -> " . $text);
		}
	}

	public function eventNotified($eventId, $eventMessage) {
//		p($eventId . " " . $eventMessage);
	}

	public function TRACE($message) {
	}

	private function text_($result) {
		return json_decode($result, true)["text"];
	}
}

$argv = array_slice($argv, 1);
for ($i = 0; $i < count($argv); $i++) {
	$argv[$i] = u($argv[$i]);
}
WrpSimpleTester::main($argv);

?>
