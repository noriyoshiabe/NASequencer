//
//  Preference.h
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Preference : NSObject
@property (readonly, nonatomic) BOOL showWelcome;
@property (readwrite, nonatomic) NSString *selectedFileTypeForCreation;
@property (readwrite, nonatomic) NSString *selectedFileTypeForExport;
@property (readwrite, nonatomic) NSString *externalEditorName;
@property (readwrite, nonatomic) NSString *includeSearchPath;
@property (readwrite, nonatomic) NSData *includeSearchPathBookmark;
+ (Preference *)sharedInstance;
- (void)initialize;
@end
