//
//  PreferenceWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PreferenceWindowController.h"
#import "GeneralViewController.h"
#import "SynthesizerViewController.h"
#import "PurchaseViewController.h"

@import QuartzCore.CAMediaTimingFunction;

@interface PreferenceWindowController ()  <NSToolbarDelegate>
@property (weak) IBOutlet NSToolbar *toolbar;
@property (strong, nonatomic) NSMutableArray *viewControllers;
@property (strong, nonatomic) NSViewController<PreferenceViewController> *selectedViewController;
@property (readonly, nonatomic) NSArray *toolbarItemIdentifiers;
@end

@implementation PreferenceWindowController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _viewControllers = [NSMutableArray array];
        
        [self addViewController:[[GeneralViewController alloc] init]];
        [self addViewController:[[SynthesizerViewController alloc] init]];
        [self addViewController:[[PurchaseViewController alloc] init]];
    }
    return self;
}

- (NSString *)windowNibName
{
    return @"PreferenceWindowController";
}

- (void)addViewController:(NSViewController<PreferenceViewController> *)viewController
{
    [_viewControllers addObject:viewController];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"PreferenceWindowFrame";
    
    _toolbar.delegate = self;
    
    NSArray *identifiers = self.toolbarItemIdentifiers;
    for (int i = 0; i < identifiers.count; ++i) {
        [_toolbar insertItemWithItemIdentifier:identifiers[i] atIndex: i];
    }
    
    [self setSelectedViewController:_viewControllers[0] animate:NO];
}

- (void)setSelectedViewController:(NSViewController<PreferenceViewController> *)controller
{
    [self setSelectedViewController:controller animate:YES];
}

- (void)setSelectedViewController:(NSViewController<PreferenceViewController> *)controller animate:(BOOL)animate
{
    if (_selectedViewController == controller) {
        return;
    }
    
    [_selectedViewController commitEditing];
    
    self.window.title = controller.toolbarItemLabel;
    [_toolbar setSelectedItemIdentifier:controller.identifier];
    
    _selectedViewController = controller;
    
    [controller layout];
    
    CGRect newWindowFrame = [self.window frameRectForContentRect:controller.view.frame];
    newWindowFrame.origin.x = NSMinX(self.window.frame);
    newWindowFrame.origin.y = NSMinY(self.window.frame) + (NSHeight(self.window.frame) - NSHeight(newWindowFrame));
    
    self.window.contentView = controller.view;
    
    controller.view.alphaValue = 0.0;
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = (animate ? 0.25 : 0);
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.window.animator setFrame:newWindowFrame display:YES];
        controller.view.animator.alphaValue = 1.0;
    } completionHandler:^{}];
}

- (NSArray *)toolbarItemIdentifiers
{
    return [_viewControllers mapObjectsUsingBlock:^id(id obj) {
        return ((id<PreferenceViewController>)obj).identifier;
    }];
}

- (NSViewController<PreferenceViewController> *)controllerForIdentifier:(NSString *)identifier
{
    NSUInteger index = [_viewControllers indexOfObjectPassingTest:^BOOL(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        return *stop = [[(NSViewController<PreferenceViewController> *)obj identifier] isEqualToString:identifier];
    }];
    return _viewControllers[index];
}

- (void)toolbarItemDidClick:(NSToolbarItem *)sender
{
    [_selectedViewController commitEditing];
    self.selectedViewController = [self controllerForIdentifier:sender.itemIdentifier];
}

#pragma mark NSToolbarDelegate

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar * __unused)toolbar
{
    return self.toolbarItemIdentifiers;
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar * __unused)toolbar
{
    return self.toolbarItemIdentifiers;
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar * __unused)toolbar
{
    return self.toolbarItemIdentifiers;
}

- (NSToolbarItem *)toolbar:(NSToolbar * __unused)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL __unused)flag
{
    NSViewController<PreferenceViewController> *controller = [self controllerForIdentifier:itemIdentifier];
    
    NSToolbarItem *toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
    toolbarItem.image = controller.toolbarItemImage;
    toolbarItem.label = controller.toolbarItemLabel;
    toolbarItem.target = self;
    toolbarItem.action = @selector(toolbarItemDidClick:);
    
    return toolbarItem;
}

@end
