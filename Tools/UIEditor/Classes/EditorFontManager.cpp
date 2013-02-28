//
//  FontManager.cpp
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//


#include "EditorFontManager.h"

EditorFontManager::EditorFontManager()
{
	defaultFont = NULL;
	Init();
}

EditorFontManager::~EditorFontManager()
{
	Reset();
}

void EditorFontManager::Init()
{
	defaultFont = LoadFont("~res:/Fonts/MyriadPro-Regular.otf", "MyriadPro-Regular.otf");
}

void EditorFontManager::Reset()
{
	defaultFont = NULL;
	
	for (FONTSMAP::iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
	{
		Font* font = iter->second;
		SafeRelease(font);
	}
}

Font* EditorFontManager::LoadFont(const String& fontPath, const String& fontName)
{
	Font* font = GetFont(fontName);
	if (font)
		return font;
	
	font = FTFont::Create(fontPath);
	if (font)
	{
		font->SetSize(12.f);
		font->SetColor(Color(1,1,1,1));
		
		fonts[fontName] = font;
        //If font was successfully loaded - emit the signal 
        emit FontLoaded();
	}
	
	return font;
}

Font* EditorFontManager::GetDefaultFont() const
{
	return defaultFont;
}

Font* EditorFontManager::GetFont(const String& name) const
{
	FONTSMAP::const_iterator iter = fonts.find(name);
	if (iter != fonts.end())
	{
		return iter->second;
	}
	return NULL;
}

const EditorFontManager::FONTSMAP& EditorFontManager::GetAllFonts() const
{
	return fonts;
}

QString EditorFontManager::GetFontName(Font* font) const
{
    if (font == NULL)
    {
        return QString();
    }

    EditorFontManager::FONTSMAP fonts = GetAllFonts();
    for (EditorFontManager::FONTSMAP::const_iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
    {
        if (font->IsEqual(iter->second))
        {
            return iter->first.c_str();
        }
    }

    return QString();
}
