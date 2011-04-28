#import <Foundation/Foundation.h>

#import "StatusBar.h"
#import "ServerTask.h"
#import "Preferences.h"
#import "GrowlNotifications.h"
#import "ConnectionServer.h"
#import "ObjectNames.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];

	ConnectionServer *server = [[[ConnectionServer alloc] init] autorelease];
	Preferences *prefs = [[[Preferences alloc] init] autorelease];

	GrowlNotifications *gn = [[[GrowlNotifications alloc] init] autorelease];
	[gn setPreferences: prefs];

	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	[bar setPreferences: prefs];

	ServerTask *task = [[[ServerTask alloc] init] autorelease];
	[task setPreferences: prefs];

	[server start];

#if 0
	[[prefs notificationCenter] postNotificationName: @PAOSX_HelperMsgServiceStarted
						  object: @PAOSX_HelperName
						userInfo: nil
				      deliverImmediately: YES];	
#endif

	[NSApp setDelegate: bar];
	[NSApp run];
	
	[task stop];

	[pool release];
	return 0;
}
