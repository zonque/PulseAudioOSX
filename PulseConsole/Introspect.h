
#import <Cocoa/Cocoa.h>

@interface Introspect : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	IBOutlet NSTableView	*selectionTableView;
	IBOutlet NSTableView	*parameterTableView;
	IBOutlet NSTableView	*propertyTableView;
	IBOutlet NSButton	*activeButton;

	NSMutableDictionary *serverinfo;
	NSMutableArray *outlineToplevel;
	NSMutableArray *cards;
	NSMutableArray *sinks;
	NSMutableArray *sinkInputs;
	NSMutableArray *sources;
	NSMutableArray *sourceOutputs;
	NSMutableArray *clients;
	NSMutableArray *modules;
	NSMutableArray *samplecache;

	NSDictionary *activeItem;
	PAServerConnection *connection;
}

@property (nonatomic, retain) PAServerConnection *connection;

- (void) serverInfoChanged: (PAServerInfo *) info;
- (void) cardsChanged: (NSArray *) cards;
- (void) sinksChanged: (NSArray *) sinks;
- (void) sinkInputsChanged: (NSArray *) inputs;
- (void) sourcesChanged: (NSArray *) sources;
- (void) sourceOutputsChanged: (NSArray *) outputs;
- (void) clientsChanged: (NSArray *) clients;
- (void) modulesChanged: (NSArray *) modules;
- (void) samplesChanged: (NSArray *) samples;
- (void) invalidateAll;

- (void) enableGUI: (BOOL) enabled;
- (void) repaintViews;

@end
