
#import <Cocoa/Cocoa.h>
#import "ServerConnection.h"

@interface Introspect : NSObject
{
	IBOutlet NSOutlineView          *outlineView;
	IBOutlet NSTableView            *parameterTableView;
	IBOutlet NSTableView            *propertyTableView;
	IBOutlet NSButton               *activeButton;

	NSDictionary *activeItem;
	ServerConnection *serverConnection;
}

@property (nonatomic, retain) ServerConnection *serverConnection;

- (void) enableGUI: (BOOL) enabled;
- (void) repaintViews;
- (void) stopProgressIndicator;

@end
