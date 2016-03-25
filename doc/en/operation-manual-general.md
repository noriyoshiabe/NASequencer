Operation Manual General
========================

File
----
### Create New File
0. Open File Creation Dialog  
    Choose \[File\] -> [New] from menu.

0. Select File Type  
    Select file type in File Creation Dialog.  
    Following file types are available

    - Note as Sequence (.nas)
    - ABC Notation (.abc)
    - Music Macro Language (.mml)

    ![](../shared/file-creation-dialog.png)

0. Enter File Name and Click \[Create\] button

**Short Cut Key**  
⌘ + N

### Open Existing File
Choose \[File\] -> \[Open...\] from menu.

**Short Cut Key**  
⌘ + O

### Open In Editor
Choose \[File\] -> \[Open In Editor\] from menu.

**Note:**  
This menu is only available when document window is opened.
Additionally, behavior is different when editor window is selected.
See below for the detail.

#### In Document Window
Open source file in editor window.
If external editor is selected in [Preferences](TODO), external editor will be launched and open source file.

#### In Embedded Editor
File Open Dialog will be shown.
After selection, open the selected file in editor window.

**Short Cut Key**  
⌘ + E

### Save Document
Choose \[File\] -> [Save...] or [Save As] from menu.
These menus are only available in editor window.

**Short Cut Key**  

- ⌘ + S
- ⌘ + Shift + S (Save with different file name)

### Export
_**Full Version Feature**_

0. Choose \[File\] -> \[Export...\] from menu or click toolbar button in document window.

    ![Export Button](../shared/export@2x.png)

0. Select file type in expoting sheet.
    Following file types are available

    - Standard MIDI File (Format 1)
    - Waveform Audio File Format (WAV 44.1kHz Linear PCM 16bit stereo)
    - Advanced Audio Coding (AAC 44.1kHz 192kbps stereo)

    ![](../shared/export-sheet.png)

0. Enter File Name and Click \[Export\] button

**Short Cut Key**  
⌘ + Shift + E

Playback Control
----------------
### Play/Pause
Play sequence in current document window.

Choose \[Control\] -> \[Play/Pause\] from menu or click toolbar button in document window.

![Play Button](../shared/play@2x.png)

![Pause Button](../shared/pause@2x.png)

**Short Cut Key**  
⌘ + P

### Rewind
Rewind playing position to beginning of sequence.

Choose \[Control\] -> \[Rewind\] from menu or click toolbar button in document window.

![Rewind Button](../shared/rewind@2x.png)

**Short Cut Key**  
⌘ + R

### Forward
Forward playing position to beginning of next measure.

Choose \[Control\] -> \[Forward\] from menu or click toolbar button in document window.

![Forward Button](../shared/forward@2x.png)

**Short Cut Key**  
⌘ + F

### Backward
Backward playing position to beginning of previous measure.

Choose \[Control\] -> \[Backward\] from menu or click toolbar button in document window.

![Backward Button](../shared/backward@2x.png)

**Short Cut Key**  
⌘ + B

### Toggle Repeat
Change repeat state in a cycle.
Repeat Off -> Repeat All -> Repeat Marker -> Repeat Off ...

Choose \[Control\] -> \[Toggle Repeat\] from menu or click toolbar button in document window.

![Repeat Button](../shared/repeat_off@2x.png)

**Short Cut Key**  
⌘ + Shift + R

#### Repeat State
##### Repeat Off
![](../shared/repeat_off@2x.png)

Stop playback when playing position reach to end of sequence.

##### Repeat All
![](../shared/repeat_all@2x.png)

Play again from beginning of sequence when playing position reach to end of sequence.

##### Repeat Marker

![](../shared/repeat_marker@2x.png)

Play again from beginning of section when playing position reach to end of section.
Section is divided by [Marker Event](TODO).
For example, if sequence events are
```
001:01:000 C2 Gatetime=1920
002:01:000 Marker A
002:01:000 D2 Gatetime=1920
003:01:000 Marker B
003:01:000 E2 Gatetime=1920
004:01:000 F2 Gatetime=1920
```
sections will be listed below.
```
   | 1               2               3               4               5
========================================================================
   | S               A               B               .               E
------------------------------------------------------------------------
   | .               .               .               x---------------.
   | .               .               x---------------.               .
   | .               .               .               .               .
   | .               x---------------.               .               .
   | .               .               .               .               .
C2 | x---------------.               .               .               .

     |<-- section -->|<-- section -->|<---------- section ---------->|

S = beginning of sequence
E = end of sequence
```


Views
-----
### Main View
![](../shared/main-view.png)

#### Conductor Panel
#### Mixer Panel
![](../shared/mixer-panel.png)
#### Measure
#### Conductor Track
#### Channel Track
#### Location

### Detail View
![](../shared/detail-view-1.png)

#### Back Button
#### Track Selection
#### Piano Roll
#### Measure
#### Keyboard
#### Velocity
#### Event List
![](../shared/detail-view-2.png)


### Error Window
![](../shared/error-window.png)
