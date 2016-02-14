//
//  EditorViewController.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorViewController.h"

@interface EditorViewController ()
@property (unsafe_unretained) IBOutlet NSTextView *textView;
@end

@implementation EditorViewController

- (NSString *)nibName
{
    return @"EditorViewController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.textView.string = [NSString stringWithContentsOfURL:_file.url encoding:NSUTF8StringEncoding error:nil];
}

@end
