#define BD C1
#define SD E1
#define HC F#1
#define HP G#1
#define HO A#1
#define CS C#2

// #include 'drums.nas'

TITLE 'Syntax of Note as Sequence'
========================
COPYRIGHT  'Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.'
RESOLUTION 480
TEMPO      120.0
TIME       4/4

MARKER 'Start'
------------------------

CHANNEL   10
------------------------
PERCUSSION ON
SYNTH      'GeneralUser GS Live/Audigy version 1.44'
BANK       128
PROGRAM    1
CHORUS     20
REVERB     20
VOLUME     100
VELOCITY   127
EXPRESSION 127
TRANSPOSE  0
PAN        0
GATETIME   64

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
    120: BD      HC
    120:     SD         
    120: BD      HC 
    120:          
    120:     SD  HC
    120:          
    120:         HO
    120:
    ------------------------
  END

  PATTERN last
    ------------------------
    120: C1      C#2 V=127
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
SYNTH     'GeneralUser GS Live/Audigy version 1.44'
BANK      0
PROGRAM   1
CHORUS    20
REVERB    20
DETUNE    +10
GATETIME  STEP - 10

PATTERN piano-chord
  VELOCITY  100
    
    0: KEY Cmaj
  
  480: C2 E2 G2 B2

  120: C2 E2 G2 B2
  120: C2 E2 G2 B2
  240: 
  240: C2 E2 G2 B2
  240: C2 E2 G2 B2
  240: C2 E2 G2 B2

  VELOCITY  60
  
    0: KEY Cmin
    
  480: C2 GT=960 E2 G2 B2
                  
  120:    E2 G2 B2 V=100
  120:    E2 G2 B2 V=100
  240: 
  240: C2 E2 G2 B2 V=127
  240:    E2 G2 B2 V=127 C3 V=127
  240: C2 E2 G2 B2 V=127
  240: C2 E2 G2 B2 V=127
  240: C2 E2 G2 B2 V=127
END

EXPAND piano-chord
EXPAND piano-chord
