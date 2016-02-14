//
//  GettingStartedWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "GettingStartedWindowController.h"

@import WebKit;

@interface GettingStartedWindowController () <WebPolicyDelegate>
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
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"getting-started"];
    NSURL* fileURL = [NSURL fileURLWithPath:filePath];
    NSURLRequest* request = [NSURLRequest requestWithURL:fileURL];
    [[_webView mainFrame] loadRequest:request];
    
    [_webView setDrawsBackground:NO];
    _webView.policyDelegate = self;
    
    self.window.backgroundColor = [NSColor grayColor];
    [self.window toggleFullScreen:self];
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
    if ([request.URL.scheme isEqualToString:@"namidi"]) {
        [self close];
    }
}

@end
