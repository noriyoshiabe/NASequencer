//
//  TipsViewController.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TipsViewController.h"

@interface TipsViewController ()
@property (weak) IBOutlet NSTextField *operationManual;
@property (weak) IBOutlet NSTextField *syntaxReference;
@end

@implementation TipsViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [_operationManual underLine];
    [_syntaxReference underLine];
}

- (IBAction)operationManualPressed:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"operation_manual" inBook:bookName];
}

- (IBAction)syntaxReferencePressed:(id)sender
{
    NSString *ext = _namidi.file.filename.pathExtension.lowercaseString;
    if ([ext isEqualToString:@"nmf"]) {
        ext = @"namidi";
    }
    
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:ext inBook:bookName];
}

@end
