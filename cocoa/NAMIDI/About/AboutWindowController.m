//
//  AboutWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AboutWindowController.h"

@interface AboutWindowController ()
@property (weak) IBOutlet NSTextField *appName;
@property (weak) IBOutlet NSTextField *version;
@property (weak) IBOutlet NSTextField *copyright;
@property (weak) IBOutlet NSTextField *license;
@property (weak) IBOutlet NSTextField *officialSite;
@property (weak) IBOutlet NSTextField *supportMail;
@end

@implementation AboutWindowController

- (instancetype)init
{
    return [self initWithWindowNibName:@"AboutWindowController"];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    _appName.stringValue = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    
    NSString *shortVersionString = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    NSString *bundleVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
    _version.stringValue = [NSString stringWithFormat:@"Version %@ (%@)", shortVersionString, bundleVersion];
    _copyright.stringValue = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"];
    
    [self underLineToTextField:_license];
    [self underLineToTextField:_officialSite];
    [self underLineToTextField:_supportMail];
}

- (void)underLineToTextField:(NSTextField *)textField
{
    NSMutableAttributedString *string = [[textField attributedStringValue] mutableCopy];
    [string addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, string.length)];
    [textField setAttributedStringValue:string];
}

- (IBAction)licensePressed:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"license" inBook:bookName];
}

- (IBAction)officialSitePressed:(NSTextField *)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:sender.stringValue]];
}

- (IBAction)supportMailPressed:(NSTextField *)sender
{
    NSString *subject = NSLocalizedString(@"About_MailSubject", @"Inquiry about NAMIDI");
    
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    NSString *osVersion = [NSString stringWithFormat:@"OS Version: %ld.%ld.%ld", version.majorVersion, version.minorVersion, version.patchVersion];
    NSString *body = [NSString stringWithFormat:NSLocalizedString(@"About_MailBody", @"%@"), osVersion];
    NSString *mailtoAddress = [NSString stringWithFormat:@"mailto:%@?Subject=%@&body=%@", sender.stringValue, subject, body];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[mailtoAddress stringByReplacingOccurrencesOfString:@" " withString:@"%20"]]];
}

@end


#pragma mark InterfaceBuilder

@interface AboutWindowTextLink : NSTextField

@end

@implementation AboutWindowTextLink
- (void)mouseUp:(NSEvent *)theEvent
{
    [self sendAction:[self action] to:[self target]];
}
@end