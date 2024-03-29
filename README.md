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

## 開発環境
動作を確認したプラットフォームと開発環境です。
### Java
- 以下の開発環境にてビルド
   - Java 7
- 以下のバージョンにて動作確認
   - Java 7
   - Java 9
   - Java 13
- 以下のプラットフォームにて動作確認
   - Windows 10
   - Cent OS 7

### Python
- 以下のバージョンにて動作確認
   - 2.7.5
   - 2.7.16
   - 3.6.8
   - 3.7.4
- 以下のプラットフォームにて動作確認
   - Windows 10
   - Cent OS 7

### PHP
- 以下のバージョンにて動作確認
   - 5.6.40
   - 7.3.11
- 以下のプラットフォームにて動作確認
   - Windows 10
   - Cent OS 7

### C#
- 以下の開発環境にてビルド
   - Visual Studio 2015 (.NET Framework 4.5.2)
- 以下のプラットフォームにて動作確認
   - Windows 10

### C++
- 以下の開発環境にてビルド
   - Visual Studio 2015 (Visuaul C++ 14.0)
   - gcc 4.8.5
- 以下のプラットフォームにて動作確認
   - Windows 10
   - Cent OS 7


## 注意事項
### すべての言語共通
- Hrp/Wrp ライブラリは HTTP や WebSocket  といったプロトコルを使って音声認識サーバと通信をするためのライブラリですが、可能な限り各プログラミング言語間の差異を排除するために、たとえ各プログラミング言語が  HTTP  やWebSocket のためのライブラリを持っていたとしても、あえてそれらのライブラリを使用せずに低レベルなソケットライブラリを使用して実装しています。
- 全てのソースファイルは UTF-8 文字エンコーディングで記述しています。
- サンプルプログラムの利用しているオープンソースのライブラリについては以下を参照ください。
  - [サンプルプログラムの補足](readme.txt)

### Java
- ライブラリは jar ファイルとして提供されます。
- java/src フォルダには、jar  ファイルをビルドするために必要な全てのソースファイル群が含まれています。
- TLS ソケットを作成する際、SSLContext.getInstance() メソッドの protocol引数には "TLSv1.2" を指定しています。
- (Hrp|Hrp)(Tester|SimpleTester).java で定義している text_()  メソッドはJSON の形式の認識結果情報から text  情報を抽出するという処理を行っていまずが、Java の標準ライブラリに JSON  文字列をパースする処理が存在しないため、当メソッドでは簡便な方法で当処理を実装しています。当メソッドがJSON  文字列をパースする処理として正しいことを保証するものではありません。

### Python
- ライブラリは .py ファイル群として提供されます。
- 同一のソースファイルにて Python 2 系と Python 3 系の両方をサポートしています。
- Python 2 系でライブラリを使用する際、 ライブラリが提供する各メソッドの文字列引数には str 型／unicode 型どちらの型でも指定できます。
- Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データベースファイルへのパスを環境変数 SSL_CERT_FILE  に指定する必要があります。

### PHP
- ライブラリは .php ファイル群として提供されます。
- 同一のソースファイルにて PHP 5 系と PHP  7  系の両方をサポートしています。
- PHP 5 系でライブラリを使用する際、ライブラリが提供する各メソッドの文字列引数には UTF-8  文字エンコーディングでエンコードされた文字列を指定する必要があり、 ライブラリのイベントリスナを通して  UTF-8  文字エンコーディングでエンコードされた文字列が通知されます。
- Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データベースファイルへのパスを php.ini ディレクティブ openssl.cafile  に指定する必要があります。
- PHP の標準ライブラリに接続済み TCP ソケットから TLS ソケットを生成する処理が存在しないため、HTTP プロキシを通して スキームが https や wss のURL に接続できない (接続できな場合がある) ので注意してください。

### C#
- ライブラリはマネージドクラスライブラリ (.dll ファイル)  として提供されます。
- cs/src フォルダには、 マネージドクラスライブラリをビルドするために必要な全てのソースファイル群が含まれています。
- TLS  ソケットを作成する際、 SslStream.AuthenticateAsClient  メソッドのenabledSslProtocols 引数にはSslProtocols.Tls12 を指定しています。
- (Hrp|Hrp)(Tester|SimpleTester).cs  で定義している  text_()  メソッドはJSON の形式の認識結果情報から text  情報を抽出するという処理を行っていまずが、C# の標準ライブラリに JSON  文字列をパースする処理が存在しないため、当メソッドでは簡便な方法で当処理を実装しています。  当メソッドがJSON  文字列をパースする処理として正しいことを保証するものではありません。

### C++
- ライブラリはダイナミックライブラリ (.dll ファイル／.so ファイル)  として提供されます。
- cpp/src フォルダには、ダイナミックライブラリをビルドするために必要な全てのソースファイル群が含まれています。
- POCO C++ Libraries 1.9.4 を使用しています。 ライセンスを[「サンプルプログラムの補足」](readme.txt)に記載します。
- POCO C++ Libraries プロジェクトがビルドした OpenSSL version 1.1.0 を使用しています。ライセンスを[「サンプルプログラムの補足」](readme.txt)に記載します。
- ダイナミックライブラリ  (.dll ファイル／.so ファイル)   をビルドするには、別途 C++ Libraries や OpenSSL のヘッダファイル群やスタティックライブラリ群が必要です。https://pocoproject.org/download.html の指示に従ってダウンロードやビルドを行ってください。
- ライブラリが提供する各メソッドの文字列引数には UTF-8  文字エンコーディングでエンコードされた文字列への const char*  ポインタを指定する必要があり、ライブラリのイベントリスナを通して UTF-8文字エンコーディングでエンコードされた文字列への const char* ポインタが通知されます。
- Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データベースファイルへのパスを環境変数 SSL_CERT_FILE  に指定する必要があります。
- (Hrp|Hrp)(Tester|SimpleTester).cpp で定義している  text_()  メソッドはJSON の形式の認識結果情報から text  情報を抽出するという処理を行っていまずが、C++ の標準ライブラリに  JSON  文字列をパースする処理が存在しないため、当メソッドでは簡便な方法で当処理を実装しています。当メソッドがJSON  文字列をパースする処理として正しいことを保証するものではありませ
ん。
- (Hrp|Hrp)(Tester|SimpleTester).cpp  で定義している  Pointer   クラス／String クラス／StringList クラスは、サンプルプログラムを実現するために必要最低限な処理のみを実装したクラスです。それぞれ汎用的な Pointer  クラス／String クラス／StringList クラスとしての動作を保証するものではありません。
