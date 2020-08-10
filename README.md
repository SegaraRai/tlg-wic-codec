# tlg-wic-codec

KIRIKIRI TLG codecs for Windows Imaging Component

このプロジェクトは[krkrz/tlg-wic-codec](https://github.com/krkrz/tlg-wic-codec)のフォークです。

吉里吉里のTLGファイルをWindows上で扱えるようにします。

具体的には、

- エクスプローラー上でのサムネイル表示
- エクスプローラー上でファイルのプロパティやツールチップからの画像の大きさ等の確認
- Windows フォト ビューアーでの表示

ができるようになります。

## ライセンス

WIC処理のオリジナルは[dds-wic-codec](http://code.google.com/p/dds-wic-codec)に由来します（MIT LICENSE）。  
`libtlg/`以下は吉里吉里に由来します（吉里吉里ライセンス）。  

私が書いた部分についてはMIT LICENSEが適用されます。

## 動作環境

Windows 10の32ビット版または64ビット版が必要です。  
それ以外のWindowsについては動作を確認しておりません。  
また、ARM版Windowsではおそらく動作しません。

管理者権限が必要です。

## インストール方法

[krkrz/tlg-wic-codec](https://github.com/krkrz/tlg-wic-codec)を導入済みの場合、そちらをアンインストールする必要はありません。  
以下のインストール手順に従うと情報が上書きされます。

1. [リリースページ](https://github.com/SegaraRai/tlg-wic-codec/releases)から最新版をダウンロードします
2. zipファイルを適当な場所に展開します
3. DLLファイルをコピーします  
   コピーしたファイルは移動や削除を行うと機能しなくなりますのでご注意ください
   - 32ビット版OSの場合：  
     `x86\tlg-wic-codec.dll`を`C:\Windows\System32`以下にコピーします
   - 64ビット版OSの場合：  
     `x86\tlg-wic-codec.dll`を`C:\Windows\SysWOW64`以下にコピーします  
     `x64\tlg-wic-codec.dll`を`C:\Windows\System32`以下にコピーします
4. DLLファイルを登録します  
   コマンドプロンプトを管理者として実行し、以下のコマンドを実行します
   - 32ビット版OSの場合：  
      `regsvr32 C:\Windows\System32\tlg-wic-codec.dll`
   - 64ビット版OSの場合：  
      `regsvr32 C:\Windows\SysWOW64\tlg-wic-codec.dll`  
      `regsvr32 C:\Windows\System32\tlg-wic-codec.dll`
5. レジストリを登録します  
   `install.reg`、`install_photoviewer.reg`の順にそれぞれダブルクリックして実行します
6. PCを再起動します

## アンインストール方法

1. レジストリを削除します  
   `uninstall.reg`をダブルクリックして実行します
2. DLLファイルを登録解除します
   コマンドプロンプトを管理者として実行し、以下のコマンドを実行します
   - 32ビット版OSの場合：  
      `regsvr32 /u C:\Windows\System32\tlg-wic-codec.dll`
   - 64ビット版OSの場合：  
      `regsvr32 /u C:\Windows\SysWOW64\tlg-wic-codec.dll`  
      `regsvr32 /u C:\Windows\System32\tlg-wic-codec.dll`
3. PCを再起動します
   これを行わないとDLLが削除できません
4. コピーしたDLLファイルを削除します  
   - 32ビット版OSの場合：  
     `C:\Windows\System32\tlg-wic-codec.dll`を削除します
   - 64ビット版OSの場合：  
     `C:\Windows\SysWOW64\tlg-wic-codec.dll`を削除します  
     `C:\Windows\System32\tlg-wic-codec.dll`を削除します

## 不具合時の対処

- Q. OSが32ビット版か64ビット版かわからない  
  A. `C:\Windows`以下に`SysWOW64`ディレクトリが存在する場合は高い確率で64ビット版です  
     存在しない場合は32ビット版と考えて良いです
- Q. `regsvr32`を実行すると、  
     `モジュール "tlg-wic-codec.dll" は読み込まれましたが、DllRegisterServer への呼び出しはエラー コード 0x80070005 により失敗しました。`  
     というダイアログが出て失敗する  
  A. コマンドプロンプトを管理者として実行してください
- Q. コマンドプロンプトを管理者として実行する方法がわからない  
  A. <kbd>Win+R</kbd>を押下し、表示されたダイアログに`cmd`と入力し、<kbd>Ctrl+Shift+Enter</kbd>を押下します  
     コマンドプロンプトのタイトルが`管理者:`で始まっていれば成功です
- Q. サムネイルが表示されない、プロパティが表示されない  
  A. ときどきレジストリの登録内容が消えてしまうことがあります  
     インストールの項の4.と5.の手順をやり直してみてください  
     また、DLLファイルを移動や削除していると機能しませんので、その場合インストール手順を最初からやり直してください
- Q. DLLファイルを削除できない  
  A. DLLファイルの登録解除ができていない、または登録解除した後にPCを再起動していない場合があります  
     アンインストールの項の2.と3.の手順をやり直してみてください
- Q. Windows 10のフォトアプリやペイントソフトで開けない  
  A. 残念ながらWindows 10のフォトアプリやペイントソフトでは開けないようです  
     原因は不明です（WICを使っていない等が考えられますが）  
     このソフトウェアによりTLGファイルが扱えるようになるOS付属のソフトウェアは、現在確認できている限りエクスプローラーとWindows フォト ビューアーのみとなります

## ビルドに必要なもの

ビルドに必要なものは以下の通りです

- Visual Studio 2019  
  Visual Studio Installerにて`C++によるデスクトップ開発`にチェックを入れておく

開発を行う場合は以下も用意してください

- [LLVM](https://llvm.org/)
  - コードフォーマットに使用
  - PATHに追加しておく
- Windows Subsystem for Linux
  - コードフォーマットに使用
- 7-Zip
  - パッケージングに使用
