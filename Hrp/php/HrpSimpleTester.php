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

class HrpSimpleTester implements com\amivoice\hrp\HrpListener {
	public static function main($args) {
		if (count($args) < 4) {
			p("Usage: php HrpSimpleTester.php <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]");
			return;
		}

		$serverURL = $args[0];
		$audioFileName = $args[1];
		$codec = $args[2];
		$grammarFileNames = $args[3];
		$authorization = (count($args) > 4) ? $args[4] : null;

		// HTTP 音声認識サーバイベントリスナの作成
		$listener = new HrpSimpleTester();

		// HTTP 音声認識サーバの初期化
		$hrp = com\amivoice\hrp\Hrp::construct();
		$hrp->setListener($listener);
		$hrp->setServerURL($serverURL);
		$hrp->setCodec($codec);
		$hrp->setGrammarFileNames($grammarFileNames);
		$hrp->setAuthorization($authorization);

		// HTTP 音声認識サーバへの接続
		if (!$hrp->connect()) {
			p($hrp->getLastMessage());
			p("HTTP 音声認識サーバ " . $serverURL . " への接続に失敗しました。");
			return;
		}

		try {
			// HTTP 音声認識サーバへの音声データの送信の開始
			if (!$hrp->feedDataResume()) {
				p($hrp->getLastMessage());
				p("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
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
					$hrp->sleep(1);

					// HTTP 音声認識サーバへの音声データの送信
					if (!$hrp->feedData($audioData, 0, strlen($audioData))) {
						p($hrp->getLastMessage());
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
				p($hrp->getLastMessage());
				p("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
				return;
			}
		} finally {
			// HTTP 音声認識サーバからの切断
			$hrp->disconnect();
		}
	}

	public function __construct() {
	}

	public function resultCreated($sessionId) {
//		p("C " . $sessionId);
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
HrpSimpleTester::main($argv);

?>
