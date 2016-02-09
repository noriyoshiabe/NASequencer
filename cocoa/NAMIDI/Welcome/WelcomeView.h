//
//  WelcomeView.h
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WelcomeView;
@protocol WelcomeViewDelegate <NSObject>
- (void)welcomeView:(WelcomeView *)view closeButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view gettingStartedButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view listenToExampleButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view createNewDocumentButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view helpButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view preferenceButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view openOtherDocumentButtonTapped:(id)sender;
- (void)welcomeView:(WelcomeView *)view showWelcomeWhenStartsToggled:(NSButton *)sender;
- (void)welcomeView:(WelcomeView *)view recentTableViewSelectionChanged:(id)sender selectedRow:(NSInteger)row;
@end

@interface WelcomeView : NSView
@property (strong, nonatomic) NSArray *recentFiles;
@property (weak, nonatomic) id<WelcomeViewDelegate> delegate;
@end
