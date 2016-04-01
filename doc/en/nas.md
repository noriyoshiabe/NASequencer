Note as Sequence Syntax
=======================
Version 0.5.0 Draft, 2016

Basics
------
### File Format
- File extension must be '.nas'
- Source file is written by plain-text
- All keyword and identifiers are case-insensitive except macro identifier
- Writing code should be ascii charset except string literal and comment
- Supported file encodings are ascii and utf-8 only

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
Current location never backwards.

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

Initial channel number is 1.

### Comment
`==` `--` `//` can be used for the line comment. It does not affect the result of sequence.
An exception is that code like `hoge---hoge` is treated as identifier.
`/* */` is also available for multiple line comments.

### String
`' '` or `" "` is treated as string literal.

```
'This is string.'
"This is string too."
```

String is used by [TITLE](#TITLE) and [COPYRIGHT](#COPYRIGHT) statements.

### Macro
Macro is a identifier which will be replaced by a code fragment.
In some cases, it is used to give symbolic names to notes for percussion track.
Identifier for macro is case-sensitive.

```
#define SD E1

480: SD // This will be replaced by E1.
        // SD means snare drum.
```

Function type macro is also available.

```
#define CHORD_WITH_ROOT_ACCENT(a,b,c) a V=127 b c

480: CHORD_WITH_ROOT_ACCENT(C,E,G) // This will be replaced by C V=127 E G
480: CHORD_WITH_ROOT_ACCENT(D,F,Z) // This will be replaced by D V=127 F Z
```

`V=127` is assignment for velocity.
See [NOTE](#NOTE) for the details.

### Include
Include directive imports the codes written in a separate source file.

```
#include 'drums.nas'
```

Included file must be placed in the directory that is specified by [Preferences - Include Search Path](preference.md#Include Search Path).

This directive is used for importing a macro, a rhythm pattern or etc those are used frequently.
See [PATTERN](#PATTERN) for definition of pattern statement.

### Line-Break
Line-break acts as end of statement.
If line-break is put into channel statement like below, it is treated as error.

```
CHANNEL // Error due to the line-break appears without channel number
1
```

#### Semicolon

If an error occurs with statements in one line, semicolon can be used for end of statement. 

```
GATETIME STEP  120: C D E F  // In this case, 120 is treated as argument of gatetime statement
                             // so ':' without step length causes error.
GATETIME STEP; 120: C D E F  // Sepalate statements by semicolon will be correctly interpreted.
```

References
----------

### STEP
Indicated by number followed by ':'.
White-space can be put into between number and ':'.

```
   0:
 480:
1920:

0   :
480 :
1920:
```

See also [About Step](#About Step).

### NOTE
#### Pitch
Pitch is notated by pitch names (CDEFGAB) + accidentals (#/b/n) + octave (-2 to 8).
Accidentals and octave can be omitted.

If octave is omitted, octave will be a octave of last note.
Initial value of octave is 2.
`#` is for the Sharp, `b` is for the `Flat` and `n` is for Natural.
Double Sharp and Double Flat is acceptable.
Natural affecting to pitch is only in the case that [KEY](#KEY) is specified.

```
480: C     // Octave is omitted so octave is default value of 2. Pitch will be C2.
480: C4    // Octave is specified.  Pitch will be C4.
480: C#    // Octave is omitted so octave is last ocvave of 4. Pitch will be C#4.
480: Cb2   // C♭2  (Same as B1)
480: C##2  // C##2 (Same as D2)
480: Cbb2  // C♭♭2 (Same as A#1)

KEY CMin   // Specify C minor key

480: E2    // Treated as E♭2 on C major key
480: En2   // Treated as E2 on C major key
```

#### Specifying Velocity
Notating `V=<Velocity>` follow after pitch specifies level of sound.
Valid velocity range is 0 to 127.
If velocity is omitted, the default value that is specified by [VELOCITY](#VELOCITY) statement is applied.

```
VELOCITY 100  // Specify default value of velocity

480: C        // Velocity will be 100
480: C V=127  // Velocity will be 127
```

#### Specifying Gatetime
Notating `GT=<Gatetime>` follow after pitch specifies duration of sound.
Valid gatetime range is 0 to 65535.

```
480: C V=100 GT=240  // Velocity will be 100. Gatetime will be 240.
```

If gatetime is omitted, gatetime value is determined by config on [GATETIME](#GATETIME) statement.
There are 2 types of gatetime config. In default, gatetime will be equal to step length unless gatetime is specified.
See [GATETIME](#GATETIME) statement for more details.

### TITLE
Indicates the title of sequence.
Indicating title is only once per sequence.
Exported to Standard MIDI File as Sequence Name (FF 03h) in first MTrk chunk.

```
TITLE 'Syntax Reference'
```

### COPYRIGHT
Indicates the copyright of sequence.
Indicating copyright is only once per sequence.
Exported to Standard MIDI File as Copyright Notice (FF 02h) in first MTrk chunk.

```
COPYRIGHT 'Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.'
```

### RESOLUTION
Changes config of ticks per quarter note.
Default value is 480.
Changing is only once per sequence.
Exported to Standard MIDI File as division(ticks per quarter-note) in Header Chunks.

```
RESOLUTION 960
```

### TEMPO
Indicates tempo change by unit of beat per minutes.
Default value is 120.0.
Valid tempo range is 30.0 to 200.0 and can be specified up to the second decimal place.
Can be indicated middle of the sequence.
Exported to Standard MIDI File as Set Tempo(FF 51h 03h) in first MTrk chunk.

```
TEMPO 128.99
```

### TIME
Indicates time signature.
Can be indicated middle of the sequence.
Exported to Standard MIDI File as Time Signature(FF 58h 04h) in first MTrk chunk.

```
TIME 3/4
```

### MARKER
Inserts a marker.
Acts as separator of sections for repeat play if [Repeat State](general.md#Toggle Repeat) on the player is Repeat Marker.
Exported to Standard MIDI File as Marker(FF 06h) in first MTrk chunk.

```
MARKER 'Intro'
```

### KEY
Indicates key signature by tonic name + major or minor.
`major` and `minor` can be abbreviated to `maj` and `min`.
White-space can be put into between tonic name and major/minor.

```
KEY CMin    // Specify C Minor key
KEY D Major // Specify D Major key
```

Available key signatures are below.

|Major   |Minor   |
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

Exported to Standard MIDI File as Key Signature(FF 59h 02h) in first MTrk chunk.

### TRANSPOSE
Ups/downs pitch of note events in a unit of semitone.
Whole tone up is `+2`.
Valid value range is -64 to +64.
Initial value is 0.

```
TRANSPOSE +2
```

### CHANNEL
Switches current channel.
Valid channel number is 1 to 16.

```
CHANNEL 10
```

### PERCUSSION
Specifies percussion mode of current channel.
If percussion mode is on, settings of [KEY](#KEY) and [TRANSPOSE](#TRANSPOSE) will not affect to scale of note.

```
TRANSPOSE +2

C1  // This will be D1 by TRANSPOSE

PERCUSSION ON  // Percussion mode is on

C1  // TRANSPOSE does not affect to scale of note and it will stay C1

PERCUSSION OFF // Percussion mode is off
```

### VELOCITY
Specifies default velocity of current channel for [NOTE](#NOTE) statement.
Valid velocity range is 0 to 127.
Initial value is 100.

```
VELOCITY 80
```

### GATETIME
Specifies gatetime config of current channel for [NOTE](#NOTE) statement。

#### Absolute Setting
Applies to note event in absolute number if gatetime is omitted with note statement.

```
GATETIME 120

480: C // Gatetime will be 480
```

#### Step Setting
Applies to note event in the value that equals to step length if gatetime is omitted with note statement.

```
GATETIME STEP

480: C // Gatetime will be 480
```

#### Step + Relative Setting
Applies to note event in result of addition to step length if gatetime is omitted with note statement.

```
GATETIME STEP -10
480: C // Gatetime will be 470
GATETIME STEP +120
480: C // Gatetime will be 600
```

### SYNTH
Changes synthesizer of current channel.
The specified SoundFont has to be loaded in advance.
See also [Synthesizer Settings](preference.md#Synthesizer Settings).

```
SYNTH 'SGM-V2.01'
```

### BANK
Used for changing preset of current channel.
Equivalent to Bank Select of MIDI message.

```
BANK 128 // Bank No=128 (MSB=1, LSB=0)
```

Exported to Standard MIDI File as followings.

- Control Change:Bank Select MSB (Bn 00h)
- Control Change:Bank Select LSB (Bn 20h)

See also [PROGRAM](#PROGRAM).

### PROGRAM
Used for changing preset of current channel.
Equivalent to Program Change of MIDI message.

```
BANK 128  // Bank No=128   Percussion bank of common synthesizers compliant General MIDI
PROGRAM 0 // Program No=0  Standard Drums
```

Exported to Standard MIDI File as Program Change(Cn).

**Attention:**  
Change timing of preset is at the moment of sending program change event to synthesizer.

### VOLUME
Specifies volume of current channel.
Valid volume range is 0 to 127.
Initial value is 100.
Exported to Standard MIDI File as Channel Volume of Control Change (Bn 07h).

```
VOLUME 100
```

### CHORUS
Specifies chorus send level of current channel.
Valid value range is 0 to 127.
Initial value is 0.
Exported to Standard MIDI File as Effects 3 Depth of Control Change (Bn 5Dh).

```
CHORUS 40
```

### REVERB
Specifies reverb send level of current channel.
Valid value range is 0 to 127.
Initial value is 0.
Exported to Standard MIDI File as Effects 1 Depth of Control Change (Bn 58h).

```
REVERB 40
```

### EXPRESSION
Specifies expression level of current channel.
Valid value range is 0 to 127.
Initial value is 127.
Exported to Standard MIDI File as Expression Controller of Control Change (Bn 0Bh).

```
EXPRESSION 100
```

### PAN
Specifies pan position of current channel.
Valid value range is -64 to +64.
-64 for left, 0 for center and +64 for right.
Initial value is 0 (center).
Exported to Standard MIDI File as Pan of Control Change (Bn 0Ah) with converting value 0 to 127.

```
PAN +30
```

### PITCH
Specifies pitch bend of current channel.
Used for choking or arming of guitar, vibrato or etc.
Valid value range is -8192 to +8191.
Initial value is 0.

```
PITCH +8191
```

Exported to Standard MIDI File as Pitch Bend Change (En) with converting value 0 to 16383.

See also [PITCH SENSE](#PITCH SENSE).

### PITCH SENSE
Configures pitch bend range of current channel in a unit of semitone.
Valid value range is 0 to 24.
Initial value is 2.

```
PITCH SENSE 4
```

Exported to Standard MIDI File as followings.

- Control Change:RPN（MSB）(Bn 65h 00h) Pitch Bend Sensitivity
- Control Change:RPN（LSB）(Bn 64h 00h) Pitch Bend Sensitivity
- Control Change:Data Entry (MSB) (Bn 06h) 
- Control Change:RPN（MSB）(Bn 65h 7Fh) RPN NULL
- Control Change:RPN（LSB）(Bn 64h 7Fh) RPN NULL

### DETUNE
Adjusts pitch of synthesizer in a unit of cent.
Valid value range is -2400 to +2400.


**Note:**  
By contrast with that [TRANSPOSE](#TRANSPOSE) changes scale of note, DETUNE change tuning of synthesizer.

```
DETUNE -50
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

### PATTERN
Pattern is for naming identifier and reusing fragment of codes that appears repeatedly.
Pattern is defined by following syntax.

```
PATTERN <identifier>
  ... // Any statements
END
```

Identifier must satisfy the following rules.

- Start with number, alphabetical letter or underscore (\_)
- Second and remaining characters must be number, alphabetical letter, underscore (\_) or hyphen
- 1 or more characters
- Does not conflicts to keywords for other statement

Defined pattern is expanded by [EXPAND](#EXPAND) statement.

Pattern definition can be nested.
Specifying identifier of expansion is different if the EXPAND statement is inside of parent pattern or not.

```
PATTERN parent
  PATTERN child-A
    ...
  END

  PATTERN child-B
    ...
  END

  EXPAND child-A // The pattern that is defind in inside of same pattern is expanded without any modifier
END

EXPAND parent:child-B // In case of exapnding nested pattern child from outside of parrent pattern,
                      // specify identifier with colon(:) like `<parrent identifier>:<child identifier>`

```

EXPAND statement can precede pattern definition.

```
EXPAND Intro

PATTERN Intro
  ...
END
```

### EXPAND
Expands the defined pattern.
Nested pattern can be expanded with colon(:).
See [PATTERN](#PATTERN) for more details.
