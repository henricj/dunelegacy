
#import <Cocoa/Cocoa.h>
#import "MacFunctions.h"

static char macLanguage[3] = "  ";

const char* getMacLanguage() {
	NSUserDefaults * userDefaults = [NSUserDefaults standardUserDefaults];
	NSArray * languageArray = [userDefaults objectForKey:@"AppleLanguages"];
	NSString * firstLanguage = [languageArray objectAtIndex:0];
	
	macLanguage[0] = [firstLanguage characterAtIndex:0];
	macLanguage[1] = [firstLanguage characterAtIndex:1];
		
	return macLanguage;
}

void getMacApplicationSupportFolder(char* buffer, int len) {
	FSRef appSupportFolder;
	OSErr error = noErr;
	NSString* appSupportFolderString;
	
	error = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &appSupportFolder);

	if(error == noErr) {
		CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, &appSupportFolder);
		appSupportFolderString = [(NSURL*) url path];
	} else {
		appSupportFolderString = [@"~/Library/Application Support" stringByExpandingTildeInPath];
	}
	
	[appSupportFolderString getCString:buffer maxLength:len-1 encoding:NSUTF8StringEncoding];
}

