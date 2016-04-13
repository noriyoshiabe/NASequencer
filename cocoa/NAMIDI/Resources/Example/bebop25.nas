TITLE 'Bebop 25'
COPYRIGHT 'By Audionautix (http://audionautix.com/) Licensed under Creative Commons: By Attribution 3.0 License http://creativecommons.org/licenses/by/3.0/'

// Conductror
========================================
    0: TIME 4/4
       TEMPO 104
40320: MARKER "A"
19200: MARKER "B"
40320: MARKER "A'"
 7680: MARKER "B'"
  960: TEMPO 102
  960: TEMPO  98
 3840: TEMPO  94


// Piano 1
========================================
CHANNEL 2
BANK 0
PROGRAM 2

VELOCITY 90
VOLUME 70
CHORUS 10
REVERB 5

EXPAND piano-left:section-a-1
EXPAND piano-left:section-b-1
EXPAND piano-left:section-a-2
EXPAND piano-left:section-b-2


// Piano 2
========================================
CHANNEL 3
BANK 0
PROGRAM 2

VELOCITY 90
VOLUME 70
CHORUS 10
REVERB 5

EXPAND piano-right:section-a-1
EXPAND piano-right:section-b-1
EXPAND piano-right:section-a-2
EXPAND piano-right:section-b-2


// Bass
========================================
CHANNEL 12
BANK 0
PROGRAM 33
VOLUME 127
CHORUS 10
REVERB 10

EXPAND bass:section-a-1
EXPAND bass:section-b-1
EXPAND bass:section-a-2
EXPAND bass:section-b-2


// Drums
========================================
CHANNEL 10
BANK 120
PROGRAM 1
VOLUME 127
CHORUS 10
REVERB 10

EXPAND drums:section-a-1
EXPAND drums:section-b-1
EXPAND drums:section-a-2
EXPAND drums:section-b-2


// Pattern - Piano 1
========================================
PATTERN piano-left
  
  PATTERN section-a-1
     10: G3 GT=960
     10: A3 GT=960
    940: C4 GT=960
    960: B3
    --------------
    960: F3 A3 C4
    480: E4
    480: D4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    1920: G3 A3 C4
    --------------
    1920: G3 C4 D4
    --------------
    1920: F3 A3 C4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    1920: G3 A3 C4
    ==============
    960: C#3 G3 A#3
    480: C#3 A3
    480: G3
    --------------
    960: D3 E3 G3
    960: C3 F3 G#3
    --------------
    960: B2 D#3 G#3
    480: B2 D#3 G3
    480: C3 D#3 G#3
    --------------
    480: A#2 D3 F3
    480: B2
    960: C3 D#3
    --------------
    960: C#3 F3 A#3
    480: B2 D3 G#3
    480: F3
    --------------
    960: G3 A3 C4
    480: B3
    480: E4
    --------------
    960: F3 A3 C4
    480: E4
    480: D4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    ==============
    1920: G3 A3 C4
    --------------
    1920: G3 C4 D4
    --------------
    1920: F3 A3 C4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    1920: G3 A3 C4
  END


  PATTERN section-b-1
    960: C#3 G3 A#3
    480: C#3 A3
    480: G3
    --------------
    960: D3 E3 G3
    960: C3 F3 G#3
    --------------
    960: B2 D#3 G#3
    480: B2 D#3 G3
    480: C3 D#3 G#3
    --------------
    480: A#2 D3 F3 
    480: B2
    960: C3 D#3
    --------------
    960: C#3 F3 A#3
    480: B2 D3 G#3
    480: F3
    --------------
    960: C1 G3 A#3
    320: C#1
     80:
    240: G0
    320:
    --------------
    960: C1 A#2
    320: C#1 B2
     80:
    240: G1 F2
    320:
    --------------
    960: C1 A#2
    320: C#1 B2
     80:
    240: G0 F2
    320:
    --------------
    960: C1 D#2
    960: E2
    --------------
     80: E2 
    1840: F2
  END


  PATTERN section-a-2
     60: C3 GT=960
     60: G3 GT=960
     60: A3 GT=960
    780: C4 GT=960
    960: B3
    --------------
    960: F3 A3 C4
    240: E4
    240: D#4
    480: D4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    480: G3 A3 D4
    1440: C4
    --------------
    1920: G3 C4 D4
    --------------
    1920: F3 A3 C4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    1920: G3 A3 C4
    ==============
    960: C#3 G3 A#3
    480: C#3 A3
    480: G3
    --------------
    480: D3 E3 G3
    480: A3
    960: C3 F3 G#3
    --------------
    960: B2 D#3 G#3
    480: B2 D#3 G3
    480: C3 D#3 G#3
    --------------
    480: A#2 D3 F3
    480: B2
    480: C3 D#3
    480: C#3
    --------------
    960: C#3 F3 A#3
    480: B2 D3 G#3
    480: F3
    --------------
    960: G3 A3 C4
    480: B3
    480: E4
    --------------
    960: F3 A3 C4
    480: E4
    480: D4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    ==============
    1920: G3 A3 C4
    --------------
    1920: G3 C4 D4
    --------------
    1920: F3 A3 C4
    --------------
    960: G3 B3
    480: G3 B3 E4
    480: G3 B3
    --------------
    1920: G3 A3 C4
  END


  PATTERN section-b-2
    960: C#3 G3 A#3
    480: C#3 A3
    480: G3
    --------------
    960: D3 E3 G3
    960: C3 F3 G#3
    --------------
    960: B2 D#3 G#3
    480: B2 D#3 G3
    480: C3 D#3 G#3
    --------------
    480: A#2 D3 F3 
    480: B2
    960: C3 D#3
    --------------
    960: C#3 F3 A#3
    480: B2 D3 G#3
    480: F3
    --------------
    3840: G3
  END

