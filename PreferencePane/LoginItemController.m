/*
 The MIT License
 Copyright (c) 2010 Justin Williams, Second Gear
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */


#import "LoginItemController.h"

@implementation LoginItemController (PrivateMethods)
- (void)enableLoginItemWithLoginItemsReference:(LSSharedFileListRef )theLoginItemsRefs ForPath:(NSString *)appPath {
        // We call LSSharedFileListInsertItemURL to insert the item at the bottom of Login Items list.
        NSDictionary *properties = [NSDictionary dictionaryWithObject: [NSNumber numberWithBool:YES]
                                                               forKey: @"com.apple.loginitem.HideOnLaunch"];
        CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:appPath];
        LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(theLoginItemsRefs,
                                                                     kLSSharedFileListItemLast,
                                                                     NULL, NULL, url,
                                                                     (CFDictionaryRef) properties,
                                                                     NULL);
        if (item)
                CFRelease(item);
}

- (void)disableLoginItemWithLoginItemsReference:(LSSharedFileListRef )theLoginItemsRefs ForPath:(NSString *)appPath {
        UInt32 seedValue;
        CFURLRef thePath;
        // We're going to grab the contents of the shared file list (LSSharedFileListItemRef objects)
        // and pop it in an array so we can iterate through it to find our item.
        CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(theLoginItemsRefs, &seedValue);
        for (id item in (NSArray *)loginItemsArray) {
                LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
                if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr) {
                        if ([[(NSURL *)thePath path] hasPrefix:appPath]) {
                                LSSharedFileListItemRemove(theLoginItemsRefs, itemRef); // Deleting the item
                        }
                        // Docs for LSSharedFileListItemResolve say we're responsible
                        // for releasing the CFURLRef that is returned
                        CFRelease(thePath);
                }
        }
        CFRelease(loginItemsArray);
}

- (BOOL)loginItemExistsWithLoginItemReference:(LSSharedFileListRef)theLoginItemsRefs ForPath:(NSString *)appPath {
        BOOL found = NO;
        UInt32 seedValue;
        CFURLRef thePath;

        // We're going to grab the contents of the shared file list (LSSharedFileListItemRef objects)
        // and pop it in an array so we can iterate through it to find our item.
        CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(theLoginItemsRefs, &seedValue);
        for (id item in (NSArray *)loginItemsArray) {
                LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
                if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr) {
                        if ([[(NSURL *)thePath path] hasPrefix:appPath]) {
                                found = YES;
                                break;
                        }
                }
                // Docs for LSSharedFileListItemResolve say we're responsible
                // for releasing the CFURLRef that is returned
                CFRelease(thePath);
        }
        CFRelease(loginItemsArray);

        return found;
}
@end

@implementation LoginItemController

static NSString * appPath = @"/Library/Frameworks/pulse.framework/Resources/bin/PulseAudioHelper.app";

- (void)awakeFromNib {
        LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
        if ([self loginItemExistsWithLoginItemReference:loginItems ForPath:appPath]) {
                [btnToggleLoginItem setState:NSOnState];
        }
        CFRelease(loginItems);
}

- (IBAction)toggleLoginItem:(id)sender {
        // Create a reference to the shared file list.
        LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
        if (loginItems) {
                if ([sender state] == NSOnState)
                        [self enableLoginItemWithLoginItemsReference:loginItems ForPath:appPath];
                else
                        [self disableLoginItemWithLoginItemsReference:loginItems ForPath:appPath];
        }
        CFRelease(loginItems);
}

@end
