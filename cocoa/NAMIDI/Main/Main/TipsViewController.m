//
//  TipsViewController.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TipsViewController.h"
#import "ApplicationController.h"

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
    [AppController openHelpPage:@"operation-manual"];
}

- (IBAction)syntaxReferencePressed:(id)sender
{
    NSString *pageName = @{@"nas": @"note-as-sequence",
                         @"abc": @"abc-notation",
                         @"mml": @"music-macro-language"}[_namidi.file.filename.pathExtension.lowercaseString];
    [AppController openHelpPage:pageName];
}

@end
