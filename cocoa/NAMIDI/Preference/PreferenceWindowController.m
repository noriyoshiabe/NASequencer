//
//  PreferenceWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PreferenceWindowController.h"

@interface PreferenceWindowController ()  <NSToolbarDelegate>
@property (weak) IBOutlet NSToolbar *toolBar;
@property (strong, nonatomic) NSMutableArray *viewControllers;
@property (strong, nonatomic) NSViewController<PreferenceViewController> *selectedViewController;
@property (readonly, nonatomic) NSArray *toolbarItemIdentifiers;
@end

@implementation PreferenceWindowController

- (instancetype)init
{
    self = [self initWithWindowNibName:@"PreferenceWindowController"];
    if (self) {
        self.viewControllers = [NSMutableArray array];
    }
    return self;
}

- (void)addViewController:(NSViewController<PreferenceViewController> *)viewController
{
    [_viewControllers addObject:viewController];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    _toolBar.delegate = self;
    self.selectedViewController = _viewControllers[0];
}

- (void)setSelectedViewController:(NSViewController<PreferenceViewController> *)controller
{
    if (_selectedViewController == controller) {
        return;
    }
    
    [_selectedViewController commitEditing];
    _selectedViewController = controller;
    
    self.contentViewController = controller;
    self.window.title = controller.title;
    [_toolBar setSelectedItemIdentifier:controller.identifier];
}

- (NSArray *)toolbarItemIdentifiers
{
    NSMutableArray *identifiers = [NSMutableArray arrayWithCapacity:[_viewControllers count]];
    for (NSViewController<PreferenceViewController> *controller in _viewControllers) {
        [identifiers addObject:controller.identifier];
    }
    return identifiers;
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
