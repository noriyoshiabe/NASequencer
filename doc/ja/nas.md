Note as Sequence Syntax
=======================
Version 0.5.0 Draft, 2016

基本的な構造
------------
### ファイルフォーマット
- 読み込み可能な拡張子は`.nas`です
- 演奏データはプレーンテキストで記述します
- マクロを除き大文字小文字は区別されません
- 文字列やコメントを除いて、ASCIIコードで記述します
- 文字コードはASCIIとUTF-8のみサポートします

### ステップの考え方
音符の入力には必ずステップの指定を伴います。
分解能を480とした場合、4分音符のドレミファの入力は以下の様になります。
```
480: C  // `480:` がステップ  `C` がド
480: D
480: E
480: F
```

休符を入れる場合は音名を入れずにステップのみ入力します。
ド・レ・4分休符・ミを入力する場合は以下の様になります。

```
480: C
480: D
480:
480: E
```

一回のステップ指定で複数の音名を指定できます。
4分音符の長さでCコードの和音を入力する場合は以下の様になります。

```
480: C E G
```

ステップを入力するたびに、音符入力の開始地点となる現在位置が進んでいきます。
現在位置が後ろに戻ることはありません。

### ステップとチャネルの考え方
ステップ入力による現在位置はチャネルごとに独立しています。
違うチャネルに切り替えると、現在位置はゼロの状態になりますが、再びもとのチャネルに戻った場合はチャネルを切り替える前の位置に戻ります。

```
480: C

CHANNEL 2
480: C

CHANNEL 1
480: D
480: E
480: F
```

上記の結果は以下のようになります。

```
            | 1
==============================
Ch1 |060:C3 | .
    |059:B  | .
    |058:A# | .
    |057:A  | .
    |056:G# | .
    |055:G  | .
    |054:F# | .
    |053:F  | .           x---
    |052:E  | .       x---
    |051:D# | .
    |050:D  | .   x---
    |049:C# | .
    |048:C2 | x---
------------------------------
Ch2 |060:C3 | .
    |059:B  | .
    |058:A# | .
    |057:A  | .
    |056:G# | .
    |055:G  | .
    |054:F# | .
    |053:F  | .
    |052:E  | .
    |051:D# | .
    |050:D  | .
    |049:C# | .
    |048:C2 | x---
```

チャネルの初期値は1です。

### コメント
`//`、`==`、`--`、から改行まではコメントとして処理されます。
ただし、`hoge---hoge` の様な記述は識別子として処理されます。
複数行のコメントは`/*`から `*/` までがコメントとして処理されます。

### 文字列
`' '` もしくは `" "` で囲まれた部分は文字列として処理されます。

```
'これは 文字列です'
"これも 文字列です"
```

