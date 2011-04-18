#import <Foundation/Foundation.h>
#import "Notification.h"
#import "StatusBar.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
	
	Notification *n = [[Notification alloc] init];
	[n start];

	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	
	[NSApp setDelegate: bar];
	[NSApp run];

	[pool drain];
	return 0;
}