END


// Pattern - Piano 2
========================================
PATTERN piano-right

  PATTERN section-a-1
    40320:
  END

  PATTERN section-b-1
    240: C#4
    240: E4
    240: G4
    240: A#4
    240: A4
    240: G4
    240: E4
    240: C#4
    --------
    240: G4
    240: F4
    240: E4
    240: G4
    960: F4
    --------
    240: B3
    240: D4
    240: F4
    240: G#4
    240: G4
    240: F4
    240: D4
    240: B3
    --------
    240: F4
    240: D#4
    240: D4
    240: B3
    960: A#3
    --------
    240: B3
    240: D4
    240: F4
    240: G#4
    240: G4
    240: D4
    240: B3
    240: D4
    --------
    1920: D4
    --------
    7680:
  END

  PATTERN section-a-2
    40320:
  END

  PATTERN section-b-2
    240: C#4
    240: E4
    240: G4
    240: A#4
    240: A4
    240: G4
    240: E4
    240: C#4
    --------
    240: G4
    240: F4
    240: E4
    240: G4
    960: F4
    --------
    240: B3
    240: D4
    240: F4
    240: G#4
    240: G4
    240: F4
    240: D4
    240: B3
    --------
    240: F4
    240: D#4
    240: D4
    240: B3
    960: A#3
    --------
    240: B3
    240: D4
    240: F4
    240: G#4
    240: G4
    240: D4
    240: B3
    240: D4
    --------
    3840: B3
  END
END


