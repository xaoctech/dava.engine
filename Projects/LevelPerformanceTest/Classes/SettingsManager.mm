#import "SettingsManager.h"

#import <Foundation/Foundation.h>

#define H 10.f
#define ANGLE_STEP 1.f
#define VELOCITY 4.5f
#define LANDSCAPE_NODE_NAME "Landscape"
#define X 3
#define Y 4

SettingsManager sm;

void SettingsManager::Init() {
	m_LandscapeNodeName = LANDSCAPE_NODE_NAME;
	m_LandscapePartitioning.x = X;
	m_LandscapePartitioning.y = Y;
	m_fCameraElevation = H;
	m_fCameraRotationAngleStep = ANGLE_STEP;
	m_fCameraMovementSpeed = VELOCITY;

	String settingsFilePath = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "settings.plist";
	NSString *nsPath = [NSString stringWithCString:settingsFilePath.c_str() encoding:NSUTF8StringEncoding];
	NSMutableDictionary *settings = [[NSMutableDictionary alloc] initWithContentsOfFile:nsPath];

	NSNumber *n;
	
	n = [settings valueForKey:@"land_parts_x"];
	if(n != nil) {
		m_LandscapePartitioning.x = [n intValue];
	}

	n = [settings valueForKey:@"land_parts_y"];
	if(n != nil) {
		m_LandscapePartitioning.y = [n intValue];
	}
	
	n = [settings valueForKey:@"camera_elevation"];
	if(n != nil) {
		m_fCameraElevation = [n floatValue];
	}
	
	n = [settings valueForKey:@"camera_rotation_angle_step"];
	if(n != nil) {
		m_fCameraRotationAngleStep = [n floatValue];
	}
	
	n = [settings valueForKey:@"camera_movement_speed"];
	if(n != nil) {
		m_fCameraMovementSpeed = [n floatValue];
	}
	
	NSString *s;
	
	s = [settings valueForKey:@"land_node_name"];
	if(s != nil) {
		m_LandscapeNodeName = String([s UTF8String]);
	}
}

Point2i SettingsManager::GetLandscapePartitioning() const {
	return m_LandscapePartitioning;
}

const String SettingsManager::GetLandscapeNodeName() const {
	return m_LandscapeNodeName;
}

float SettingsManager::GetCameraElevation() const {
	return m_fCameraElevation;
}

float SettingsManager::GetCameraRotationAngleStep() const {
	return m_fCameraRotationAngleStep;
}

float SettingsManager::GetCameraMovementSpeed() const {
	return m_fCameraMovementSpeed;
}