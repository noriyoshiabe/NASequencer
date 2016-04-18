//
//  MainWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainWindowController.h"
#import "MainViewController.h"
#import "DetailViewController.h"
#import "LocationView.h"
#import "ErrorWindowController.h"
#import "TrackSelection.h"
#import "ApplicationController.h"
#import "EditorWindowController.h"
#import "Preference.h"
#import "PresetSelectionWindowController.h"

@interface MainWindowController () <TrackSelectionDelegate, NAMidiRepresentationObserver, PlayerRepresentationObserver, MainViewControllerDelegate, NSUserInterfaceValidations, NSWindowDelegate> {
    BOOL _isPresetSelectionShown;
}

@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet LocationView *locationView;
@property (strong, nonatomic) MainViewController *mainVC;
@property (strong, nonatomic) DetailViewController *detailVC;
@property (strong, nonatomic) ErrorWindowController *errorWC;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (strong, nonatomic) EditorWindowController *editorWC;
@property (strong, nonatomic) PresetSelectionWindowController *presetSelectionController;
@end

@implementation MainWindowController

- (NSString *)windowNibName
{
    return @"MainWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"MainWindowFrame";
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.masksToBounds = YES;
    
    [_namidi addObserver:self];
    [_namidi.player addObserver:self];
    
    _mainVC = [[MainViewController alloc] init];
    _detailVC = [[DetailViewController alloc] init];
    
    _presetSelectionController = [[PresetSelectionWindowController alloc] init];
    
    _mainVC.delegate = self;
    
    _mainVC.namidi = _namidi;
    _detailVC.namidi = _namidi;
    
    _trackSelection = [[TrackSelection alloc] init];
    _trackSelection.delegate = self;
    _trackSelection.namidi = _namidi;
    
    _mainVC.trackSelection = _trackSelection;
    _detailVC.trackSelection = _trackSelection;
    
    _scaleAssistant = [[MeasureScaleAssistant alloc] init];
    _scaleAssistant.namidi = _namidi;
    
    _mainVC.scaleAssistant = _scaleAssistant;
    _detailVC.scaleAssistant = _scaleAssistant;
    
    _locationView.player = _namidi.player;
    
    [self showMainView];
    
    if (_namidi.hasError) {
        [self showErrorWindow];
    }
    
    [self showEditor];
}

- (void)dealloc
{
    [_namidi removeObserver:self];
    [_namidi.player removeObserver:self];
}

- (void)showErrorWindow
{
    if (!_errorWC) {
        _errorWC = [[ErrorWindowController alloc] init];
        _errorWC.namidi = _namidi;
        
        CGRect frame = self.window.frame;
        frame.origin.x = frame.origin.x + frame.size.width / 2 - CGRectGetWidth(_errorWC.window.frame) / 2;
        frame.origin.y = frame.origin.y + frame.size.height / 2 - CGRectGetHeight(_errorWC.window.frame) / 2;
        frame.size = _errorWC.window.frame.size;
        [_errorWC.window setFrame:frame display:YES];
    }
    
    [self.window addChildWindow:_errorWC.window ordered:NSWindowAbove];
}

- (void)showMainView
{
    [_detailVC.view removeFromSuperview];
    [_contentView addSubviewWithFitConstraints:_mainVC.view];
    _mainVC.scrollPoint = _detailVC.scrollPoint;
}

- (void)showDetailView
{
    [_mainVC.view removeFromSuperview];
    [_contentView addSubviewWithFitConstraints:_detailVC.view];
    _detailVC.scrollPoint = _mainVC.scrollPoint;
}

- (void)showEditor
{
    if ([Preference sharedInstance].externalEditorName) {
        [self showExternalEditor];
    }
    else {
        [self showEditorWindow];
    }
}

- (void)showEditorWindow
{
    if ([Preference sharedInstance].externalEditorName) {
        return;
    }
    
    if (!_editorWC) {
        _editorWC = [[EditorWindowController alloc] init];
    }

    [_editorWC showWindow:self];
    [_editorWC addFileRepresentation:_namidi.file];
}

- (void)showExternalEditor
{
    [[NSWorkspace sharedWorkspace] openFile:_namidi.file.url.path withApplication:[Preference sharedInstance].externalEditorName];
}

