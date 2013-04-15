#ifndef __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__
#define __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__

#include "DAVAEngine.h"

using namespace DAVA;

struct FpsStatItem
{
	float32 minFps;
	DAVA::Rect rect;
	Vector3 position;
	Vector3 viewTarget;
	
	FpsStatItem()
	:	minFps(std::numeric_limits<float32>::infinity())
	{
	}
};

class LandscapeTestData
{
private:
	DAVA::Rect landscapeRect;
	Vector<FpsStatItem> stat;

	uint32 textureMemorySize;
	uint32 textureFilesSize;
	uint32 sceneFileSize;

public:
	void SetLandscapeRect(const DAVA::Rect& rect);
	void AddStatItem(const FpsStatItem& item);

	void SetTextureMemorySize(uint32 size);
	void SetTexturesFilesSize(uint32 size);
	void SetSceneFileSize(uint32 size);
	uint32 GetTextureMemorySize() const;
	uint32 GetTexturesFilesSize() const;
	uint32 GetSceneFileSize() const;

	const DAVA::Rect& GetLandscapeRect() const;
	uint32 GetItemCount() const;
	const FpsStatItem& GetItem(uint32 index) const;

	void Clear();
	DAVA::Rect TranslateRect(const DAVA::Rect& rect, const DAVA::Rect& destPlane) const;
	Vector2 TranslatePoint(const Vector2& point, const DAVA::Rect& destPlane) const;
};

#endif
