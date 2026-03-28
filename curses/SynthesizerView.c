#include "SynthesizerView.h"
#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "KeyHandler.h"
#include "Attribute.h"
#include "Debug.h"
#include "PresetHelper.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct _Key {
    int code;
    int x;
    int y;
    char label;
    int baseNote;
    int sentNoteNo;
    int64_t sentAt;
    struct _Key *next;
} Key;

struct _SynthesizerView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    Mixer *mixer;
    MixerChannel *mixerChannel;
    int channel;
    int octave;
    int scroll;
    Key keys[32];
    Key *activeKeyList;
    int64_t currentMsec;
    Controller *controller;
};

static ViewNode *SynthesizerViewGetNode(View *self);
static void SynthesizerViewDraw(View *self, Size size);
static void SynthesizerViewDestroy(View *self);

static const ViewVtbl SynthesizerViewViewVtbl = {
    .getNode = SynthesizerViewGetNode,
    .draw = SynthesizerViewDraw,
    .destroy = SynthesizerViewDestroy,
};

static bool SynthesizerViewOnKeyEvent(KeyHandler *self, int code);
static void SynthesizerViewSetNextKeyHandler(KeyHandler *self, KeyHandler *next);
static KeyHandler *SynthesizerViewGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl SynthesizerViewKeyHandlerVtbl = {
    .onKeyEvent = SynthesizerViewOnKeyEvent,
    .setNextKeyHandler = SynthesizerViewSetNextKeyHandler,
    .getNextKeyHandler = SynthesizerViewGetNextKeyHandler,
};

static void *SynthesizerViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&SynthesizerViewViewVtbl;
    if (iid == IIDKeyHandler)
        return (void*)&SynthesizerViewKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl SynthesizerViewInterfaceVtbl = {
    .queryInterface = SynthesizerViewQueryInteface,
};

static void SynthesizerViewTimerCallback(void *receiver, int64_t msec);
static void SynthesizerViewInitializeKeyPad(SynthesizerView *self);

static MixerObserverCallbacks SynthesizerViewMixerObserverCallbacks;

static int lastOctave = 2;

SynthesizerView *SynthesizerViewCreate(Mixer *mixer, int channel)
{
    SynthesizerView *self = calloc(1, sizeof(SynthesizerView));
    self->vtbl = &SynthesizerViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    self->mixer = mixer;
    self->mixerChannel = NAArrayGetValueAt(MixerGetChannels(mixer), channel - 1);
    self->channel = channel;

    SynthesizerViewInitializeKeyPad(self);
    self->octave = lastOctave;

    PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);
    int index = PresetHelperFindPresetInfoIndex(self->mixerChannel, preset);
    self->scroll = MAX(0, index + 1 - 7);

    MixerAddObserver(self->mixer, self, &SynthesizerViewMixerObserverCallbacks);
    return self;
}

void SynthesizerViewSetController(SynthesizerView *self, Controller *controller)
{
    self->controller = controller;
    ControllerRegisterTimer(self->controller, self, SynthesizerViewTimerCallback);
}

