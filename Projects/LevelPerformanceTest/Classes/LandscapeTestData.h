#ifndef __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__
#define __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__

#include "DAVAEngine.h"

#define SECTORS_COUNT 8

struct FpsStatItem
{
    DAVA::float32 avFps[SECTORS_COUNT];
	DAVA::Rect rect;
	
	FpsStatItem()
	{
	}
};

class LandscapeTestData
{
private:
	DAVA::Rect landscapeRect;
    DAVA::Vector<FpsStatItem> stat;

	DAVA::uint32 textureMemorySize;
	DAVA::uint32 textureFilesSize;
	DAVA::FilePath sceneFilePath;

public:
	void SetLandscapeRect(const DAVA::Rect& rect);
	void AddStatItem(const FpsStatItem& item);

	void SetTextureMemorySize(DAVA::uint32 size);
	void SetTexturesFilesSize(DAVA::uint32 size);
	void SetSceneFilePath(const DAVA::FilePath & path);
	DAVA::uint32 GetTextureMemorySize() const;
	DAVA::uint32 GetTexturesFilesSize() const;
	const DAVA::FilePath & GetSceneFilePath() const;

	const DAVA::Rect& GetLandscapeRect() const;
	DAVA::uint32 GetItemCount() const;
	const FpsStatItem& GetItem(DAVA::uint32 index) const;

	void Clear();
	DAVA::Rect TranslateRect(const DAVA::Rect& rect, const DAVA::Rect& destPlane) const;
	DAVA::Vector2 TranslatePoint(const DAVA::Vector2& point, const DAVA::Rect& destPlane) const;
};

#endif
