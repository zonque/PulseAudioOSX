#import <Foundation/Foundation.h>
#import "Notification.h"
#import "StatusBar.h"
#import "ServerTask.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
	
	Notification *n = [[[Notification alloc] init] autorelease];
	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	ServerTask *server = [[[ServerTask alloc] init] autorelease];
	
	[NSApp setDelegate: bar];
	[NSApp run];
	
	[server stop];

	[pool drain];
	return 0;
}
