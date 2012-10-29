#ifndef __TEXTURE_PROPERTIES_H__
#define __TEXTURE_PROPERTIES_H__

#include "DAVAEngine.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"
#include "QtPropertyBrowser/qtgroupboxpropertybrowser.h"

#include <QMap>

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

	DAVA::Texture* getTexture();
	DAVA::TextureDescriptor* getTextureDescriptor();

signals:
	void formatChangedPVR(const DAVA::PixelFormat &newFormat);
	void formatChangedDXT(const DAVA::PixelFormat &newFormat);

private slots:
	void propertyChanged(QtProperty * property);

private:
	struct enumPropertiesHelper
	{
		int value(const QString &key);
		int indexK(const QString &key);
		int indexV(const int &value);

		void push_back(const QString &key, const int &value);
		QStringList keyList();

	private:
		QVector<QString> keys;
		QVector<int> values;
	};

	QWidget *oneForAllParent;

	enumPropertiesHelper helperPVRFormats;
	enumPropertiesHelper helperDXTFormats;
	enumPropertiesHelper helperWrapModes;

	DAVA::Texture *curTexture;
	DAVA::TextureDescriptor *curTextureDescriptor;

	bool reactOnPropertyChange;

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
	QtProperty *intBasePVRMipmapLevel;

	QtProperty *enumDXTFormat;
	QtProperty *intBaseDXTMipmapLevel;

	QtProperty *boolGenerateMipMaps;
	QtProperty *enumWrapModeS;
	QtProperty *enumWrapModeT;

	void Save();
};

#endif // __TEXTURE_PROPERTIES_H__
