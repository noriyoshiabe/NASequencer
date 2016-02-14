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
        self.viewControllers = [NSMutableArray array];
        
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
    self.selectedViewController = _viewControllers[0];
    
    NSArray *identifiers = self.toolbarItemIdentifiers;
    for (int i = 0; i < identifiers.count; ++i) {
        [_toolbar insertItemWithItemIdentifier:identifiers[i] atIndex: i];
    }
}

- (void)setSelectedViewController:(NSViewController<PreferenceViewController> *)controller
{
    if (_selectedViewController == controller) {
        return;
    }
    
    [_selectedViewController commitEditing];
    _selectedViewController = controller;
    
    self.contentViewController = controller;
    self.window.title = controller.toolbarItemLabel;
    [_toolbar setSelectedItemIdentifier:controller.identifier];
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
    [self.contentViewController commitEditing];
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
