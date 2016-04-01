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
- Writing code should be ascii charset except string literal and comment
- Supported file encodings are ascii and utf-8 only

### Track Separator
Semicolon(;) acts as track separator and switches current track.
If current track switches to new track, current location for note event will be zero.

#### Track and Channel
Channel of MIDI is not synchronized with track.
Switching channel is by [Channel](#Channel) command.

#### Example for Track and Channel

```
cccc;
eeee;

@CH2
cdef;
```

The result of above sequence will be following.

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

String is used by [Title](#Title) and [Copyright](#Copyright) directives。

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

<i>Excerpted from [FlMML - Detail of Macro](https://flmml.codeplex.com/wikipage?title=Reference&referringTitle=Documentation#fl_macro)</i>

**Note:**  
Macro function confirms to the FlMML specification.

### Include
Include directive imports the codes written in a separate source file.

```
#include 'drums.mml'
```

Included file must be placed in the directory that is specified by [Preferences - Include Search Path](preference.md#Include Search Path).

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
Length can be omitted and if it is so, value of [Default Note Length](#Default Note Length) is applied to note.

```
C4 // Quarter note
```

Dotted note is represented by dot(.) follow after note length,

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
Length can be omitted and if it is so, value of [Default Note Length](#Default Note Length) is applied to rest.

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

Behavior of '<' and '>' can be reversed by [Octave Reverse](#Octave Reverse) directive.

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
In the case that the specified number is more than 384 or the result of calculated length is not a multiple of 1/384 note length,
NASequencer deals with it as error.
Acceptable values are followings.

```
L384 L192 L128 L96 L64 L48 L32 L24 L16 L12 L8 L6 L4 L3 L2 L1
```

### Gatetime
#### Gatetime by Ratio
Specify ratio of gatetime.
Valid value range is 0 to 16.
The gatetime of note will be specified value / 16.
Initial value is 15.

```
Q14
C4 // Gatetime will be 14/16 of quarter note length
```

#### Minus Value of Gatetime Ratio
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

How much one step changes velocity is different by the last velocity command which is used `V` or `@V`.
With number follow after '(' or ')', how many steps up/down velocity can be specified.

```
V10 // Velocity will be 87
(2  // Up 2 steps(8x2=16) so velocity will be 103

@V100 // Velocity will be 100
)5  // Up 5 steps(1x5=5) so velocity will be 105
```

Behavior of '(' and ')' can be reversed by [Velocity Reverse](#Velocity Reverse) directive.

### Tuplet
Notes surrounded by `{}` acts as tuplet.
Note length can be specified after `{}`.
If note length is omitted, [Default Note Length](#Default Note Length) is applied to tuplet.
Length of each note in tuplet will evenly be &lt;note length of tuplet&gt; / &lt;count of notes in tuplet&gt;.

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

The result of above sequence will be following.

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

<i>Excerpted from [FlMML - About Number of Voices, Chord Notation](https://flmml.codeplex.com/wikipage?title=Reference&referringTitle=Documentation#fl_poly)</i>

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
Indicates repeat phrase with `/:[<Number of repeat count>] ... / ... :/`.
If &lt;Number of repeat count&gt; is omitted, repeat count will be twice.
In last loop of repeating, if '/' is there, exit from loop.
'/' can be omitted.

```
/:3 cde / fg :/  // This will equivalent to `cde fg cde fg cde`
```

### Title
Indicates the title of sequence.
Indicating title is only once per sequence.
Exported to Standard MIDI File as Sequence Name (FF 03h) in first MTrk chunk.

```
#title 'Syntax Reference'
```

### Copyright
Indicates the copyright of sequence.
Indicating copyright is only once per sequence.
Exported to Standard MIDI File as Copyright Notice (FF 02h) in first MTrk chunk.

```
#copyright 'Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.'
```

### Marker
Inserts a marker.
Acts as separator of sections for repeat play if [Repeat State](general.md#Repeat State) on the player is Repeat Marker.
Exported to Standard MIDI File as Marker(FF 06h) in first MTrk chunk.

```
MARKER 'Intro'
```

**Attention:**  
Repeat for marker is different from [Repeat](#Repeat) notation.
Marker repeating is NASequencer's own mechanism.
By contrast with that [Repeat](#Repeat) affects to sequence events, marker repeating is for playback control on document window.

### Velocity Reverse
Reverses behavior for [Velocity Shift](#Velocity Shift)

```
#velocity reverse
```

Default behavior is below.

|Function|Symbol|
|---|---|
|Velocity Up| `(` |
|Velocity Down| `)` |

### Octave Reverse
Reverses behavior for [Octave Shift](#Octave Shift)

```
#octave reverse
```

Default behavior is below.

|Function|Symbol|
|---|---|
|Octave Up| `<` |
|Octave Down| `>` |

### Tempo
Indicates tempo change by unit of beat per minutes.
Default value is 120.0.
Valid tempo range is 30.0 to 200.0 and can be specified up to the second decimal place.
Can be indicated middle of the sequence.
Exported to Standard MIDI File as Set Tempo(FF 51h 03h) in first MTrk chunk.

```
t 128.99
```

### Channel
Switches current channel.
Valid channel number is 1 to 16.

```
@ch 10
```

### Synthesizer
Changes synthesizer of current channel.
The specified SoundFont has to be loaded in advance.
See also [Synthesizer Settings](preference.md#Synthesizer Settings).

```
@sy 'SGM-V2.01'
```

### Bank Select
Used for changing preset of current channel.
Equivalent to Bank Select of MIDI message.

```
@bs 128 // Bank No=128 (MSB=1, LSB=0)
```

Exported to Standard MIDI File as followings.

- Control Change:Bank Select MSB (Bn 00h)
- Control Change:Bank Select LSB (Bn 20h)

See also [Programe Change](#Program Change).

### Program Change
Used for changing preset of current channel.
Equivalent to Program Change of MIDI message.

```
@bs 128 // Bank No=128   Percussion bank of common synthesizers compliant General MIDI
@pc 0   // Program No=0  Standard Drums
```

Exported to Standard MIDI File as Program Change(Cn).

**Attention:**  
Change timing of preset is at the moment of sending program change event to synthesizer.

### Volume
Specifies volume of current channel.
Valid volume range is 0 to 127.
Initial value is 100.
Exported to Standard MIDI File as Channel Volume of Control Change (Bn 07h).

```
@vl100
```

### Chorus
Specifies chorus send level of current channel.
Valid value range is 0 to 127.
Initial value is 0.
Exported to Standard MIDI File as Effects 3 Depth of Control Change (Bn 5Dh).

```
@cs40
```

### Reverb
Specifies reverb send level of current channel.
Valid value range is 0 to 127.
Initial value is 0.
Exported to Standard MIDI File as Effects 1 Depth of Control Change (Bn 58h).

```
@rv40
```

### Expression
Specifies expression level of current channel.
Valid value range is 0 to 127.
Initial value is 127.
Exported to Standard MIDI File as Expression Controller of Control Change (Bn 0Bh).

```
@x100
```

### Pan
Specifies pan position of current channel.
Valid value range is -64 to +64.
-64 for left, 0 for center and +64 for right.
Initial value is 0 (center).
Exported to Standard MIDI File as Pan of Control Change (Bn 0Ah) with converting value 0 to 127.

```
@p30
```

### Pitch Bend
Specifies pitch bend of current chanel.
Used for choking or arming of guiter, vibrato or etc.
Initial value is 0.

#### Coarse Pitch Bend Value
Valid value range is -64 to 63.
Pitch bend value will be specified value x 128.

```
pb +63
```

#### Absolute Pitch Bend Value
Valid value range is -8192 to +8191.

```
@pb +8191
```

Exported to Standard MIDI File as Pitch Bend Change (En) with converting value 0 to 16383.

See also [Pitch Bend Sensitivity](#Pitch Bend Sensitivity).

### Pitch Bend Sensitivity
Configs pitch bend range of current channel in a unit of semitone.
Valid value range is 0 to 24.
Initial value is 2.

```
@ps 4
```

Exported to Standard MIDI File as followings.

- Control Change:RPN（MSB）(Bn 65h 00h) Pitch Bend Sensitivity
- Control Change:RPN（LSB）(Bn 64h 00h) Pitch Bend Sensitivity
- Control Change:Data Entry (MSB) (Bn 06h) 
- Control Change:RPN（MSB）(Bn 65h 7Fh) RPN NULL
- Control Change:RPN（LSB）(Bn 64h 7Fh) RPN NULL

### Detune
Adjusts pitch of synthesizer in a unit of cent.
Valid value range is -2400 to +2400.


**Note:**  
By contrast with that [Note Shift](#Note Shift) changes scale of note, Detune change tuning of synthesizer.

```
@d-50
```

Exported to Standard MIDI File as followings.

- Control Change:RPN（MSB）(Bn 65h 00h) Channel Fine Tuning
- Control Change:RPN（LSB）(Bn 64h 01h) Channel Fine Tuning
- Control Change:Data Entry (MSB) (Bn 06h)
- Control Change:Data Entry (LSB) (Bn 26h)
- Control Change:RPN（MSB）(Bn 65h 00h) Channel Coarse Tuning
- Control Change:RPN（LSB）(Bn 64h 02h) Channel Coarse Tuning
- Control Change:Data Entry (MSB) (Bn 06h) 
- Control Change:RPN（MSB）(Bn 65h 7Fh) RPN NULL
- Control Change:RPN（LSB）(Bn 64h 7Fh) RPN NULL
