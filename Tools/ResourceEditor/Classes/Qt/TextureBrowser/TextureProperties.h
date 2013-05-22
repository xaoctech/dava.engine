/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __TEXTURE_PROPERTIES_H__
#define __TEXTURE_PROPERTIES_H__

#include "DAVAEngine.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"
#include "QtPropertyBrowser/qtgroupboxpropertybrowser.h"

#include <QMap>
#include <QSize>

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

	typedef enum PropertiesType
	{
		TYPE_COMMON,
		TYPE_COMMON_MIPMAP,
		TYPE_PVR,
		TYPE_DXT
	} PropertiesType;

	void setTextureDescriptor(/*DAVA::Texture *texture, */DAVA::TextureDescriptor *descriptor);
	void setOriginalImageSize(const QSize &size);

	//const DAVA::Texture* getTexture();
	const DAVA::TextureDescriptor* getTextureDescriptor();

public slots:
	void resetCommonProp();

signals:
	void propertyChanged(const int propGroup);

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
	enumPropertiesHelper<int> helperMinGLModesWithMipmap;
	enumPropertiesHelper<QSize> helperMipMapSizes;

	DAVA::TextureDescriptor *curTextureDescriptor;
	QSize origImageSize;

	bool texturePropertiesChanged;
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

	void MinFilterCustomSetup();
	void MipMapSizesInit(int baseWidth, int baseHeight);
	void MipMapSizesReset();
};

#endif // __TEXTURE_PROPERTIES_H__
