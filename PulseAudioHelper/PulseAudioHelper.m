#import <Foundation/Foundation.h>
#import "Notification.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	Notification *n = [[Notification alloc] init];
	[n start];
	[[NSRunLoop currentRunLoop] run];
	[pool drain];
	return 0;
}
