#include "LandscapeTestData.h"

class FpsStatItemCompare
{
public:
	bool operator()(const FpsStatItem& a, const FpsStatItem& b)
	{
		return a.minFps < b.minFps;
	}
};

void LandscapeTestData::AddStatItem(const FpsStatItem &item)
{
	stat.push_back(item);
	std::sort(stat.begin(), stat.end(), FpsStatItemCompare());
}

void LandscapeTestData::SetLandscapeRect(const Rect &rect)
{
	landscapeRect = rect;
}

const Rect& LandscapeTestData::GetLandscapeRect() const
{
	return landscapeRect;
}

uint32 LandscapeTestData::GetItemCount() const
{
	return stat.size();
}

const FpsStatItem& LandscapeTestData::GetItem(uint32 index) const
{
	DVASSERT(index < stat.size());
	return stat[index];
}

void LandscapeTestData::Clear()
{
	stat.clear();
}

Rect LandscapeTestData::TranslateRect(const Rect &rect, const Rect& destPlane) const
{
	DVASSERT(landscapeRect.dx != 0 && landscapeRect.dy !=0);
	
	Vector2 origPlaneSize = landscapeRect.GetSize();
	Vector2 destPlaneSize = destPlane.GetSize();

	Vector2 scale(destPlaneSize.x / origPlaneSize.x,
				   destPlaneSize.y / origPlaneSize.y);

	Vector2 relPos = rect.GetPosition() - landscapeRect.GetPosition();

	Vector2 newRelPos(relPos.x * scale.x,
					  relPos.y * scale.y);

	Vector2 newPos = newRelPos + destPlane.GetPosition();

	Vector2 newSize(rect.GetSize().x * scale.x,
					rect.GetSize().y * scale.y);

	return Rect(newPos, newSize);
}

Vector2 LandscapeTestData::TranslatePoint(const Vector2& point, const DAVA::Rect& destPlane) const
{
	DVASSERT(landscapeRect.dx != 0 && landscapeRect.dy !=0);
	
	Vector2 origPlaneSize = landscapeRect.GetSize();
	Vector2 destPlaneSize = destPlane.GetSize();

	Vector2 scale(destPlaneSize.x / origPlaneSize.x,
				  destPlaneSize.y / origPlaneSize.y);
	
	Vector2 relPos = point - landscapeRect.GetPosition();

	Vector2 newRelPos(relPos.x * scale.x,
					  relPos.y * scale.y);

	Vector2 newPos = newRelPos + destPlane.GetPosition();

	return newPos;
}

void LandscapeTestData::SetTextureMemorySize(uint32 size)
{
	textureMemorySize = size;
}

void LandscapeTestData::SetSceneFilePath(const FilePath & path)
{
	sceneFilePath = path;
}

uint32 LandscapeTestData::GetTextureMemorySize() const
{
	return textureMemorySize;
}

const FilePath & LandscapeTestData::GetSceneFilePath() const
{
	return sceneFilePath;
}

void LandscapeTestData::SetTexturesFilesSize(uint32 size)
{
	textureFilesSize = size;
}

uint32 LandscapeTestData::GetTexturesFilesSize() const
{
	return textureFilesSize;
}