- (void)updateToolBarItem
{
    NSToolbarItem *playPause = [self.window.toolbar.items objectPassingTest:^BOOL(__kindof NSToolbarItem *obj, NSUInteger idx, BOOL *stop) {
        return *stop = [obj.itemIdentifier isEqualToString:@"PlayPause"];
    }];
    playPause.image = [NSImage imageNamed:_namidi.player.isPlaying ?  @"pause" : @"play"];
    
    NSToolbarItem *repeat = [self.window.toolbar.items objectPassingTest:^BOOL(__kindof NSToolbarItem *obj, NSUInteger idx, BOOL *stop) {
        return *stop = [obj.itemIdentifier isEqualToString:@"Repeat"];
    }];
    
    switch (_namidi.player.repeatState) {
        case PlayerRepeatStateRepeatOff:
            repeat.image = [NSImage imageNamed:@"repeat_off"];
            break;
        case PlayerRepeatStateRepeatAll:
            repeat.image = [NSImage imageNamed:@"repeat_all"];
            break;
        case PlayerRepeatStateRepeatSection:
            repeat.image = [NSImage imageNamed:@"repeat_marker"];
            break;
    }
}

#pragma mark Menu/Toolbar Action

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
    if (@selector(goBack:) == anItem.action) {
        return nil != _detailVC.view.superview;
    }
    else {
        return YES;
    }
}

- (IBAction)openDocumentInEditor:(id)sender
{
    [self showEditor];
}

- (IBAction)goBack:(id)sender
{
    [self showMainView];
}

- (IBAction)rewind:(id)sender
{
    [_namidi.player rewind];
}

- (IBAction)playPause:(NSToolbarItem *)sender
{
    [_namidi.player playPause];
}

- (IBAction)backward:(id)sender
{
    [_namidi.player backward];
}

- (IBAction)forward:(id)sender
{
    [_namidi.player forward];
}

- (IBAction)toggleRepeat:(id)sender
{
    [_namidi.player toggleRepeat];
}

- (IBAction)export:(id)sender
{
    [AppController exportDocumentForWindow:self.window file:_namidi.file];
}

#pragma mark NSWindowDelegate

- (BOOL)windowShouldClose:(id)sender
{
    [_namidi.player stop];
    return YES;
}

#pragma mark MainViewControllerDelegate

- (void)mainViewControllerDidEnterSelection:(MainViewController *)controller
{
    [self showDetailView];
}

- (void)mainViewController:(MainViewController *)controller didSelectPresetButtonWithChannel:(MixerChannelRepresentation *)mixerChannel
{
    _presetSelectionController.mixerChannel = mixerChannel;
    _isPresetSelectionShown = YES;
    [self.window beginSheet:_presetSelectionController.window completionHandler:^(NSModalResponse returnCode) {
        _isPresetSelectionShown = NO;
    }];
}

#pragma mark TrackSelectionDelegate

- (void)trackSelectionDidEnterSelection:(TrackSelection *)trackSelection
{
    [self showDetailView];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    if (namidi.hasError) {
        if (GeneralParseErrorFileNotFound == parseInfo.errors[0].code) {
            [_errorWC close];
            
            if (_isPresetSelectionShown) {
                [self.window endSheet:_presetSelectionController.window];
            }
            
            NSString *informative = NSLocalizedString(@"Main_FileMissingInformative", @"\"%@/%@\" is missing.\nThis error is unrecoverable.");
            
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = NSLocalizedString(@"FileMissing", @"File missing");
            alert.informativeText = [NSString stringWithFormat:informative, _namidi.file.directory, _namidi.file.filename];
            [alert addButtonWithTitle:NSLocalizedString(@"Close", @"Close")];;
            [alert beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse returnCode) {
                [self close];
            }];
        }
        else {
            [self showErrorWindow];
        }
    }
    else {
        [_errorWC close];
    }
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onNotifyEvent:(PlayerEvent)event
{
    switch (event) {
        case PlayerEventPlay:
        case PlayerEventStop:
        case PlayerEventRepeatStateChange:
            [self updateToolBarItem];
            break;
        default:
            break;
    }
}

@end

#pragma mark Drag and Drop

@interface MainWindow : NSWindow
@end

@implementation MainWindow

- (void)awakeFromNib
{
    [self registerForDraggedTypes:@[NSFilenamesPboardType]];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSArray *files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    for (NSString *filename in files) {
        if (![AppController.allowedFileTypes containsObject:filename.pathExtension.lowercaseString]) {
            return NSDragOperationNone;
        }
    }
    
    return NSDragOperationCopy;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSArray *files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    for (NSString *filename in files) {
        [AppController openDocumentWithContentsOfURL:[NSURL fileURLWithPath:filename]];
    }
    return YES;
}

@end
