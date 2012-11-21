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

	void setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor);

	const DAVA::Texture* getTexture();
	const DAVA::TextureDescriptor* getTextureDescriptor();

signals:
	void propertyChanged();

private slots:
	void propertyChanged(QtProperty * property);

private:
	template<typename T>
	struct enumPropertiesHelper
	{
		T value(const QString &key)
		{
			int i = keys.indexOf(key);

			if(i != -1)
			{
				return values[i];
			}

			return T();
		}

		void push_back(const QString &key, const T &value)
		{
			keys.push_back(key);
			values.push_back(value);
		}

		void clear()
		{
			keys.clear();
			values.clear();
		}

		int indexK(const QString &key)
		{
			return keys.indexOf(key);
		}

		int indexV(const T &value)
		{
			return values.indexOf(value);
		}

		QStringList keyList()
		{
			QStringList ret;

			for(int i = 0; i < keys.count(); ++i)
			{
				ret.append(keys[i]);
			}

			return ret;
		}

	private:
		QVector<QString> keys;
		QVector<T> values;
	};

	QWidget *oneForAllParent;

	enumPropertiesHelper<int> helperPVRFormats;
	enumPropertiesHelper<int> helperDXTFormats;
	enumPropertiesHelper<int> helperWrapModes;
	enumPropertiesHelper<int> helperMinGLModes;
	enumPropertiesHelper<int> helperMagGLModes;
	enumPropertiesHelper<QSize> helperMipMapSizes;

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
	QtProperty *enumBasePVRMipmapLevel;

	QtProperty *enumDXTFormat;
	QtProperty *enumBaseDXTMipmapLevel;

	QtProperty *boolGenerateMipMaps;
	QtProperty *enumWrapModeS;
	QtProperty *enumWrapModeT;
	QtProperty *enumMinGL;
	QtProperty *enumMagGL;

	void Save();


	void InitMipMapSizes(int baseWidth, int baseHeight);
	/*
	int GetBaseSizeIndex(QtProperty *enumPropetie, int baseWidth, int baseHeight);
	QSize GetBaseSize(QtProperty *enumPropetie, int index);
	*/
};

#endif // __TEXTURE_PROPERTIES_H__
