Note as Sequence Syntax
=======================
Version 0.5.0 Draft, 2016

Basics
------
### File Format
- File extension must be '.nas'
- Source file is written by plain-text
- All keyword and identifiers are case-insensitive
- Writing code shold be ascii charset except string literal and comment
- Supported file encoding is ascii or utf-8 only

### About Step
Indicating note events are preceded by Step.
If resolution of quarter note is 480, quarter notes of C D E F will be below.

```
480: C  // `480:` is step  `C` is note of C
480: D
480: E
480: F
```

Rest is indicated by writing step without following note.
Quarter notes of C D rest F will be below.

```
480: C
480: D
480:
480: E
```

Multiple note events are acceptable on one step notation.
Quarter note length of C major will be below.

```
480: C E G
```

Each time step is indicated, current location forwards by step value.
Current location naver backwards.

### Step with Channel
Current location is channel individual.
If current channel switches to new channel, current location will be zero.
However, current channel back to previous channel, current location also back to the location before switching channel.

```
480: C

CHANNEL 2
480: C

CHANNEL 1
480: D
480: E
480: F
```

The result of avobe sequence will be following.

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

Default channel number is 1.

### Comment
`==` `--` `//` can be used the line comment. It does not affect the result of sequence.
An exception is that code like `hoge---hoge` is treated as identifier;
`/* */` is also available for multiple line comments.

### String
`' '` or `" "` is treated as string literal.

```
'This is string.'
"This is string too."
```

String is used by [TITLE](#TITLE) and [COPYRIGHT](#COPYRIGHT) statements.

### Macro

TODO...
