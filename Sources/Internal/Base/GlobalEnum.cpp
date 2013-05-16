#include "Base/GlobalEnum.h"

#include "Render/Texture.h"

using namespace DAVA;

ENUM_DECLARE(Texture::TextureWrap)
{
	ENUM_ADD(Texture::WRAP_CLAMP_TO_EDGE);
}

ENUM_DECLARE(Texture::TextureFilter)
{
	ENUM_ADD(Texture::FILTER_LINEAR);
	ENUM_ADD(Texture::FILTER_NEAREST);
}

/*
void f()
{
}

GlobalEnum *globalEnum = GlobalEnum::Instance();
f();
*/
/*
->Add(DAVA::MetaInfo::Instance<DAVA::Texture::TextureWrap>(), DAVA::Texture::WRAP_CLAMP_TO_EDGE, "WRAP_CLAMP_TO_EDGE");

ENUM_ADD(DAVA::Texture::TextureWrap, DAVA::Texture::WRAP_CLAMP_TO_EDGE, "WRAP_CLAMP_TO_EDGE");
ENUM_ADD(DAVA::Texture::TextureWrap, DAVA::Texture::WRAP_REPEAT, "WRAP_REPEAT");
*/
