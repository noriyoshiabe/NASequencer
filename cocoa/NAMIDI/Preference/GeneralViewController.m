//
//  GeneralViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "GeneralViewController.h"

@interface GeneralViewController ()
@property (assign, nonatomic) BOOL useExternalEditor;
@property (readonly, nonatomic) NSImage *editorIconImage;
@property (readonly, nonatomic) NSString *editorName;
@property (readonly, nonatomic) NSString *editorSelectButtonLabel;
@end

@implementation GeneralViewController

- (NSString *)identifier
{
    return @"General";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNamePreferencesGeneral];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_General", @"General");
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
}

- (void)setUseExternalEditor:(BOOL)useExternalEditor
{
    _useExternalEditor = useExternalEditor;
    if (!_useExternalEditor) {
        
    }
    
    [self willChangeValueForKey:@"editorName"];
    [self didChangeValueForKey:@"editorName"];
    [self willChangeValueForKey:@"editorIconImage"];
    [self didChangeValueForKey:@"editorIconImage"];
    [self willChangeValueForKey:@"editorSelectButtonLabel"];
    [self didChangeValueForKey:@"editorSelectButtonLabel"];
}

- (NSImage *)editorIconImage
{
    return _useExternalEditor ? [NSImage imageNamed:@"NMFIcon"] : [NSApp applicationIconImage];
}

- (NSString *)editorName
{
    return _useExternalEditor ? @"Internal Editor -" : @"Internal Editor";
}

- (NSString *)editorSelectButtonLabel
{
    return _useExternalEditor ? @"Select -" : @"Select";
}

@end
