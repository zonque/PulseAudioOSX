#import <Foundation/Foundation.h>
#import "StatusBar.h"
#import "ServerTask.h"
#import "Preferences.h"
#import "GrowlNotifications.h"
#import "SocketServer.h"
#import "AudioDeviceClient.h"

int main (int argc, const char * argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];

	SocketServer *server = [[[SocketServer alloc] init] autorelease];
	Preferences *prefs = [[[Preferences alloc] init] autorelease];

	AudioDeviceClient *deviceClient = [[AudioDeviceClient alloc] init];
	[deviceClient setSocketServer: server];
	
	GrowlNotifications *gn = [[[GrowlNotifications alloc] init] autorelease];
	[gn setPreferences: prefs];

	StatusBar *bar = [[[StatusBar alloc] init] autorelease];
	[bar setPreferences: prefs];

	ServerTask *task = [[[ServerTask alloc] init] autorelease];
	[task setPreferences: prefs];

	[server start];
	
	[NSApp setDelegate: bar];
	[NSApp run];
	
	[server stop];
	[task stop];

	[pool drain];
	return 0;
}
