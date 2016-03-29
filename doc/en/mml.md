MML for NASequencer Syntax
==========================
Version 0.5.0 Draft, 2016

About MML
---------
Music Macro Language (MML) is a music description language that was invented in the early days of computer music for sequencing by text descriptions.
For historical reasons, there are many implementations and no standards of language definition.
The MML on NASequencer bases on notation of [FIMML](https://flmml.codeplex.com) that is a Flash library commonly used in Japan.
However, notations for sound source is not compatible because NASequencer focuses MIDI in contrast that FIMML focuses FM oscillator.
For notations for MIDI, NASequencer adopts several notations of [mml2mid](http://hpc.jp/~mml2mid/mml2mid.txt) and adds NASequencer's own notations.


Basics
------
### File Format
- File extension must be '.mml'
- Source file is written by plain-text
- All keyword and identifiers are case-insensitive except macro identifier
- Writing code shold be ascii charset except string literal and comment
- Supported file encodings are ascii and utf-8 only

### Track Separator
Semicolon(;) acts as track separator and switches current track.
If current track switches to new track, current location for note event will be zero.

#### Track and Channel
Channel of MIDI is not synchronized with track.
Switching channel is by [@ch](#@ch) command.

#### Example for Track and Channel

```
cccc;
eeee;

@CH2
cdef;
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
    |053:F  | .
    |052:E  | x---x---x---x---
    |051:D# | .
    |050:D  | .
    |049:C# | .
    |048:C2 | x---x---x---x---
------------------------------
Ch2 |060:C3 | .
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
```

Initial channel number is 1.

### Comment
`//` can be used for the line comment. It does not affect the result of sequence.
`/* */` is also available for multiple line comments.

### String
`' '` or `" "` is treated as string literal.

```
'This is string.'
"This is string too."
```

String is used by [\#title](##title) and [\#copyright](##copyright) directives。

### Macro
Macro is a identifier which will be replaced by a code fragment.
In some cases, it is used to give symbolic names to notes for percussion track.
A macro is defined by `$<Identifier> = <Replacement>;`.

```
$MACRO = abcd;

$MACRO // This will be replaced by abcd
```

Function type macro is also available.
Defined by `$<Identifier>{<Argument1>, <Argument2>[, ...]} = <Replacement>;`
Replacement with `%<Argument Name>` will be replaced with the argument on the macro is referenced.

```
$m{vol, note} = @V%vol %note;

$m{100, CDE} // This will be replaced by @V100 CDE
```

_Excerpted by [FIMML Detail of Macro](https://flmml.codeplex.com/wikipage?title=Reference&referringTitle=Documentation#fl_macro)_

**Note:**  
Macro function confirms to the FIMML specification.

### Include
Include directive imports the codes written in a sepalate source file.

```
#include 'drums.mml'
```

Included file must be placed in the directory that is specified by [Include Search Path](TODO).

This directive is used for importing a macro, a initialization or etc those are used frequently.

References
----------
### Note
#### Pitch
Pitch is notated by pitch names (CDEFGAB) + accidentals (+/#/-).
Accidentals can be omitted.
Double Sharp (##/++) and Double Flat (--) is acceptable.

```
C    // C
C+   // C#
C++  // C## (Same as D)
C#   // C#
C##  // C## (Same as D)
D-   // D♭
D--  // D♭♭ (Same as C)
```

#### Note Length
Length of note can be notated by a number follow after pitch.
A number represents for N of 1/N note length.
Length can be omitted and if it is so, value of [Default Note Length](#Default_Note_Length) is applied to note.

```
C4 // Quarter note
```

Dotted note is represened by dot(.) follow after note length,

```
C4. // Dotted quarter note
```

Double-dotted note is acceptable.

```
C4.. // 7/4 of quarter note length
```

Absolute note length with `%` is acceptable.

```
C%480 // Quarter note length on that resolution of quarter note is 480
```

### REST
Rest evetn is notated by `R`.
In the same way for [Note](#Note), length of rest can be notated by a number follow after rest.
Length can be omitted and if it is so, value of [Default Note Length](#Default_Note_Length) is applied to rest.

```
R4 // quarter rest
```