文字列は[TITLE](#TITLE) や [COPYRIGHT](#COPYRIGHT) 文で使用します。

### マクロ
マクロにより、事前に定義された内容と置き換えができます。
リズムトラックの音名に別名をつけるなどの目的で使用します。

```
#define SD E1

480: SD // これは E1として展開されます。
        // SDはスネアドラムの意味です。
```

関数型マクロも使用できます。

```
#define CHORD_WITH_ROOT_ACCENT(a,b,c) a V=127 b c

480: CHORD_WITH_ROOT_ACCENT(C,E,G) // これは C V=127 E G として展開されます
480: CHORD_WITH_ROOT_ACCENT(D,F,Z) // これは D V=127 F Z として展開されます
```

`V=127`はベロシティの指定です。
詳細は[NOTE](#NOTE)を参照して下さい。

### インクルード
インクルードにより、他のファイルの定義などを読み込むことが出来ます。

```
#include 'drums.nas'
```

インクルードされるファイルは[インクルードサーチパス](TODO)で指定されたディレクトリ配下に配置して下さい。

他のファイルに、リズムトラックのマクロを定義したり、よく使うパターンを定義して使用出来ます。
パターンの定義については[PATTERN](#PATTERN)を参照して下さい。


### 改行とセミコロン
改行は一つの構文の区切りとして作用します。
例えば以下の様にチャネルの指定に改行を挟んだ場合はエラーになります。

```
CHANNEL // 数値の指定が無いまま改行が現れたためエラー
1
```

一行で複数の構文を入力する際にエラーになる場合はセミコロンで区切りを付けます。

```
GATETIME STEP  120: C D E F  // この場合、120がGATETIMEの引数として扱われるのでエラー
GATETIME STEP; 120: C D E F  // セミコロンで区切ると正しく解釈される
```


リファレンス
------------
### STEP
数値 + コロン(:) で指定します。
数値とコロンの間にはスペースを入れることが出来ます。

```
   0:
 480:
1920:

0   :
480 :
1920:
```

### NOTE
#### 音階
音階の入力は 音名(CDEFGAB) + 臨時記号(#/b/n) + オクターブ(-2 ~ 8) で行います。
臨時記号とオクターブは省略できます。

オクターブを省略した場合、直前に入力した音階のオクターブになります。
オクターブの初期値は2です。
\#/bはそれぞれダブルシャープ(##)・ダブルフラット(bb)の指定も可能です。
ナチュラル(n)は[KEY](#KEY)文でキーを指定した場合に作用します。

```
480: C     // オクターブを省略しているので初期値のオクターブが反映されて C2
480: C4    // オクターブを指定しているので C4
480: C#    // オクターブを初略しているので直前のオクターブが反映されて C#4
480: Cb2   // C♭2  (B1と同じ)
480: C##2  // C##2 (D2と同じ)
480: Cbb2  // C♭♭2 (A#1と同じ)

KEY CMin   // Cマイナーキーを指定

480: E2    // CメジャーキーのE♭2 として扱われる
480: En2   // CメジャーキーのE2 として扱われる
```

#### ベロシティ
音階に続けて `V=<ベロシティ>` と記述すると音の強さを指定できます。
ベロシティの有効な値の範囲は0~127です。
ベロシティを省略した場合、[VELOCITY](#VELOCITY)文で指定したデフォルトの値として扱われます。

```
VELOCITY 100  // デフォルトベロシティの指定

480: C        // ベロシティ=100
480: C V=127  // ベロシティ=127
```

#### ゲートタイム
音階に続けて `GT=<ゲートタイム>` と記述すると音の長さを指定できます。
ゲートタイムの有効な値の範囲は0~65535です。

```
480: C V=100 GT=240  // ベロシティ100、ゲートタイム240
```

ゲートタイムを省略した場合、[GATETIME](#GATETIME)文の指定により長さが決まります。
ゲートタイムの指定方法は２つあり、デフォルトでは省略するとステップと同じ長さとして扱われます。
詳細は[GATETIME](#GATETIME)文を参照して下さい。

### TITLE
演奏データのタイトルを設定します。
一つの演奏データにつき、１回だけ指定できます。
SMFファイルに出力した際、第一トラックにシーケンス名(FF 03h)として出力されます。

```
TITLE 'Syntax Reference'
```

### COPYRIGHT
演奏データの著作件情報を設定します。
一つの演奏データにつき、１回だけ指定できます。
SMFファイルに出力した際、第一トラックに著作権表示(FF 02h)として出力されます。
```
COPYRIGHT 'Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.'
```

### RESOLUTION
4分音符の分解能を指定します。
デフォルトは480です。
一つの演奏データにつき、１回だけ指定できます。
SMFファイルに出力した際、ヘッダチャンクの時間単位の値になります。
```
RESOLUTION 960
```

### TEMPO
Beat per Minutes(1分間の４分音符の数)の単位でテンポを指定します。
デフォルトは120.0です。
小数点第2位まで指定できます。
演奏データの途中から変更可能です。
SMFファイルに出力した際、第一トラックにテンポチェンジイベント(FF 51h 03h)として出力されます。

```
TEMPO 128.99
```

### TIME
拍子を指定します。
演奏データの途中から変更可能です。
SMFファイルに出力した際、第一トラックに拍子イベント(FF 58h 04h)として出力されます。

```
TIME 3/4
```

### KEY
調号を指定します。
音階名 + major or minor で指定します。
major/minorはmaj/minとして省略出来ます。
音階名とmajor/minorの間にはスペースを入れることが出来ます。

```
KEY CMin    // Cマイナーキーを指定
KEY D Major // Dメジャーキーを指定
```

※ チャネル単位ではなく演奏データ全体(リズム用チャネルなど)に影響することに注意して下さい。

指定可能な調号は以下になります。

|メジャー|マイナー|
|--------|--------|
|C Major |A Minor |
|G Major |E Minor |
|D Major |B Minor |
|A Major |F# Minor|
|E Major |C# Minor|
|B Major |G# Minor|
|F# Major|D# Minor|
|C# Major|A# Minor|
|F Major |D Minor |
|Bb Major|G Minor |
|Eb Major|C Minor |
|Ab Major|F Minor |
|Db Major|Bb Minor|
|Gb Major|Eb Minor|
|Cb Major|Ab Minor|

SMFファイルに出力した際、第一トラックに調号イベント(FF 59h 02h)として出力されます。

### VELOCITY
[NOTE](#NOTE)入力時のデフォルトベロシティを設定します。
指定可能な値は0~127です。
初期値は100です。

```
VELOCITY 80
```

### GATETIME
[NOTE](#NOTE)入力時の音の長さを指定します。

#### 絶対値指定
NOTE入力時にゲートタイムを省略した場合、絶対値指定で指定されたゲートタイムになります。

```
GATETIME 120

480: C // ゲートタイムは120
```

#### STEP指定
NOTE入力時にゲートタイムを省略した場合、ゲートタイムはNOTE入力時のSTEPと同じ長さになります。

```
GATETIME STEP

480: C // ゲートタイムは480
```

#### STEP + 相対値指定

NOTE入力時にゲートタイムを省略した場合、ゲートタイムはNOTE入力時のSTEPから指定した値を増減させた長さになります。

```
GATETIME STEP -10
480: C // ゲートタイムは470
GATETIME STEP +120
480: C // ゲートタイムは600
```

### MARKER
マーカーを挿入します。
プレイヤーのリピート状態が[リピートマーカー](TODO)になっている場合、リピート再生のセクション区切りとして作用します。
SMFファイルに出力した際、第一トラックにマーカーイベント(FF 06h)として出力されます。

```
MARKER 'Intro'
```

### CHANNEL
チャネルを切り替えます。
使用可能なチャネルは1~16です。

```
CHANNEL 10
```

### SYNTH
チャネルのサウンドフォントを切り替えます。
予め指定するサウンドフォントが読み込まれている必要があります。
[シンセサイザー設定](../operation_manual-preference.md#Synthesizer_Settings)も参照して下さい。

```
SYNTH 'SGM-V2.01'
```

### BANK
チャネルの音色変更で使用します。
MIDIメッセージのバンクセレクトに相当します。

```
BANK 128 // Bank No=128 (MSB=1, LSB=0)
```

SMFファイルに出力した際、

- コントロールチェンジ:バンクセレクトMSB(Bn 00h)
- コントロールチェンジ:バンクセレクトLSB(Bn 20h)

として出力されます。
[PROGRAM](#PROGRAM)も参照して下さい。

### PROGRAM
チャネルの音色変更で使用します。
MIDIメッセージのプログラムチェンジに相当します。

```
BANK 128  // Bank No=128   一般的なGeneral MIDI対応シンセサイザーでのパーカッションバンク
PROGRAM 0 // Program No=0  Standard Drums
```

SMFファイルに出力した際、プログラムチェンジ(Cn) として出力されます。

※ 音色が切り替わるのはPROGRAMイベントが再生された時になる事に注意して下さい

### VOLUME
チャネルのボリュームを指定します。
指定可能な値は0~127です。
デフォルトは100です。
SMFファイルに出力した際、コントロールチェンジ(Bn 07h) チャネルボリューム として出力されます。

```
VOLUME 100
```

### CHORUS
チャネルのコーラスエフェクトのかかり具合を変更します。
指定可能な値は0~127です。
デフォルトは0です。

SMFファイルに出力した際、コントロールチェンジ(Bn 5Dh) エフェクト3デプスとして出力されます。

```
CHORUS 40
```

### REVERB
チャネルのリバーブエフェクトのかかり具合を変更します。
指定可能な値は0~127です。
デフォルトは0です。
SMFファイルに出力した際、コントロールチェンジ(Bn 5Bh) エフェクト1デプスとして出力されます。

```
REVERB 40
```

### EXPRESSION
チャネルのエクスプレッションを指定します。
指定可能な値は0~127です。
デフォルトは127です。
SMFファイルに出力した際、コントロールチェンジ(Bn 0Bh) エクスプレッション として出力されます。

```
EXPRESSION 100
```

### PAN
チャネルのパン(左右の定位)を指定します。
指定可能な値は-64 ~ +64です。
マイナスに行くほど左に振れ、プラスに行くほど右に振れます。
デフォルトは0(中央)です。

```
PAN -40
```

SMFファイルに出力した際、コントロールチェンジ(Bn 0Ah) パンとして、値を0~127に変換して出力されます。

### TRANSPOSE
[NOTE](#NOTE)入力時の音階を半音単位で上下させます。
1音上げの場合`+2`です。
指定可能な値は-24 ~ +24です。
デフォルトは0です。

```
TRANSPOSE +2
```

### DETUNE
シンセサイザのピッチをセント単位で調整します。
指定可能な値は上下2オクターブ(-2400 ~ +2400)です。
[TRANSPOSE](#TRANSPOSE)が音階を変更するのに対し、DETUNEでは音階は変えずにシンセサイザのピッチを変更します。

```
DETUNE -50
```

SMFファイルに出力した際、

- コントロールチェンジ:RPN（MSB）(Bn 65h 0) チャネル・ファイン・チューニング
- コントロールチェンジ:RPN（LSB）(Bn 64h 1) チャネル・ファイン・チューニング
- コントロールチェンジ:データエントリー(MSB) (Bn 06h)
- コントロールチェンジ:データエントリー(LSB) (Bn 26h)
- コントロールチェンジ:RPN（MSB）(Bn 65h 0) チャネル・コース・チューニング
- コントロールチェンジ:RPN（LSB）(Bn 64h 2) チャネル・コース・チューニング
- コントロールチェンジ:データエントリー(MSB) (Bn 06h) 
- コントロールチェンジ:RPN（MSB）(Bn 65h 7Fh) RPNヌル
- コントロールチェンジ:RPN（LSB）(Bn 64h 7Fh) RPNヌル

として出力されます。

### PATTERN
繰り返し現れるフレーズなどをパターンとして定義できます。
パターンは以下の構文で定義します。

```
PATTERN <識別子>
  ... // 任意の文
END
```

識別子は以下の条件を満たしている必要があります。

- ー文字以上
- 数字、アルファベット、アンダースコア(_) のいずれかから始まる
- 二文字以降は数字、アルファベット、アンダースコア(_)、ハイフン(-)のいずれか
- 他の構文で使う単語と衝突しない

定義したパターンは[EXPAND](#EXPAND)文で展開します。

入れ子のパターンを定義することが出来ます。
親パターンの外と中で展開時の識別子の指定方法が異なります。

```
PATTERN parent
  PATTERN child-A
    ...
  END

  PATTERN child-B
    ...
  END

  EXPAND child-A // 同じパターン内で定義したパターンはそのまま識別子を指定して展開
END

EXPAND parent:child-B // パターンの外から入れ子になっているパターンを展開する場合は、
                      // <親パターン名>:<子パターン名> の様に識別子を指定する

```

パターンの展開はパターンの定義より先に書くことが出来ます。

```
EXPAND Intro

PATTERN Intro
  ...
END
```

### EXPAND
定義済みパターンを展開します。
入れ子のパターンはコロン(:)を付けて展開できます。

詳細は[PATTERN](#PATTERN)を参照して下さい。