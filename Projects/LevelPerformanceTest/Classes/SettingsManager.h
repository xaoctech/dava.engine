#ifndef LevelPerformanceTest_SettingsManager_h
#define LevelPerformanceTest_SettingsManager_h

#include "DAVAEngine.h"

using namespace DAVA;

class SettingsManager: public Singleton<SettingsManager>
{
public:
	void Init();
    void InitWithFile(const String& filename);
	
	Point2i GetLandscapePartitioning() const;
	const String GetLandscapeNodeName() const;
	float32 GetCameraElevation() const;
	float32 GetCameraRotationSpeed() const;
	float32 GetCameraMovementSpeed() const;
private:
	Point2i landscapePartitioning;
	String landscapeNodeName;
	float32 cameraElevation;
	float32 cameraRotationSpeed;
	float32 cameraMovementSpeed;
};

#endif
