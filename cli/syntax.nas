TITLE 'Syntax of NAMIDI'
========================
RESOLUTION 480
TEMPO      120.0
TIME       4/4

CHANNEL   10
--------------------------
SYNTH     'GeneralUser GS Live-Audigy v1.44'
VOICE     0 120 0
CHORUS    100
REVERB    100
VOLUME    100
TRANSPOSE 0
PAN       0
------------------------
MARKER 'Start'
------------------------

DEFINE rhythm-pattern
------------------------
CONTEXT DEFAULT
------------------------
  120: C1      C#2 
  120:          
  120: C1      F#1 
  120:          
  120:     E1  F#1 
  120:          
  120: C1      F#1 
  120:     E1         
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
------------------------
CONTEXT LAST
------------------------
  120: C1      C#2 127
  120:          
  120: C1      F#1 
  120:          
  120:     E1  F#1 
  120:          
  120: C1      F#1 
  120:     E1      
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
------------------------
END

EXPAND rhythm-pattern
EXPAND rhythm-pattern
EXPAND rhythm-pattern
EXPAND rhythm-pattern WITH LAST


CHANNEL   1
------------------------
SYNTH     'GeneralUser GS Live-Audigy v1.44'
VOICE     0 0 0
CHORUS    100
REVERB    100
------------------------

DEFINE piano-chord
------------------------
KEY Cmaj

480: C2 E2 G2 B2

120: C2 E2 G2 B2
120: C2 E2 G2 B2
240: 
240: C2 E2 G2 B2
240: C2 E2 G2 B2
240: C2 E2 G2 B2

KEY Cmin

480: C2 - 960 E2 G2 B2
                
120:    E2 G2 B2
120:    E2 G2 B2
240: 
240: C2 E2 G2 B2
240:    E2 G2 B2 C3 127
240: C2 E2 G2 B2
240: C2 E2 G2 B2
240: C2 E2 G2 B2
------------------------
END

EXPAND piano-chord
EXPAND piano-chord
