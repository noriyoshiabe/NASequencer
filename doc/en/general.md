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

**Shortcut Key**  
⌘ + N

### Open Existing File
Choose \[File\] -> \[Open...\] from menu.

**Shortcut Key**  
⌘ + O

### Open In Editor
Choose \[File\] -> \[Open In Editor\] from menu.

**Note:**  
This menu is only available when document window is opened.
Additionally, behavior is different when editor window is selected.
See below for the detail.

#### In Document Window
Open source file in editor window.
If external editor is selected in [Preferences - External Editor](preference.md#External Editor), external editor will be launched and open source file.

#### In Embedded Editor
File Open Dialog will be shown.
After selection, open the selected file in editor window.

**Shortcut Key**  
⌘ + E

### Save Document
Choose \[File\] -> [Save...] or [Save As] from menu.
These menus are only available in editor window.

**Shortcut Key**  

- ⌘ + S
- ⌘ + Shift + S (Save with different file name)

### Export
_**Full Version Feature**_

0. Choose \[File\] -> \[Export...\] from menu or click toolbar button in document window.

![](../shared/export@2x.png) _Export Button_

0. Select file type in exporting sheet.
    Following file types are available

    - Standard MIDI File (Format 1)
    - Waveform Audio File Format (WAV 44.1kHz Linear PCM 16bit stereo)
    - Advanced Audio Coding (AAC 44.1kHz 192kbps stereo)

    ![](../shared/export-sheet.png)

0. Enter File Name and Click \[Export\] button

**Shortcut Key**  
⌘ + Shift + E

Playback Control
----------------
### Play/Pause
Play sequence in current document window.

Choose \[Control\] -> \[Play/Pause\] from menu or click toolbar button in document window.

![](../shared/play@2x.png) _Play Button_

![](../shared/pause@2x.png) _Pause Button_

**Shortcut Key**  
⌘ + P

### Rewind
Rewind playing location to beginning of sequence.

Choose \[Control\] -> \[Rewind\] from menu or click toolbar button in document window.

![](../shared/rewind@2x.png) _Rewind Button_

**Shortcut Key**  
⌘ + R

### Forward
Forward playing location to beginning of next measure.

Choose \[Control\] -> \[Forward\] from menu or click toolbar button in document window.

![](../shared/forward@2x.png) _Forward Button_

**Shortcut Key**  
⌘ + →

### Backward
Backward playing location to beginning of previous measure.

Choose \[Control\] -> \[Backward\] from menu or click toolbar button in document window.

![](../shared/backward@2x.png) _Backward Button_

**Shortcut Key**  
⌘ + ←

### Forward to Marker
Forward playing location to next marker notated by [Marker Event](nas.md#MARKER).

Choose \[Control\] -> \[Forward to Marker\] from menu or click toolbar button with shift key in document window.

![](../shared/forward_marker@2x.png) _Forward to Marker Button (With Shift Key Pressed)_

**Shortcut Key**  
Shift + ⌘ + →

### Backward to Marker
Backward playing location to next marker notated by [Marker Event](nas.md#MARKER).

Choose \[Control\] -> \[Backward to Marker\] from menu or click toolbar button with shift key in document window.

![](../shared/backward_marker@2x.png) _Backward to Marker Button (With Shift Key Pressed)_

**Shortcut Key**  
Shift + ⌘ + ←

### Toggle Repeat
Change repeat state in a cycle.
Repeat Off -> Repeat All -> Repeat Marker -> Repeat Off ...

Choose \[Control\] -> \[Toggle Repeat\] from menu or click toolbar button in document window.

![](../shared/repeat_off@2x.png) _Repeat Button_

**Shortcut Key**  
⌘ + Shift + R

#### Repeat State
##### Repeat Off
![](../shared/repeat_off@2x.png)

Stop playback when playing location reach to end of sequence.

##### Repeat All
![](../shared/repeat_all@2x.png)

Play again from beginning of sequence when playing location reach to end of sequence.

##### Repeat Marker
![](../shared/repeat_marker@2x.png)

Play again from beginning of section when playing location reach to end of section.
Section is divided by [Marker Event](nas.md#MARKER).
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

### Toggle Auto Scroll
Toggle enable/disable auto scroll sequence view to playing position.

![](../shared/auto_scroll_off@2x.png) _Auto Scroll Button (Disable)_
![](../shared/auto_scroll_on@2x.png) _Auto Scroll Button (Enable)_

**Shortcut Key**  
Control + S


Main View
---------
![](../shared/main-view.png)

### Conductor Panel
#### Tempo
Display tempo value on current location.

#### Time Signature
Display tempo value on current location.

### Mixer Panel
![](../shared/mixer-panel.png)

#### Channel
Display channel number.

#### Mute
Button to toggle mute on/off.

#### Solo
Button to toggle solo on/off.

#### Synth
Pull-down menu to select synthesizer.

_If there is control event in sequence, player will change synthesizer and pull-down selection will be updated._

#### Preset
Button to show [Preset Selection](#Preset Selection) sheet.

_If there is control event in sequence, player will send the event to synthesizer and preset selection will be updated._

#### Volume / Pan / Chorus / Reverb
Sliders for each synthesis value.

_If there is control event in sequence, player will send the event to synthesizer and slider position will be updated._

#### Level Indicator
Display output level for each channel.

### Preset Selection
![](../shared/preset-selection.png)

Shown by preset button on [Mixer Panel](#Mixer Panel).

**Note:**  
Text field for Synthesizer Name is selectable.
You can copy it and paste to source file for [SYNTH](nas.md#SYNTH) statement.

#### Preset List
List of available presets of synthesizer. By click list or ←→ key up/down, selected preset will be changed.

#### Keyboard Preview
In Preset Selection sheet, previewing sound of preset is available.
Mapping of hardware keyboard is shown on the sheet.
←→ keys are for octave shift.

<a name="general__main_measure"></a>
### Measure
- Display guide for measure and beat
- Seek current location to clicked position with click

<a name="general__main_conductor_track"></a>
### Conductor Track
- Display events below.
    - [Tempo Change](nas.md#TEMPO)
    - [Time Signature](nas.md#TIME)
    - [Marker](nas.md#MARKER)
- Select track with click
- Show detail view with click after selection

### Channel Track
- Display [Note](nas.md#NOTE) events
- Select track with click
- Show detail view with click after selection

**Tips**  
You can select multiple track with ⌘ + click or Shift + click  
Additionally, following shortcut key is available.

- ⌘ + A for select all tracks (including conductor track)
- ESC for deselect all tracks

### Location
Display current location. location format is listed below from left side.

- &lt;measure number&gt;:&lt;beat&gt;:&lt;tick&gt;
- &lt;minutes&gt;:&lt;seconds&gt;:&lt;milliseconds&gt;


Detail View
-----------
![](../shared/detail-view-1.png)

### Back Button
Button for back to main view.

### Track Selection
Button to toggle display for events in track.

#### Selection State
##### Display
![](../shared/track-selection-display.png)
##### No Display
![](../shared/track-selection-no-display.png)
##### Disable (There is no event in track)
![](../shared/track-selection-disable.png)

<a name="general__detail_measure"></a>
### Measure
Behavior is same as main-view.

- Display guide for measure and beat
- Seek current location to clicked position with click

<a name="general__detail_conductor_track"></a>
### Conductor Track
Display events below.

- [Tempo Change](nas.md#TEMPO)
- [Time Signature](nas.md#TIME)
- [Marker](nas.md#MARKER)

### Piano Roll
Display [Note](nas.md#NOTE) events in selected channel tracks.

### Keyboard
- Guide for scale
- Preview sound on selected channel track with click

### Velocity
Display velocity of [Note](nas.md#NOTE) events in selected channel track.

### Event List Switch
Button to toggle display event list.

### Event List
Display events in selected tracks

![](../shared/detail-view-2.png)

#### Filter Switch
##### Note
Button to toggle display note events.

##### Control
Button to toggle display events except note event.


Error Window
------------
If there is syntax or grammar error in source file, error window shows up.
Error window can be manually hidden by [x] button. Also, it is automatically hidden after error is corrected and source file is saved.

![](../shared/error-window.png)