// Pattern - Bass
========================================
PATTERN bass

  PATTERN main
    240: C2
    240: B1
    240: A1
    240: G1
    240: E1
    240: C1
    240: G0
    240: C0
    --------
    240: D0
    240: E0
    240: F0
    240: G0
    240: A0
    240: C1
    240: E1
    240: D1
    --------
    240: G0
    240: A0
    240: B0
    240: D1
    240: G#1
    240: G1
    240: F1
    240: D1
    --------
    240: C1
    240: D1
    240: D#1
    240: E1
    240: A1
    240: G1
    240: A1
    240: G1
  END

  PATTERN section-a-1
    EXPAND main
    EXPAND main

    240: A#1
    240: G1
    240: E1
    240: C#1
    240: A#0
    240: A0
    240: G0
    240: E0
    --------
    240: D0
    240: E0
    240: F0
    240: A0
    240: D1
    240: E1
    240: F1
    240: D1
    --------
    240: G0
    240: G#0
    240: B0
    240: D1
    240: F1
    240: G1
    240: B1
    240: D2
    --------
    240: D#2
    240: D2
    240: C2
    240: G#1
    240: G1
    240: F1
    240: D#1
    240: A#0
    --------
    240: G1
    240: G1
    240: F1
    240: F1
    240: D#1
    240: G1
    240: D1
    240: G0
    --------
    240: C2
    240: B1
    240: A1
    240: G1
    240: E1
    240: C1
    240: D1
    240: G0
    --------
    240: D0
    240: A0
    240: D1
    240: A0
    240: D1
    240: E1
    240: F1
    240: A1
    --------
    240: G1
    240: D1
    240: G0
    240: G1
    240: G#1
    240: D1
    240: G0
    240: D1
    ========
    240: C1 GT=1920
    240: D2
    240: D#2
    240: E2
    240: A2
    240: G2
    240: E2
    240: C2

    EXPAND main
  END

  PATTERN section-b-1
    240: A#1
    240: G1
    240: E1
    240: C#1
    240: A#0
    240: A0
    240: G0
    240: E0
    --------
    240: D0
    240: E0
    240: F0
    240: A0
    240: D1
    240: E1
    240: F1
    240: D1
    --------
    240: G0
    240: G#0
    240: B0
    240: D1
    240: F1
    240: G1
    240: B1
    240: D2
    --------
    240: G2
    240: D2
    240: C2
    240: G#1
    240: G1
    240: F1
    240: D#1
    240: A#0
    --------
    240: G1
    240: G1
    240: F1
    240: F1
    240: D#1
    240: G1
    240: D1
    240: G0
    --------
    960: C1
    320: C#1 
     80:
    480: G0
     80:
    --------
    960: C1
    320: C#1
     80:
    480: G1
     80:
    --------
    960: C1
    320: C#1
     80:
    480: G0
     80: B0
    ========
    320: C1
     80:
    240: C0
     80:
    320: C1
     80:
    240: C0
     80:
    320: C1
    160:
    --------
     80: F#1
   1840: G1
  END

  PATTERN section-a-2
    EXPAND section-a-1
  END

  PATTERN section-b-2
    240: A#1
    240: G1
    240: E1
    240: C#1
    240: A#0
    240: A0
    240: G0
    240: E0
    --------
    240: D0
    240: E0
    240: F0
    240: A0
    240: D1
    240: E1
    240: F1
    240: D1
    --------
    240: G0
    240: G#0
    240: B0
    240: D1
    240: F1
    240: G1
    240: B1
    240: D2
    --------
    240: D#2
    240: D2
    240: C2
    240: G#1
    240: G1
    240: F1
    240: D#1
    240: A#0
    --------
    240: G1
    240: G1
    240: F1
    240: F1
    240: D#1
    240: G1
    240: D1
    240: G0
    --------
    960: C1
    960: G0
   1920: C0
  END

END


// Pattern - Drums
========================================

#define BD  C1
#define SD  E1
#define FT1 G1
#define FT2 F1
#define RS1 D#2
#define RS2 F2
#define CS1 C#2
#define CS2 A2
#define HO  A#1
#define HC  F#1

PATTERN drums

  PATTERN basic
    240: RS1
    160: RS1
     80: RS1
    240: RS1
    160: RS1
     80: RS1
    240: RS1
    240: RS1
    160: RS1
     80: RS2
    240: RS1
  END

  PATTERN section-a-1
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
  END

  PATTERN section-b-1
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND fill1
    EXPAND fill1
    EXPAND fill1
    EXPAND fill1
    EXPAND fill2

    PATTERN fill1
      240: HO
      160: HC
       80: HC
      240: HO
      160: HC
       80: HC
      240: HO
      160: HC
       80: HC
      240: HO
      160: HC
       80: HC
    END

    PATTERN fill2
      240: BD       RS1
      240: BD       RS1
      240: SD V=127
      60:           RS1 
      60:
      60:     E1    
      60:     E1
      80: BD
      80:
      80:
      80:     E1
      80:     E1
      80:     E1
      80: FT2 E1 V=127
      80:     E1
      80:     E1
      80:     E1
      80:     E1
      80:     E1
    END
  END

  PATTERN section-a-2
    EXPAND section-a-1
  END

  PATTERN section-b-2
    EXPAND basic
    EXPAND basic
    EXPAND basic
    EXPAND basic

    1440: CS1
    120:  FT1
    120:  FT1
    240:  FT2
    --------
    3840: BD CS2
  END

END
