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
#import "ApplicationController.h"
#import "IAP.h"

#define kAddSynthesizerHeight 35.0
#define kExplanationHeight 33.30

@import QuartzCore.CAMediaTimingFunction;

@interface SynthesizerViewController () <NSTableViewDataSource, NSTableViewDelegate, MidiSourceManagerRepresentationObserver, SynthesizerCellViewDelegate, IAPObserver> {
    MidiSourceManagerRepresentation *_manager;
    CGRect _initialViewRect;
    CGFloat _initilalTableViewHeight;
    NSUInteger _draggedIndex;
}
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSLayoutConstraint *tableViewHeightConstraint;
@property (weak) IBOutlet NSLayoutConstraint *addSynthesizerHeightConstraint;
@property (weak) IBOutlet NSLayoutConstraint *explanationHeightConstraint;
@end

@implementation SynthesizerViewController

- (NSString *)identifier
{
    return @"Synthesizer";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:@"synthesizer"];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Synthesizer", @"Synthesizer");
}

- (void)layout
{
    [self view];
    
    _tableViewHeightConstraint.constant = self.desiredTableViewHeight;
    [self updateAdditionalViewHeightConstraint];
    
    self.view.frame = self.desiredViewFrame;
}

- (CGRect)desiredViewFrame
{
    CGFloat height = _initialViewRect.size.height - kAddSynthesizerHeight - kExplanationHeight
                   + (_manager.descriptions.count - 1) * _initilalTableViewHeight
                   + _addSynthesizerHeightConstraint.constant + _explanationHeightConstraint.constant;
    return CGRectMake(0, 0, self.view.frame.size.width, height);
}

- (CGFloat)desiredTableViewHeight
{
    return _initilalTableViewHeight * _manager.descriptions.count;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [_tableView registerForDraggedTypes:@[NSStringPboardType]];
    _tableView.selectionHighlightStyle = NSTableViewSelectionHighlightStyleNone;
    
    _initialViewRect = self.view.frame;
    _initilalTableViewHeight = _tableView.frame.size.height;
    
    _manager = [MidiSourceManagerRepresentation sharedInstance];
    [_manager addObserver:self];
    
    _tableView.dataSource = self;
    _tableView.delegate = self;
    
    [_tableView reloadData];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    [[IAP sharedInstance] addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [[IAP sharedInstance] removeObserver:self];
}

- (void)dealloc
{
    [_manager removeObserver:self];
}

- (void)updateAdditionalViewHeightConstraint
{
    [[IAP sharedInstance] findIAPProduct:kIAPProductFullVersion found:^(NSString *productID, int quantity) {
        _addSynthesizerHeightConstraint.constant = kAddSynthesizerHeight;
    } notFound:^(NSString *productID) {
        _addSynthesizerHeightConstraint.constant = 0.0;
    }];
    
    _explanationHeightConstraint.constant = 1 < _manager.descriptions.count ? kExplanationHeight : 0.0;
}

- (IBAction)addSynthesizerButtonPressed:(id)sender
{
    [[IAP sharedInstance] findIAPProduct:kIAPProductFullVersion found:^(NSString *productID, int quantity) {
        NSOpenPanel *openPanel = [NSOpenPanel openPanel];
        openPanel.allowedFileTypes = @[@"sf2"];
        
        [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
            if (NSFileHandlingPanelOKButton == result) {
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                    [_manager loadMidiSourceDescriptionFromSoundFont:openPanel.URL.path];
                });
            }
        }];
    } notFound:nil];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    [self.view.window makeFirstResponder:self];
}

- (void)resizeWindowFrameAndReload
{
    [self updateAdditionalViewHeightConstraint];
    
    CGRect newWindowFrame = [self.view.window frameRectForContentRect:self.desiredViewFrame];
    newWindowFrame.origin.x = NSMinX(self.view.window.frame);
    newWindowFrame.origin.y = NSMinY(self.view.window.frame) + (NSHeight(self.view.window.frame) - NSHeight(newWindowFrame));
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.25;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.view.window.animator setFrame:newWindowFrame display:YES];
        _tableViewHeightConstraint.animator.constant = self.desiredTableViewHeight;
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

#pragma mark IAPObserver

- (void)iap:(IAP *)iap didUpdateTransaction:(SKPaymentTransaction *)transaction
{
    [self resizeWindowFrameAndReload];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _manager.descriptions.count;
}

#pragma mark NSTableViewDelegate


- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
    return [tableView makeViewWithIdentifier:@"SynthesizerRow" owner:nil];
}

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
    if (-1 != _tableView.selectedRow) {
        NSTableRowView *myRowView = [_tableView rowViewAtRow:_tableView.selectedRow makeIfNecessary:NO];
        [myRowView setEmphasized:NO];
    }
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
    if (NSTableViewDropOn == dropOperation) {
        return _draggedIndex != row ? NSDragOperationMove : NSDragOperationNone;
    }
    else {
        return NSDragOperationNone;
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
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

- (void)synthesizerCellViewDidClickDelete:(SynthesizerCellView *)view
{
    [_manager unloadMidiSourceDescription:view.description];
}

- (void)synthesizerCellViewDidClickReload:(SynthesizerCellView *)view
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        bool success = [_manager reloadMidiSourceDescription:view.description];
        
        if (!success) {
            NSBeep();
        }
    });
}

#pragma mark MidiSourceManagerRepresentationObserver

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
    [_manager saveMidiSourcePreference];
}

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
    [_manager saveMidiSourcePreference];
}

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onReorderMidiSourceDescriptions:(NSArray<MidiSourceDescriptionRepresentation *> *)descriptions availableDescriptions:(NSArray<MidiSourceDescriptionRepresentation *> *)availableDescriptions
{
    [_tableView reloadData];
    [_manager saveMidiSourcePreference];
}

@end

#pragma mark Custom selection background

@interface SynthesizerRowView : NSTableRowView
@end

@implementation SynthesizerRowView

- (void)awakeFromNib
{
    [super awakeFromNib];
    self.draggingDestinationFeedbackStyle = NSTableViewDraggingDestinationFeedbackStyleNone;
}

- (void)drawDraggingDestinationFeedbackInRect:(NSRect)dirtyRect
{
    [[NSColor selectedControlColor] set];
    [[NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:4.0 yRadius:4.0] fill];
}

@end
