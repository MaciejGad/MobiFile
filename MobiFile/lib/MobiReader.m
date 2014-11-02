//
//  MobiReader.m
//  MobiTest
//
//  Created by Maciej Gad on 02.11.2014.
//  Copyright (c) 2014 Maciej Gad. All rights reserved.
//
#import "MobiReader.h"
#import "util.h"

@interface MobiReader ()

@property (nonatomic, assign) MOBIData *data;

@end

@implementation MobiReader

-(BOOL) readFileForm:(NSURL *) url error:(NSError *__autoreleasing *) error{
    
    MOBI_RET mobi_ret;
    
    /* Initialize main MOBIData structure */
    MOBIData *m = mobi_init();
    if (m == NULL) {
        *error = [self errorWithCode:MobiReaderErrorMemoryAllocationFailed message:@"Memory allocation failed"];
        return NO;
    }

    NSFileHandle *fh = [NSFileHandle fileHandleForReadingFromURL:url error:error];
    if (fh == nil) {
        mobi_free(m);
        return NO;
    }
    FILE *file = fdopen([fh fileDescriptor], "rb");
    if (file == NULL) {
        *error = [self errorWithCode:MobiReaderErrorOpeningFile message:@"Error opening file:"];
        mobi_free(m);
        return NO;
    }
    
    /* MOBIData structure will be filled with loaded document data and metadata */
    mobi_ret = mobi_load_file(m, file);
    fclose(file);
    
    if (mobi_ret != MOBI_SUCCESS) {
        *error = [self errorWithCode:mobi_ret message:@"error while loading document"];
        mobi_free(m);
        return NO;
    }
    mobi_free(self.data);
    self.data = m;
    
    return YES;
}

-(NSString *) fullname {
    if (self.data != NULL && self.data->mh && self.data->mh->full_name_offset && self.data->mh->full_name_length) {
        size_t len = *self.data->mh->full_name_length;
        char full_name[len + 1];
        if(mobi_get_fullname(self.data, full_name, len) == MOBI_SUCCESS) {
            return [NSString stringWithUTF8String:full_name];
        }
    }
    return nil;
}

-(NSString *) readContents:(NSError *__autoreleasing *) error{
    MOBI_RET mobi_ret;
    if (self.data != NULL){
        const size_t maxlen = mobi_get_text_maxsize(self.data);
        char *text = malloc(maxlen + 1);
        if (text == NULL) {
            *error = [self errorWithCode:MobiReaderErrorMemoryAllocationFailed message:@"Memory allocation failed"];
            return nil;
        }
        /* Extract text records, unpack, merge and copy it to text string */
        size_t length = maxlen;
        mobi_ret = mobi_get_rawml(self.data, text, &length);
        if (mobi_ret != MOBI_SUCCESS) {
            free(text);
            *error = [self errorWithCode:MobiReaderErrorParsingText message:@"Error parsing text"];
            return nil;
        }
        /* Work on utf-8 encoded text */
        if (memcmp(text, REPLICA_MAGIC, 4) != 0 && mobi_is_cp1252(self.data)) {
            /* extreme case in which each input character is converted
             to 3-byte utf-8 sequence */
            size_t out_length = 3 * length + 1;
            char *out_text = malloc(out_length);
            if (out_text == NULL) {
                free(text);
                *error = [self errorWithCode:MobiReaderErrorMemoryAllocationFailed message:@"Memory allocation failed"];
                return nil;
            }
            mobi_ret = mobi_cp1252_to_utf8(out_text, text, &out_length, length);
            free(text);
            if (mobi_ret != MOBI_SUCCESS || out_length == 0) {
                free(out_text);
                *error = [self errorWithCode:MobiReaderErrorConversion message:@"conversion from cp1252 to utf8 failed"];
                return nil;
            }
            text = malloc(out_length + 1);
            if (text == NULL) {
                free(out_text);
                *error = [self errorWithCode:MobiReaderErrorMemoryAllocationFailed message:@"Memory allocation failed"];
                return nil;
            }
            memcpy(text, out_text, out_length);
            free(out_text);
            text[out_length] = '\0';
            length = out_length;
        }
        NSString *textString = [NSString stringWithUTF8String:text];
        free(text);
        return textString;
    }
    return nil;
}

-(void) dealloc {
    mobi_free(self.data);
}

-(NSError *) errorWithCode:(NSInteger) code message:(NSString *) message {
    if (message == nil) {
        message = @"Unknow error";
    }
    NSDictionary *errorDetail = @{NSLocalizedDescriptionKey: message};
    return  [NSError errorWithDomain:mobiReaderDomainError code:code userInfo:errorDetail];
}

@end
