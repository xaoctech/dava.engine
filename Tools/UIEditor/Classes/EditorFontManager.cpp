//
//  FontManager.cpp
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//


#include "EditorFontManager.h"
#include "StringUtils.h"

static const String DEFAULT_FONT_NAME = "MyriadPro-Regular.otf";
static const String DEFAULT_FONT_PATH = "~res:/Fonts/MyriadPro-Regular.otf";

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
	defaultFont = LoadFont(DEFAULT_FONT_PATH, DEFAULT_FONT_NAME);
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

void EditorFontManager::SetDefaultFont(Font *font)
{
	defaultFont = font->Clone();
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

EditorFontManager::DefaultFontPath EditorFontManager::GetDefaultFontPath()
{
	String defFontPath;
	String defFontSpritePath;

	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();		
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
				String ftFontPath = ftFont->GetFontPath();
				// Don't save standart default font
				if (ftFontPath.find(DEFAULT_FONT_NAME) == String::npos)
				{
					// Set font path
					defFontPath = ftFontPath;
				}
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
                // Try to get font sprite
                Sprite *fontSprite = gFont->GetFontSprite();
				// Save font only if sprite is available
                if (fontSprite)
                {
					// Set font definition and sprite relative path
					defFontPath = gFont->GetFontDefinitionName();
					defFontSpritePath = fontSprite->GetName();
                }
				break;
            }
        }	
	}
	
	return DefaultFontPath(defFontPath, defFontSpritePath);
}

void EditorFontManager::InitDefaultFontFromPath(const EditorFontManager::DefaultFontPath& defaultFontPath)
{
	String fontPath = defaultFontPath.fontPath;
	String fontSpritePath = defaultFontPath.fontSpritePath;
	Font* loadedFont = NULL;
	// Create font from loaded paths
	if (!fontPath.empty())
	{
		// Grpahics font
		if (!fontSpritePath.empty())
		{
			loadedFont = GraphicsFont::Create(fontPath, TruncateTxtFileExtension(fontSpritePath));
		}
		else // True type font
		{
			loadedFont = FTFont::Create(fontPath);
		}
	}
	// Set default font only if font was really created
	if (loadedFont)
	{
		// TODO: We don't have font color property for now.
        // Initialize created font with white color

		// Reset default font
		if (defaultFont)
		{
			defaultFont = NULL;
		}
		defaultFont = loadedFont;
	}	
}

QString EditorFontManager::GetDefaultFontName() const
{		
	Font::eFontType fontType = defaultFont->GetFontType();
    switch (fontType)
    {
    	case Font::TYPE_FT:
        {
        	FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
			return QString::fromStdString(ftFont->GetFontPath());
        }
		case Font::TYPE_GRAPHICAL:
        {
        	GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
			return QString::fromStdString(gFont->GetFontDefinitionName());
		}
	}
	return QString::fromStdString(DEFAULT_FONT_PATH);
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
