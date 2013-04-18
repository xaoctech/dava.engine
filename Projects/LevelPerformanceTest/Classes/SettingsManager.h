#ifndef LevelPerformanceTest_SettingsManager_h
#define LevelPerformanceTest_SettingsManager_h

#include "DAVAEngine.h"

using namespace DAVA;

class SettingsManager: public Singleton<SettingsManager>
{
public:
    void InitWithFile(const String& filename);
	
	Vector2 GetLandscapePartitioningSize() const;
	const String GetLandscapeNodeName() const;
	float32 GetCameraElevation() const;
	float32 GetCameraRotationSpeed() const;
	float32 GetCameraMovementSpeed() const;
	float32 GetMinFps() const;
	uint32 GetMinFpsSectorCount() const;
	uint8 GetColorTransparency() const;
	Color GetColorByFps(float32 fps) const;
private:
	Vector2 landscapePartitioningSize;
	String landscapeNodeName;
	float32 cameraElevation;
	float32 cameraRotationSpeed;
	float32 cameraMovementSpeed;
	float32 minFps;
	uint32 minFpsSectorCount;
	uint8 colorTransparency;
	Vector<Color> colorList;
	Vector<float32> fpsList;
};

#endif
