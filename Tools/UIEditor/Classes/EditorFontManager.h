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
    
	Font* GetFont(const String& name) const;
    QString GetFontName(Font* font) const;

	const FONTSMAP& GetAllFonts() const;
    
signals:
    void FontLoaded();
	
private:
	void Init();
	
private:
	Font* defaultFont;
	FONTSMAP fonts; 

};

#endif /* defined(__UIEditor__FontManager__) */
