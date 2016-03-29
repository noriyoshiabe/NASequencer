MML for NASequencer Syntax
==========================
Version 0.5.0 Draft, 2016

About MML
---------
Music Macro Language (MML) is a music description language that was invented in the early days of computer music for sequencing by text descriptions.
For historical reasons, there are many implementations and no standards of language definition.
The MML on NASequencer bases on notation of [FlMML](https://flmml.codeplex.com) that is a Flash library commonly used in Japan.
However, notations for sound source is not compatible because NASequencer focuses MIDI in contrast that FlMML focuses FM oscillator.
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

_Excerpted from [FlMML Detail of Macro](https://flmml.codeplex.com/wikipage?title=Reference&referringTitle=Documentation#fl_macro)_

**Note:**  
Macro function confirms to the FlMML specification.

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

### Rest
Rest event is notated by `R`.
In the same way for [Note](#Note), length of rest can be notated by a number follow after rest.
Length can be omitted and if it is so, value of [Default Note Length](#Default_Note_Length) is applied to rest.

```
R4 // quarter rest
```

### Octave
#### Absolute Setting
Specifies octave for [Note](#Note).
Valid octave range is -2 to +8.
Initial value is +2.

```
@o4  // Octave 4
```

#### Octave Shift
##### <
Ups octave relatively by current setting.

##### >
Downs octave relatively by current setting.

Behavior of '<' and '>' can be reversed by [#octave reverse](##octave_reverse) directive.

### Timebase
Changes config of ticks per quarter note.
Default value is 480.
Changing is only once per sequence.
Exported to Standard MIDI File as division(ticks per quarter-note) in Header Chunks.

```
#timebase 960
```

### Note Shift
#### Absolute Note Shift
Ups/downs pitch of note events in a unit of semitone.

```
NS2  // Whole tone up
C    // This will be D
NS-2 // Whole tone down
C    // This will be A#
```

#### Relative Note Shift
Ups/downs pitch of note events in a unit of semitone.
Difference from Absolute Note Shift is that Relative Note Shift ups value of note shift relatively from current value of note shift.

```
@NS2  // Whole tone up
C     // This will be D
@NS-2 // Whole tone down from whole tone up
C     // This will be back to C
```

### Default Note Length
Specifies default note length for when length of [Note](#Note) is omitted.
Valid value range is 1 to 384.
Initial value is 4.
In the case that the specified number is more than 384 or the result of caluclated length is not a multiple of 1/384 note length,
NASequencer deals with it as error.
Acceptable values are followings.

```
L384 L192 L128 L96 L64 L48 L32 L24 L16 L12 L8 L6 L4 L3 L2 L1
```

### Gatetime
#### Gatetime by Ratio
Specify ratio of gatetime.
Valid value range is 0 to 16.
The gatetime of note will be <value>/16.
Initial value is 15.

```
Q14
C4 // Gatetime will be 14/16 of quarter note length
```

#### Minux Value of Gatetime Ratio
Subtract length of specified value / 192 from result of gatetime with Gatetime by Ratio.

```
Q16
@Q 2
C4 // Gatetime will be 190/192 of quarter note length
```

### Velocity
#### Coarse Velocity Value
Coarsely specifies velocity for [Note](#Note).
Valid value range is 0 to 15.
Velocity value will be specified value x 8 + 7.

```
V10  // Same as @V87
```

#### Absolute Velocity Value
Specifies velocity for [Note](#Note) with absolute value.
Valid value range is 0 to 127.
Initial value is 100.

```
@V120  // Velocity 120
```

#### Velocity Shift
##### (
Ups velocity one step relatively by current setting.

##### )
Downs velocity one step relatively by current setting.

Differents how much one step changes velocity by the last velocity command whith is used `V` or `@V`.
With number follow after '(' or ')', how many steps up/down velocity can be specified.

```
V10 // Velocity will be 87
(2  // Up 2 steps(8x2=16) so velocity will be 103

@V100 // Velocity will be 100
)5  // Up 5 steps(1x5=5) so velocity will be 105
```

Behavior of '(' and ')' can be reversed by [#velocity reverse](##velocity_reverse) directive.

### Tuplet
Notes surrounded by `{}` acts as tuplet.
Note length can be specified after `{}`.
If note length is omitted, [Default Note Length](#Default_Note_Length) is applied to tuplet.
Length of each note in tuplet will evenly be \<note length of tuplet\> / \<count of notes in tuplet\>.

### Chord
Notes surrounded by `[]` acts as chord.

```
L4
[CEG] // C chord with length of quarter note
```

With 'R' command, timing of sound can be delayed.

```
L8 [C4.RERG]D. C2
```

The result of avobe sequence will be following.

```
            | 1               2
===============================
Ch1 |060:C3 | .               .
    |059:B  | .               .
    |058:A# | .               .
    |057:A  | .               .
    |056:G# | .               .
    |055:G  | .   x-          .
    |054:F# | .               .
    |053:F  | .               .
    |052:E  | . x-            .
    |051:D# | .               .
    |050:D  | .     x--       .
    |049:C# | .               .
    |048:C2 | x-----   x-------
```

_Excerpted from [FlMML About Number of Voices, Chord Notation](https://flmml.codeplex.com/wikipage?title=Reference&referringTitle=Documentation#fl_poly)_

### Tie
'&' between note and note with same pitch acts as tie.

```
C4 & C8 // In this case、note length will equals length of dotted quarter note
```

Tie can be placed between chord and chord.

```
L4
[CEG] & [CEG] // This will be C chord with half note length
```

If the notes joined by tie are not same pitch, tie does not affect to the notes.

**Note:**  
NASequencer does not support slur with '&'.

### Repeat
