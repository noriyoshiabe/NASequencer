//
//  EditorTabItem.h
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class EditorTabItem;
@protocol EditorTabItemDelegate <NSObject>
- (void)tabItemDidPressCloseButton:(EditorTabItem *)item;
@end

@interface EditorTabItem : NSCollectionViewItem
@property (weak, nonatomic) id<EditorTabItemDelegate> delegate;
@property (assign, nonatomic) BOOL active;
@end
