#import <Foundation/Foundation.h>
#import "StatusBar.h"
#import "ServerTask.h"
#import "Preferences.h"
#import "GrowlNotifications.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
	
	Preferences *prefs = [[[Preferences alloc] init] autorelease];

	GrowlNotifications *gn = [[[GrowlNotifications alloc] init] autorelease];
	[gn setPreferences: prefs];

	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	[bar setPreferences: prefs];

	ServerTask *server = [[[ServerTask alloc] init] autorelease];
	[server setPreferences: prefs];

	[NSApp setDelegate: bar];
	[NSApp run];
	
	[server stop];

	[pool drain];
	return 0;
}
