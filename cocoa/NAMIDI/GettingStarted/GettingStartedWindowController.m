//
//  GettingStartedWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "GettingStartedWindowController.h"
#import "ApplicationController.h"
#import "Color.h"

@import WebKit;

@interface GettingStartedWindowController () <WebPolicyDelegate, NSWindowDelegate> {
    BOOL _needOpenExample;
}
@property (weak) IBOutlet WebView *webView;
@end

@implementation GettingStartedWindowController

- (NSString *)windowNibName
{
    return @"GettingStartedWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"GettingStarted"];
    NSURL* fileURL = [NSURL fileURLWithPath:filePath];
    NSURLRequest* request = [NSURLRequest requestWithURL:fileURL];
    [[_webView mainFrame] loadRequest:request];
    
    [_webView setDrawsBackground:NO];
    _webView.policyDelegate = self;
    
    self.window.backgroundColor = [Color darkGray];
    [self.window toggleFullScreen:self];
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    if ([request.URL.scheme isEqualToString:@"nasequencer"]) {
        if ([request.URL.host isEqualToString:@"play"]) {
            
        }
        else if ([request.URL.host isEqualToString:@"exit"]) {
            [self close];
        }
        else if ([request.URL.host isEqualToString:@"example"]) {
            if (self.window.isFullScreen) {
                _needOpenExample = YES;
                [self.window toggleFullScreen:self];
            }
            else {
                [AppController openExampleDocument:@"nas"];
            }
        }
        else if ([request.URL.host isEqualToString:@"syntax"]) {
            if (self.window.isFullScreen) {
                [self.window toggleFullScreen:self];
            }
            NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
            [[NSHelpManager sharedHelpManager] openHelpAnchor:@"syntax_reference" inBook:bookName];
        }
        else if ([request.URL.host isEqualToString:@"operation"]) {
            if (self.window.isFullScreen) {
                [self.window toggleFullScreen:self];
            }
            NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
            [[NSHelpManager sharedHelpManager] openHelpAnchor:@"operation_manual" inBook:bookName];
        }
    }
}

#pragma mark NSWindowDelegate

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    if (_needOpenExample) {
        [AppController openExampleDocument:@"nas"];
        _needOpenExample = NO;
    }
}

@end