static void SynthesizerViewInitializeKeyPad(SynthesizerView *self)
{
    int i = 0;

    self->keys[i++] = (Key){'z', 3, 7, 'Z', -3, -1, 0, NULL};
    self->keys[i++] = (Key){'s', 5, 6, 'S', -2, -1, 0, NULL};
    self->keys[i++] = (Key){'x', 7, 7, 'X', -1, -1, 0, NULL};
    self->keys[i++] = (Key){'c', 11, 7, 'C', 0, -1, 0, NULL};
    self->keys[i++] = (Key){'f', 13, 6, 'F', 1, -1, 0, NULL};
    self->keys[i++] = (Key){'v', 15, 7, 'V', 2, -1, 0, NULL};
    self->keys[i++] = (Key){'g', 17, 6, 'G', 3, -1, 0, NULL};
    self->keys[i++] = (Key){'b', 19, 7, 'B', 4, -1, 0, NULL};
    self->keys[i++] = (Key){'n', 23, 7, 'N', 5, -1, 0, NULL};
    self->keys[i++] = (Key){'j', 25, 6, 'J', 6, -1, 0, NULL};
    self->keys[i++] = (Key){'m', 27, 7, 'M', 7, -1, 0, NULL};
    self->keys[i++] = (Key){'k', 29, 6, 'K', 8, -1, 0, NULL};
    self->keys[i++] = (Key){',', 31, 7, ',', 9, -1, 0, NULL};
    self->keys[i++] = (Key){'l', 33, 6, 'L', 10, -1, 0, NULL};
    self->keys[i++] = (Key){'.', 35, 7, '.', 11, -1, 0, NULL};
    self->keys[i++] = (Key){'/', 39, 7, '/', 12, -1, 0, NULL};

    self->keys[i++] = (Key){'q', 0, 5, 'Q', 9, -1, 0, NULL};
    self->keys[i++] = (Key){'2', 2, 4, '2', 10, -1, 0, NULL};
    self->keys[i++] = (Key){'w', 4, 5, 'W', 11, -1, 0, NULL};
    self->keys[i++] = (Key){'e', 8, 5, 'E', 12, -1, 0, NULL};
    self->keys[i++] = (Key){'4', 10, 4, '4', 13, -1, 0, NULL};
    self->keys[i++] = (Key){'r', 12, 5, 'R', 14, -1, 0, NULL};
    self->keys[i++] = (Key){'5', 14, 4, '5', 15, -1, 0, NULL};
    self->keys[i++] = (Key){'t', 16, 5, 'T', 16, -1, 0, NULL};
    self->keys[i++] = (Key){'y', 20, 5, 'Y', 17, -1, 0, NULL};
    self->keys[i++] = (Key){'7', 22, 4, '7', 18, -1, 0, NULL};
    self->keys[i++] = (Key){'u', 24, 5, 'U', 19, -1, 0, NULL};
    self->keys[i++] = (Key){'8', 26, 4, '8', 20, -1, 0, NULL};
    self->keys[i++] = (Key){'i', 28, 5, 'I', 21, -1, 0, NULL};
    self->keys[i++] = (Key){'9', 30, 4, '9', 22, -1, 0, NULL};
    self->keys[i++] = (Key){'o', 32, 5, 'O', 23, -1, 0, NULL};
    self->keys[i++] = (Key){'p', 36, 5, 'P', 24, -1, 0, NULL};
}

static ViewNode *SynthesizerViewGetNode(View *_self)
{
    SynthesizerView *self = _self;
    return self->node;
}

static void SynthesizerViewDraw(View *_self, Size size)
{
    SynthesizerView *self = _self;

    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(self->mixerChannel);
    PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 0, "Channel: %-2d", self->channel);
    ViewPrintf(self, 0, 1, "Synthesizer Name: %s", description->name);
    ViewPrintf(self, 0, 2, "Bank No: %-3d Program No: %-3d", preset->bankNo, preset->programNo);

    ViewPrintf(self, 43, 2, "Preset Name:");
    ViewPrintf(self, 43, 3, "+---------------------");
    for (int i = 4; i <= 11; ++i) {
        ViewPrintf(self, 43, i, "|");
    }

    ViewPrintf(self, 12, 8, "*");
    ViewPrintf(self, 0, 9, "< >: Octave Shift: *C");
    ViewPrintf(self, 21, 9, self->octave == 0 ? "%-3d" : "%+-2d", self->octave);

    ViewPrintf(self, 0, 10, "ESC: Close Modal");

    PresetInfo **presetInfos = MixerChannelGetPresetInfos(self->mixerChannel);
    int countPreset = MixerChannelGetPresetCount(self->mixerChannel);
    int countTo = MIN(countPreset, self->scroll + 7);
    for (int i = self->scroll; i < countTo; ++i) {
        PresetInfo *_preset = presetInfos[i];
        if (_preset == preset) {
            ViewSetAttr(self, Attribute(ColorDefault, true, true));
        }
        else {
            ViewSetAttr(self, AttributeDefault);
        }

        char name[21];
        snprintf(name, 20, "%s", _preset->name);
        ViewPrintf(self, 45, i - self->scroll + 4, name);
    }

    for (int i = 0; i < 32; ++i) {
        Key *key = &self->keys[i];
        ViewSetAttr(self, key->sentNoteNo != -1 ? Attribute(ColorDefault, true, true) : AttributeDefault);
        ViewPrintf(self, key->x, key->y, "[%c]", key->label);
    }
}

