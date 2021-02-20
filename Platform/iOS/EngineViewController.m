#import "EngineView.h"
#import "EngineViewController.h"

@interface EngineViewController ()
{
	EngineView *_engineView;
}

@end

@implementation EngineViewController

- (void)viewDidLoad {
	[super viewDidLoad];
	
	_engineView = [[EngineView alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
	
	self.view = _engineView;
	[self.view retain];
}

@end
