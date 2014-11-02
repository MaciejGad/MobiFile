#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "MobiReader.h"

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
    MobiReader *reader = [[MobiReader alloc] init];
    NSError *error;
    if ([reader readFileForm:(__bridge NSURL *)url error:&error]) {
        NSString *html = [reader readContents:&error];
        NSData *data = [html dataUsingEncoding:NSUTF8StringEncoding];
        if (data) {
            NSRect viewRect = NSMakeRect(0.0, 0.0, 600.0, 800.0);
            float scale = maxSize.height / 800.0;
            NSSize scaleSize = NSMakeSize(scale, scale);
            CGSize thumbSize = NSSizeToCGSize(
                                              NSMakeSize((maxSize.width * (600.0/800.0)),
                                                         maxSize.height));
            
            WebView* webView = [[WebView alloc] initWithFrame: viewRect];
            [webView scaleUnitSquareToSize: scaleSize];
            [[[webView mainFrame] frameView] setAllowsScrolling:NO];
            [[webView mainFrame] loadData: data
                                 MIMEType: @"text/html"
                         textEncodingName: @"utf-8"
                                  baseURL: nil];
            
            while([webView isLoading]) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
            }
            
            [webView display];
            
            CGContextRef context =
            QLThumbnailRequestCreateContext(thumbnail, thumbSize, false, NULL);
            
            if (context) {
                NSGraphicsContext* nsContext =
                [NSGraphicsContext
                 graphicsContextWithGraphicsPort: (void*) context
                                                        flipped: [webView isFlipped]];
                
                [webView displayRectIgnoringOpacity: [webView bounds]
                                          inContext: nsContext];
                
                QLThumbnailRequestFlushContext(thumbnail, context);
                
                CFRelease(context);
            }
        }
    }
    return noErr;
    // To complete your generator please implement the function GenerateThumbnailForURL in GenerateThumbnailForURL.c
    return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}
