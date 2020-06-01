AmiVoice 音声認識APIのサンプルプログラム
==========================================================================

AmiVoice 音声認識APIを使ったサンプルプログラムです。APIの詳細は、[AmiVoice Cloud Platform
マニュアル](https://acp.amivoice.com/main/manual/)を参照してください。サンプルプログラムの実行方法は、マニュアル中の[HTTP音声認識APIの利用体験](https://acp.amivoice.com/main/manual/http%e9%9f%b3%e5%a3%b0%e8%aa%8d%e8%ad%98api%e3%81%ae%e5%88%a9%e7%94%a8%e4%bd%93%e9%a8%93/)を参照してください。

## プログラミング言語
このリポジトリには以下のプログラミング言語によるサンプルプログラムが含まれています。
- Java
- Python
- PHP
- C#
- C++
- Javascript
- バッチ・シェルスクリプト (HTTP音声認識APIのみ)

 Javascriptのサンプルは、Webブラウザから実行するサンプルですが、その他はすべてコンソールから実行するアプリケーションです。

## APIについて
音声を送信する方法によって２つのAPIがあります。
- ストリーミング (WebSocket音声認識API)
- 音声ファイル (HTTP音声認識API)

ストリーミングのサンプルはWrpディレクトリ、音声ファイルのサンプルはHrpディレクトリを参照してください。


## その他
サービスの詳細や開発者向けのドキュメントは以下を参照ください。

- [AmiVoice Cloud Platform 公式サイト](https://acp.amivoice.com/main/)
- [AmiVoice Cloud Platform マニュアル](https://acp.amivoice.com/main/manual/)

サンプルプログラムの動作確認済みの環境、注意事項、利用しているオープンソースのライブラリについては以下を参照ください。
- [サンプルプログラムの補足](readme.txt)
