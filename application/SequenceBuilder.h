#import <Foundation/Foundation.h>

#import "Sequence.h"
#import "NAMidiParser.h"

@interface SequenceBuilder : NSObject

- (void)addEvent:(NAMidiParserEventType)type argList:(va_list)argList;
- (void)setTimeTable:(TimeTable *)timeTable;
- (Sequence *)build;

@end
