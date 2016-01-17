TITLE 'Syntax of NAMIDI'
========================
COPYRIGHT  'Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.'
RESOLUTION 480
TEMPO      120.0
TIME       4/4

MARKER 'Start'
------------------------

CHANNEL   10
------------------------
SYNTH      'GeneralUser GS Live-Audigy v1.44'
VOICE      0 120 0
CHORUS     100
REVERB     100
VOLUME     100
VELOCITY   127
EXPRESSION 127
TRANSPOSE  0
PAN        0

PATTERN rhythm---pattern
  PATTERN default
    ------------------------
    120: C1      C#2 
    120:          
    120: C1      F#1 
    120:          
    120:     E1  F#1 
    120:          
    120: C1      F#1 
    120:     E1         
    ------------------------
    120: C1      F#1 
    120:     E1         
    120: C1      F#1 
    120:          
    120:     E1  F#1 
    120:          
    120:         A#1
    120:
    ------------------------
  END

  PATTERN last
    ------------------------
    120: C1      C#2 127
    120:          
    120: C1      F#1 
    120:          
    120:     E1  F#1 
    120:          
    120: C1      F#1 
    120:     E1      
    ------------------------
    120: C1  E1  F#1 
    120:     E1     
    120: C1  E1  F#1 
    120:     E1 
     20:     E1  F#1
    100: E1 
     60:     E1 
     60:     E1 
    120:     E1  A#1
    120: C1      G#1
    ------------------------
  END

  EXPAND default
END

EXPAND rhythm---pattern
EXPAND rhythm---pattern
EXPAND rhythm---pattern
EXPAND rhythm---pattern:last


CHANNEL   1
------------------------
SYNTH     'GeneralUser GS Live-Audigy v1.44'
VOICE     0 0 0
CHORUS    100
REVERB    100
DETUNE    +10

PATTERN piano-chord
  KEY Cmaj
  VELOCITY  100

  480: C2 E2 G2 B2

  120: C2 E2 G2 B2
  120: C2 E2 G2 B2
  240: 
  240: C2 E2 G2 B2
  240: C2 E2 G2 B2
  240: C2 E2 G2 B2

  KEY Cmin
  VELOCITY  60

  480: C2 - 960 E2 G2 B2
                  
  120:    E2 G2 B2 100
  120:    E2 G2 B2 100
  240: 
  240: C2 E2 G2 B2 127
  240:    E2 G2 B2 127 C3 127
  240: C2 E2 G2 B2 127
  240: C2 E2 G2 B2 127
  240: C2 E2 G2 B2 127
END

EXPAND piano-chord
EXPAND piano-chord