static bool SynthesizerViewOnKeyEvent(KeyHandler *_self, int code)
{
    SynthesizerView *self = _self;

    switch (code) {
    case KEY_DOWN:
        {
            PresetInfo *preset = PresetHelperGetNextPresetInfo(self->mixerChannel);
            MixerChannelSetPresetInfo(self->mixerChannel, preset);
            int index = PresetHelperFindPresetInfoIndex(self->mixerChannel, preset);
            if (self->scroll + 7 + 1 < index) {
                ++self->scroll;
                ViewInvalidate(self);
            }
        }
        return true;
    case KEY_UP:
        {
            PresetInfo *preset = PresetHelperGetPreviousPresetInfo(self->mixerChannel);
            MixerChannelSetPresetInfo(self->mixerChannel, preset);
            int index = PresetHelperFindPresetInfoIndex(self->mixerChannel, preset);
            if (self->scroll - 7 - 1 > index) {
                --self->scroll;
                ViewInvalidate(self);
            }
        }
        return true;
    case '<':
        if (-2 < self->octave) {
            --self->octave;
            ViewInvalidate(self);
        }
        return true;
    case '>':
        if (8 > self->octave) {
            ++self->octave;
            ViewInvalidate(self);
        }
        return true;
    }

    for (int i = 0; i < sizeof(self->keys)/sizeof(self->keys[0]); ++i) {
        if (self->keys[i].code == code) {
            int noteNo = self->keys[i].baseNote + (self->octave + 2) * 12;
            if (0 <= noteNo && noteNo <= 127) {
                bool active = -1 != self->keys[i].sentNoteNo;

                if (active) {
                    NoteEvent event = { .channel = self->channel, .noteNo = self->keys[i].sentNoteNo };
                    MixerSendNoteOff(self->mixer, &event);
                }

                NoteEvent event = { .channel = self->channel, .noteNo = noteNo, .velocity = 100 };
                MixerSendNoteOn(self->mixer, &event);
                self->keys[i].sentNoteNo = noteNo;
                self->keys[i].sentAt = self->currentMsec;

                if (!active) {
                    self->keys[i].next = self->activeKeyList;
                    self->activeKeyList = &self->keys[i];
                }

                ViewInvalidate(self);
            }
            return true;
        }
    }

    return false;
}

static void SynthesizerViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    SynthesizerView *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *SynthesizerViewGetNextKeyHandler(KeyHandler *_self)
{
    SynthesizerView *self = _self;
    return self->nextKeyHandler;
}

static void SynthesizerViewDestroy(View *_self)
{
    SynthesizerView *self = _self;
    lastOctave = self->octave;
    ControllerUnregisterTimer(self->controller, self);
    MixerRemoveObserver(self->mixer, self);
    ViewNodeDestroy(self->node);
    free(self);
}

static void SynthesizerViewTimerCallback(void *receiver, int64_t msec)
{
    SynthesizerView *self = receiver;
    self->currentMsec = msec;

    bool shoudInvalidate = false;

    Key *prev = NULL;
    for (Key *key = self->activeKeyList; key; key = key->next) {
        if (200 < msec - key->sentAt) {
            NoteEvent event = { .channel = self->channel, .noteNo = key->sentNoteNo };
            MixerSendNoteOff(self->mixer, &event);
            key->sentNoteNo = -1;
            
            if (key == self->activeKeyList) {
                self->activeKeyList = key->next;
            }
            if (prev) {
                prev->next = key->next;
            }

            shoudInvalidate = true;
        }

        prev = key;
    }

    if (shoudInvalidate) {
        ViewInvalidate(self);
    }
}

static void SynthesizerViewMixerOnChannelStatusChange(void *receiver, MixerChannel *channel, MixerChannelStatusKind kind)
{
    SynthesizerView *self = receiver;
    if (self->mixerChannel == channel) {
        if (kind == MixerChannelStatusKindPreset) {
            PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);
            int index = PresetHelperFindPresetInfoIndex(self->mixerChannel, preset);
            self->scroll = MAX(0, index + 1 - 7);
            ViewInvalidate(self);
        }
    }

    for (Key *key = self->activeKeyList; key; key = key->next) {
        key->sentNoteNo = -1;
    }

    self->activeKeyList = NULL;
    MixerSendAllNoteOff(self->mixer);
}

static void SynthesizerViewMixerOnAvailableMidiSourceChange(void *receiver, NAArray *descriptions)
{
}

static void SynthesizerViewMixerOnLevelUpdate(void *receiver)
{
}

static MixerObserverCallbacks SynthesizerViewMixerObserverCallbacks = {
    SynthesizerViewMixerOnChannelStatusChange,
    SynthesizerViewMixerOnAvailableMidiSourceChange,
    SynthesizerViewMixerOnLevelUpdate,
};
