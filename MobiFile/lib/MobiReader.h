//
//  MobiReader.h
//  MobiTest
//
//  Created by Maciej Gad on 02.11.2014.
//  Copyright (c) 2014 Maciej Gad. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "mobi.h"

static NSString *mobiReaderDomainError = @"mobiReaderDomainError";

typedef enum : NSUInteger {
    MobiReaderErrorMemoryAllocationFailed = 100,
    MobiReaderErrorOpeningFile,
    MobiReaderErrorParsingText,
    MobiReaderErrorConversion
} MobiReaderErrorCode;

@interface MobiReader : NSObject

@property (nonatomic, readonly) MOBIData *data;

-(BOOL) readFileForm:(NSURL *) url error:(NSError *__autoreleasing*) error;
-(NSString *) fullname;
-(NSString *) readContents:(NSError *__autoreleasing *) error;
@end
