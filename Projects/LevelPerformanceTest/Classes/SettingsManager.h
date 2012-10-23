#ifndef LevelPerformanceTest_SettingsManager_h
#define LevelPerformanceTest_SettingsManager_h

#include "DAVAEngine.h"

using namespace DAVA;

class SettingsManager: public Singleton<SettingsManager> {
public:
	void Init();
	
	Point2i GetLandscapePartitioning() const;
	const String GetLandscapeNodeName() const;
	float GetCameraElevation() const;
	float GetCameraRotationAngleStep() const;
	float GetCameraMovementSpeed() const;
private:
	Point2i m_LandscapePartitioning;
	String m_LandscapeNodeName;
	float m_fCameraElevation;
	float m_fCameraRotationAngleStep;
	float m_fCameraMovementSpeed;
};

#endif
