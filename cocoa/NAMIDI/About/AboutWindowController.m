//
//  AboutWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AboutWindowController.h"

@interface AboutWindowController ()
@property (readonly) NSString *appName;
@property (readonly) NSString *version;
@property (readonly) NSString *copyright;
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
    
    [_license underLine];
    [_officialSite underLine];
    [_supportMail underLine];
}

- (NSString *)appName
{
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
}

- (NSString *)copyright
{
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"];
}

- (NSString *)version
{
    return [NSBundle versionString];
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
    NSString *subject = NSLocalizedString(@"About_MailSubject", @"Inquiry about NASequencer");
    
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    NSString *osVersion = [NSString stringWithFormat:@"OS Version: %ld.%ld.%ld", version.majorVersion, version.minorVersion, version.patchVersion];
    NSString *body = [NSString stringWithFormat:NSLocalizedString(@"About_MailBody", @"%@"), osVersion];
    NSString *mailtoAddress = [NSString stringWithFormat:@"mailto:%@?Subject=%@&body=%@", sender.stringValue, subject, body];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[mailtoAddress stringByReplacingOccurrencesOfString:@" " withString:@"%20"]]];
}

@end
