//
//  FontManager.h
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//

#ifndef __UIEditor__FontManager__
#define __UIEditor__FontManager__

#include <iostream>
#include <QObject>
#include <DAVAEngine.h>
using namespace DAVA;

class EditorFontManager: public QObject, public Singleton<EditorFontManager>
{
    Q_OBJECT
public:
	typedef std::map<String, Font*> FONTSMAP;	
	EditorFontManager();
	~EditorFontManager();
	
	void Reset();

	Font* LoadFont(const String& fontPath, const String& fontName);
	
	Font* GetDefaultFont() const;
	void SetDefaultFont(Font* font);
    
	Font* GetFont(const String& name) const;
    QString GetFontName(Font* font) const;

	const FONTSMAP& GetAllFonts() const;
	
	struct DefaultFontPath
	{
		String fontPath;
		String fontSpritePath;
		
		DefaultFontPath(String fontPath, String fontSpritePath)
		{
			this->fontPath = fontPath;
			this->fontSpritePath = fontSpritePath;
		}
	};
	
	DefaultFontPath GetDefaultFontPath();
	void InitDefaultFontFromPath(const DefaultFontPath& defaultFontPath);
	QString GetDefaultFontName() const;
    
signals:
    void FontLoaded();
	
private:
	void Init();
	
private:
	Font* defaultFont;
	FONTSMAP fonts; 

};

#endif /* defined(__UIEditor__FontManager__) */
