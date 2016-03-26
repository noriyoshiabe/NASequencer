ABC Syntax
==========

NASequencerで扱うABC NotationはABC Notation standard 2.1の仕様に準じます。
詳細なリファレンスは本家サイトを参照して下さい。  
[http://abcnotation.com/wiki/abc:standard:v2.1](http://abcnotation.com/wiki/abc:standard:v2.1)

また、NASequencerでは以下の制限事項があります。

- 文字コードはASCIIとUTF-8のみサポートします
- MIDI再生に影響しない楽譜記述用の構文は無視されます
- 2.0以下の廃止となった記法はサポートしません
- Include field を使用する場合、インクルードされるファイルは[インクルードサーチパス](TODO)で指定されたディレクトリ配下に配置する必要があります
