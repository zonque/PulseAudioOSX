#import <Foundation/Foundation.h>
#import "StatusBar.h"
#import "ServerTask.h"
#import "Preferences.h"
#import "GrowlNotifications.h"
#import "SocketServer.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
	
	Preferences *prefs = [[[Preferences alloc] init] autorelease];

	GrowlNotifications *gn = [[[GrowlNotifications alloc] init] autorelease];
	[gn setPreferences: prefs];

	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	[bar setPreferences: prefs];

	ServerTask *task = [[[ServerTask alloc] init] autorelease];
	[task setPreferences: prefs];

	SocketServer *server = [[SocketServer alloc] init];
	
	[NSApp setDelegate: bar];
	[NSApp run];
	
	[task stop];

	[pool drain];
	return 0;
}
