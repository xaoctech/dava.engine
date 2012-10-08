#import <Foundation/Foundation.h>

#import "FileManagerWrapper.h"

const Vector<String> FileManagerWrapper::GetFileListByExtension(const String& path, const String& ext) {
	NSString *searchPath=[NSString stringWithCString:path.c_str() encoding:[NSString defaultCStringEncoding]];
	NSFileManager* fm=[NSFileManager defaultManager];
	NSArray *searchDir=[fm contentsOfDirectoryAtPath:searchPath error:nil];
	
	NSString *nsExt=[NSString stringWithCString:ext.c_str() encoding:[NSString defaultCStringEncoding]];
	NSString *predicateFormat=[NSString stringWithFormat:@"self ENDSWITH '%@'", nsExt];
	NSPredicate *filter=[NSPredicate predicateWithFormat:predicateFormat];
	NSArray *fileList=[searchDir filteredArrayUsingPredicate:filter];

	Vector<String> list;
	if([fileList count] > 0) {
		for(id file in fileList) {
			list.push_back([file UTF8String]);
		}
	}

	return list;
}