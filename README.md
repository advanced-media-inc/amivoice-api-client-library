AmiVoice API クライアントライブラリ
===================================

AmiVoice APIを利用して音声認識アプリケーションを開発するためのクライアントライブラリ`Hrp`や`Wrp`と、そのサンプルプログラムを提供しています。

## クライアントライブラリについて
AmiVoice APIで音声認識する際、主に2つのAPIがあります。

- ストリーミング (WebSocket音声認識API)
- 音声ファイル (HTTP音声認識API)

それぞれのAPIを直接利用することもできますが、サーバとの通信をラップしてAmiVoice SDKのようなインタフェースで音声認識アプリケーションの開発ができます。

WebSocket音声認識APIを使ったストリーミングは`Wrp`クラスライブラリ、音声ファイルを認識するためのHTTP音声認識APIのライブラリは`Hrp`クラスライブラリを利用します。


>**Warning**
> Hrpは同期HTTP音声認識APIのみに対応しています。非同期HTTP音声認識APIには対応していません。

## 対応している言語
以下のプログラミング言語に対応しています。

- Java
- Python
- PHP
- C#
- C++
- Javascript

サンプルプログラムは、JavascriptのみWebブラウザから実行するアプリケーションです。その他の言語にはコンソールから実行するアプリケーションが付属しています。

## リソース
サービスの詳細や開発者向けのドキュメントは以下を参照ください。

- [AmiVoice Cloud Platform 公式サイト](https://acp.amivoice.com/main/)
- [AmiVoice API マニュアル](https://docs.amivoice.com/amivoice-api/manual/)
  - [クライアントライブラリ](https://docs.amivoice.com/amivoice-api/manual/client-library/) (最新版のクライアントライブラリとサンプルプログラムのビルド済みバイナリも提供しています。)

サンプルプログラムの動作確認済みの環境、注意事項、利用しているオープンソースのライブラリについては以下を参照ください。
- [サンプルプログラムの補足](readme.txt)
