#ifndef __TEXTURE_PROPERTIES_H__
#define __TEXTURE_PROPERTIES_H__

#include "DAVAEngine.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"
#include "QtPropertyBrowser/qtgroupboxpropertybrowser.h"

class QtGroupPropertyManager;
class QtIntPropertyManager;
class QtBoolPropertyManager;
class QtEnumPropertyManager;
class QtStringPropertyManager;

class QtSpinBoxFactory;
class QtCheckBoxFactory;
class QtLineEditFactory;
class QtEnumEditorFactory;

class TextureProperties : public QtGroupBoxPropertyBrowser//QtTreePropertyBrowser
{
	Q_OBJECT

public:
	TextureProperties(QWidget *parent = 0);
	~TextureProperties();

	void setTexture(DAVA::Texture *texture);

signals:
	void formatChangedPVR(bool emptyFormat);
	void formatChangedDXT(bool emptyFormat);

private slots:
	void enumPropertyChanged(QtProperty * property);
	void intPropertyChanged(QtProperty * property);
	void boolPropertyChanged(QtProperty * property);
	void stringPropertyChanged(QtProperty * property);

private:
	QWidget *oneForAllParent;

	QtGroupPropertyManager *propertiesGroup;
	QtIntPropertyManager *propertiesInt;
	QtBoolPropertyManager *propertiesBool;
	QtEnumPropertyManager *propertiesEnum;
	QtStringPropertyManager *propertiesString;

	QtSpinBoxFactory *editorInt;
	QtCheckBoxFactory *editorBool;
	QtLineEditFactory *editorString;
	QtEnumEditorFactory *editorEnum;

	QtProperty *enumPVRFormat;
	QtProperty *boolPVRFlipVertical;

	QtProperty *enumDXTFormat;

	QtProperty *enumWrapModeS;
	QtProperty *enumWrapModeT;
	QtProperty *intBaseMipmapLevel;
};

#endif // __TEXTURE_PROPERTIES_H__
