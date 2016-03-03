//
//  SynthesizerViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerViewController.h"
#import "SynthesizerCellView.h"
#import "MidiSourceManagerRepresentation.h"

@import QuartzCore.CAMediaTimingFunction;

@interface SynthesizerViewController () <NSTableViewDataSource, NSTableViewDelegate, MidiSourceManagerRepresentationObserver, SynthesizerCellViewDelegate> {
    MidiSourceManagerRepresentation *_manager;
    CGRect _initialViewRect;
    CGFloat _initilalTableViewHeight;
    NSUInteger _draggedIndex;
}
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSTextField *explanationLabel;
@property (weak) IBOutlet NSLayoutConstraint *tableViewHeightConstraint;
@end

@implementation SynthesizerViewController

- (NSString *)identifier
{
    return @"Synthesizer";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNameApplicationIcon];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Synthesizer", @"Synthesizer");
}

- (void)layout
{
    [self view];
    self.view.frame = self.desiredViewFrame;
    _tableViewHeightConstraint.constant = self.desiredTableViewHeight;
}

- (CGRect)desiredViewFrame
{
    CGFloat height = _initialViewRect.size.height + (_manager.descriptions.count - 1) * _initilalTableViewHeight - _explanationLabel.frame.size.height + self.desiredExplanationLabelHeight;
    return CGRectMake(0, 0, self.view.frame.size.width, height);
}

- (CGFloat)desiredTableViewHeight
{
    return _initilalTableViewHeight * _manager.descriptions.count;
}

- (CGFloat)desiredExplanationLabelHeight
{
    return 1 < _manager.descriptions.count ? _explanationLabel.frame.size.height : 0;
}

- (BOOL)isExplanationLabelHidden
{
    return 2 > _manager.descriptions.count;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [_tableView registerForDraggedTypes:@[NSStringPboardType]];
    
    _initialViewRect = self.view.frame;
    _initilalTableViewHeight = _tableView.frame.size.height;
    
    _manager = [MidiSourceManagerRepresentation sharedInstance];
    [_manager addObserver:self];
    
    _tableView.dataSource = self;
    _tableView.delegate = self;
    
    _explanationLabel.hidden = self.isExplanationLabelHidden;
    
    [_tableView reloadData];
}

- (void)dealloc
{
    [_manager removeObserver:self];
}

- (IBAction)addSynthesizerButtonPressed:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = @[@"sf2"];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                [_manager loadMidiSourceDescriptionFromSoundFont:openPanel.URL.path];
            });
        }
    }];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    [self.view.window makeFirstResponder:self];
}

- (void)resizeWindowFrameAndReload
{
    CGRect newWindowFrame = [self.view.window frameRectForContentRect:self.desiredViewFrame];
    newWindowFrame.origin.x = NSMinX(self.view.window.frame);
    newWindowFrame.origin.y = NSMinY(self.view.window.frame) + (NSHeight(self.view.window.frame) - NSHeight(newWindowFrame));
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.25;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.view.window.animator setFrame:newWindowFrame display:YES];
        _tableViewHeightConstraint.animator.constant = self.desiredTableViewHeight;
        _explanationLabel.animator.hidden = self.isExplanationLabelHidden;
    } completionHandler:^{
        [_tableView reloadData];
    }];
}

- (void)changeMidiSourceOrder:(NSUInteger)fromIndex toIndex:(NSUInteger)toIndex
{
    NSMutableArray *descriptions = _manager.descriptions.mutableCopy;
    MidiSourceDescriptionRepresentation *description = _manager.descriptions[fromIndex];
    [descriptions removeObject:description];
    [descriptions insertObject:description atIndex:toIndex];
    [_manager setReorderdDescriptions:descriptions];
    [_tableView.animator moveRowAtIndex:fromIndex toIndex:toIndex];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _manager.descriptions.count;
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    SynthesizerCellView *view = [tableView makeViewWithIdentifier:@"SynthesizerCell" owner:nil];
    view.delegate = self;
    view.description = _manager.descriptions[row];
    return view;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    NSInteger selectedRow = [_tableView selectedRow];
    NSTableRowView *myRowView = [_tableView rowViewAtRow:selectedRow makeIfNecessary:NO];
    [myRowView setEmphasized:NO];
}

- (id<NSPasteboardWriting>)tableView:(NSTableView *)tableView pasteboardWriterForRow:(NSInteger)row
{
    return _manager.descriptions[row].filepath;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
    _draggedIndex = rowIndexes.firstIndex;
    
    SynthesizerCellView *view = [_tableView viewAtColumn:0 row:rowIndexes.firstIndex makeIfNecessary:NO];
    
    if (view) {
        [session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
                                           forView:tableView
                                           classes:[NSArray arrayWithObject:[NSPasteboardItem class]]
                                     searchOptions:@{}
                                        usingBlock:^(NSDraggingItem *draggingItem, NSInteger index, BOOL *stop)
         {
             draggingItem.imageComponentsProvider = ^NSArray*(void) {
                 return view.draggingImageComponents;
             };
         }];
    }
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation
{
    if (NSTableViewDropAbove == dropOperation) {
        if (_draggedIndex < row) {
            --row;
        }
        return _draggedIndex != row ? NSDragOperationMove : NSDragOperationNone;
    }
    else {
        return NSDragOperationNone;
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
    if (_draggedIndex < row) {
        --row;
    }
    
    if (_draggedIndex != row) {
        [self changeMidiSourceOrder:_draggedIndex toIndex:row];
        return YES;
    }
    else {
        return NO;
    }
}

#pragma mark SynthesizerRowViewDelegate

- (void)synthesizerCellViewDidClickUnload:(SynthesizerCellView *)view
{
    [_manager unloadMidiSourceDescription:view.description];
}

#pragma mark MidiSourceManagerRepresentationObserver

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
}

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
}

@end
