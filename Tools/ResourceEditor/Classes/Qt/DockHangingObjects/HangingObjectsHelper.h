#ifndef __RESOURCEEDITORQT__HANGING_OBJECTS_HELPER_H__
#define __RESOURCEEDITORQT__HANGING_OBJECTS_HELPER_H__

#include "DAVAEngine.h"

class SceneData;


namespace DAVA
{
class HangingObjectsHelper
{
public:
	
	static void ProcessHangingObjectsUpdate(float value, bool isEnabled);


private:

	static Vector3 GetLandscapePointAtCoordinates(const Vector2& centerXY, SceneData *sceneData);

	static Vector3 GetLowestPointFromRect(Rect& rect, float density, SceneData *sceneData);

	static void GetLowerPoint(const Vector2& candidate, Vector3& currentLower, SceneData *sceneData);
};
};

#endif /* defined(__RESOURCEEDITORQT__HANGING_OBJECTS_HELPER_H__) */
