//
//  GettingStartedWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "GettingStartedWindowController.h"
#import "ApplicationController.h"
#import "Color.h"
#import "AudioOut.h"
#import "MidiSourceManager.h"

@import WebKit;

@interface GettingStartedWindowController () <WebPolicyDelegate, NSWindowDelegate> {
    BOOL _needOpenExample;
    AudioOut *_audioOut;
@public
    MidiSource *_midiSource;
}
@property (weak) IBOutlet WebView *webView;
@end

static void __AudioCallback(void *receiver, AudioSample *buffer, uint32_t count)
{
    GettingStartedWindowController *controller = (__bridge GettingStartedWindowController *)receiver;
    MidiSource *midiSource = controller->_midiSource;
    double dummy[count];
    midiSource->computeAudioSample(midiSource, buffer, dummy, dummy, count);
}

@implementation GettingStartedWindowController

- (NSString *)windowNibName
{
    return @"GettingStartedWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    _audioOut = AudioOutCreate();
    
    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);
    NAIterator *iterator = NAArrayGetIterator(descriptions);
    while (iterator->hasNext(iterator)) {
        MidiSourceDescription *description = iterator->next(iterator);
        if (0 == strcmp(description->name, "GeneralUser GS Live/Audigy version 1.44")) {
            _midiSource = MidiSourceManagerAllocMidiSource(manager, description, _audioOut->getSampleRate(_audioOut));
            break;
        }
    }
    
    _audioOut->registerCallback(_audioOut, __AudioCallback, (__bridge void *)self);
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"GettingStarted"];
    NSURL* fileURL = [NSURL fileURLWithPath:filePath];
    NSURLRequest* request = [NSURLRequest requestWithURL:fileURL];
    [[_webView mainFrame] loadRequest:request];
    
    [_webView setDrawsBackground:NO];
    _webView.policyDelegate = self;
    
    self.window.backgroundColor = [Color darkGray];
    [self.window toggleFullScreen:self];
}

- (void)close
{
    _audioOut->unregisterCallback(_audioOut, __AudioCallback, (__bridge void *)self);
    [super close];
}

- (void)dealloc
{
    AudioOutDestroy(_audioOut);
    MidiSourceManagerDeallocMidiSource(MidiSourceManagerSharedInstance(), _midiSource);
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    if ([request.URL.scheme isEqualToString:@"nasequencer"]) {
        if ([request.URL.host isEqualToString:@"note_on"]) {
            int noteNo = [request.URL.lastPathComponent intValue];
            uint8_t bytes[3] = {0x91, noteNo, 100};
            _midiSource->send(_midiSource, bytes, 3);
        }
        else if ([request.URL.host isEqualToString:@"note_off"]) {
            int noteNo = [request.URL.lastPathComponent intValue];
            uint8_t bytes[3] = {0x81, noteNo, 0};
            _midiSource->send(_midiSource, bytes, 3);
        }
        else if ([request.URL.host isEqualToString:@"all_note_off"]) {
            uint8_t bytes[3] = {0xB1, 0x7B, 0x00};
            _midiSource->send(_midiSource, bytes, 3);
            bytes[1] = 0x78;
            _midiSource->send(_midiSource, bytes, 3);
        }
        else if ([request.URL.host isEqualToString:@"exit"]) {
            [self close];
        }
        else if ([request.URL.host isEqualToString:@"example"]) {
            if (self.window.isFullScreen) {
                _needOpenExample = YES;
                [self.window toggleFullScreen:self];
            }
            else {
                [AppController openExampleDocument:@"nas"];
            }
        }
        else if ([request.URL.host isEqualToString:@"syntax"]) {
            if (self.window.isFullScreen) {
                [self.window toggleFullScreen:self];
            }
            NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
            [[NSHelpManager sharedHelpManager] openHelpAnchor:@"syntax_reference" inBook:bookName];
        }
        else if ([request.URL.host isEqualToString:@"operation"]) {
            if (self.window.isFullScreen) {
                [self.window toggleFullScreen:self];
            }
            NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
            [[NSHelpManager sharedHelpManager] openHelpAnchor:@"operation_manual" inBook:bookName];
        }
    }
}

#pragma mark NSWindowDelegate

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    if (_needOpenExample) {
        [AppController openExampleDocument:@"nas"];
        _needOpenExample = NO;
    }
}

@end
