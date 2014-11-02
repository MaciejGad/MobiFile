#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#include <Cocoa/Cocoa.h>
#import "MobiReader.h"

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
//    NSString *_content = [NSString stringWithContentsOfURL:(__bridge NSURL *)url encoding:NSUTF8StringEncoding error:nil];
    MobiReader *reader = [[MobiReader alloc] init];
    NSError *error;
    if ([reader readFileForm:(__bridge NSURL *)url error:&error]) {
        NSString *html = [reader readContents:&error];
        QLPreviewRequestSetDataRepresentation(preview,(__bridge CFDataRef)[html dataUsingEncoding:NSUTF8StringEncoding],kUTTypeHTML,NULL);
    } 

    return noErr;
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    //kUTTypeText
    // Implement only if supported
}
