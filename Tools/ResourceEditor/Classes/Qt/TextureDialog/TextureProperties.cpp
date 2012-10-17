#include "TextureDialog/TextureProperties.h"
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "QtPropertyBrowser/qteditorfactory.h"

#include "Render/TextureDescriptor.h"

TextureProperties::TextureProperties(QWidget *parent /* = 0 */)
	: QtGroupBoxPropertyBrowser(parent)//QtTreePropertyBrowser(parent)
	, curTexture(NULL)
	, curTextureDescriptor(NULL)
	, reactOnPropertyChange(true)
{
	// initialize list with string for different comboboxs
	{
		helperPVRFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA5551), DAVA::FORMAT_RGBA5551);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA4444), DAVA::FORMAT_RGBA4444);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB888), DAVA::FORMAT_RGB888);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB565), DAVA::FORMAT_RGB565);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A8), DAVA::FORMAT_A8);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A16), DAVA::FORMAT_A16);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR4), DAVA::FORMAT_PVR4);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR2), DAVA::FORMAT_PVR2);


		helperDXTFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA5551), DAVA::FORMAT_RGBA5551);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA4444), DAVA::FORMAT_RGBA4444);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB888), DAVA::FORMAT_RGB888);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB565), DAVA::FORMAT_RGB565);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A8), DAVA::FORMAT_A8);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A16), DAVA::FORMAT_A16);

		helperWrapModes.push_back("Clamp", DAVA::Texture::WRAP_CLAMP_TO_EDGE);
		helperWrapModes.push_back("Repeat", DAVA::Texture::WRAP_REPEAT);
	}

	// parent widget
	oneForAllParent = new QWidget();

	// property managers
	propertiesGroup = new QtGroupPropertyManager(oneForAllParent);
	propertiesInt = new QtIntPropertyManager(oneForAllParent);
	propertiesBool = new QtBoolPropertyManager(oneForAllParent);
	propertiesEnum = new QtEnumPropertyManager(oneForAllParent);
	propertiesString = new QtStringPropertyManager(oneForAllParent);

	// property editors
	editorInt = new QtSpinBoxFactory(oneForAllParent);
	editorBool = new QtCheckBoxFactory(oneForAllParent);
	editorString = new QtLineEditFactory(oneForAllParent);
	editorEnum = new QtEnumEditorFactory(oneForAllParent);

	// setup property managers with appropriate property editors
	setFactoryForManager(propertiesInt, editorInt);
	setFactoryForManager(propertiesBool, editorBool);
	setFactoryForManager(propertiesEnum, editorEnum);
	setFactoryForManager(propertiesString, editorString);

	// Adding properties

	// groups
	QtProperty* groupPVR = propertiesGroup->addProperty("PVR");
	QtProperty* groupDXT = propertiesGroup->addProperty("DXT");
	QtProperty* groupCommon = propertiesGroup->addProperty("Common");

	// PVR group
	enumPVRFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumPVRFormat, helperPVRFormats.keyList());

	boolPVRFlipVertical = propertiesBool->addProperty("Flip vertical");

	intBasePVRMipmapLevel = propertiesInt->addProperty("Base Mipmap level");

	groupPVR->addSubProperty(enumPVRFormat);
	groupPVR->addSubProperty(boolPVRFlipVertical);
	groupPVR->addSubProperty(intBasePVRMipmapLevel);
	addProperty(groupPVR);

	// DXT group
	enumDXTFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumDXTFormat, helperDXTFormats.keyList());

	boolDXTFlipVertical = propertiesBool->addProperty("Flip vertical");

	intBaseDXTMipmapLevel = propertiesInt->addProperty("Base Mipmap level");

	groupDXT->addSubProperty(enumDXTFormat);
	groupDXT->addSubProperty(boolDXTFlipVertical);
	groupDXT->addSubProperty(intBaseDXTMipmapLevel);
	addProperty(groupDXT);

	// Common group

	// Mip maps
	boolGenerateMipMaps = propertiesBool->addProperty("Generate MipMaps");
	groupCommon->addSubProperty(boolGenerateMipMaps);

	// wrapmode t
	enumWrapModeS = propertiesEnum->addProperty("Wrap mode S");
	propertiesEnum->setEnumNames(enumWrapModeS, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeS);

	// wrapmode s
	enumWrapModeT = propertiesEnum->addProperty("Wrap mode T");
	propertiesEnum->setEnumNames(enumWrapModeT, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeT);

	addProperty(groupCommon);

	QObject::connect(propertiesEnum, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesInt, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesBool, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesString, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
}

TextureProperties::~TextureProperties()
{
	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	delete oneForAllParent;
}

void TextureProperties::setTexture(DAVA::Texture *texture)
{
	reactOnPropertyChange = false;

	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	curTexture = texture;
	curTextureDescriptor = DAVA::Texture::CreateDescriptorForTexture(curTexture->GetPathname());

	DAVA::SafeRetain(curTexture);

	// set loaded descriptor to current properties
	{
		// pvr
		propertiesEnum->setValue(enumPVRFormat, helperPVRFormats.indexV(curTextureDescriptor->pvrCompression.format));
		propertiesBool->setValue(boolPVRFlipVertical, curTextureDescriptor->pvrCompression.flipVertically);
		propertiesInt->setValue(intBasePVRMipmapLevel, curTextureDescriptor->pvrCompression.baseMipMapLevel);

		// dxt
		propertiesEnum->setValue(enumDXTFormat, helperDXTFormats.indexV(curTextureDescriptor->dxtCompression.format));
		propertiesBool->setValue(boolDXTFlipVertical, curTextureDescriptor->dxtCompression.flipVertically);
		propertiesInt->setValue(intBaseDXTMipmapLevel, curTextureDescriptor->dxtCompression.baseMipMapLevel);

		// mipmap
		propertiesBool->setValue(boolGenerateMipMaps, curTextureDescriptor->generateMipMaps);

		// wrap mode
		propertiesEnum->setValue(enumWrapModeS, helperWrapModes.indexV(curTextureDescriptor->wrapModeS));
		propertiesEnum->setValue(enumWrapModeT, helperWrapModes.indexV(curTextureDescriptor->wrapModeT));
	}

	reactOnPropertyChange = true;
}

void TextureProperties::propertyChanged(QtProperty * property)
{
	if(reactOnPropertyChange)
	{
		if(property == enumPVRFormat)
		{
			DAVA::PixelFormat newPVRFormat = (DAVA::PixelFormat) helperPVRFormats.value(enumPVRFormat->valueText());
			curTextureDescriptor->pvrCompression.format = newPVRFormat;
			emit formatChangedPVR(newPVRFormat);
		}
		else if(property == enumDXTFormat)
		{
			DAVA::PixelFormat newDXTFormat = (DAVA::PixelFormat) helperDXTFormats.value(enumDXTFormat->valueText());
			curTextureDescriptor->dxtCompression.format = newDXTFormat;
			emit formatChangedDXT(newDXTFormat);
		}
		else if(property == boolPVRFlipVertical)
		{
			curTextureDescriptor->pvrCompression.flipVertically = propertiesBool->value(boolPVRFlipVertical);
		}
		else if(property == boolDXTFlipVertical)
		{
			curTextureDescriptor->dxtCompression.flipVertically = propertiesBool->value(boolDXTFlipVertical);
		}
		else if(property == boolGenerateMipMaps)
		{
			curTextureDescriptor->generateMipMaps = (int) propertiesBool->value(boolGenerateMipMaps);
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeS = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeS->valueText());
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeT = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeT->valueText());
		}

		curTextureDescriptor;
	}
}

void TextureProperties::enumPropertiesHelper::push_back(const QString &key, const int &value)
{
	keys.push_back(key);
	values.push_back(value);
}

int TextureProperties::enumPropertiesHelper::value(const QString &key)
{
	int i = keys.indexOf(key);
	if(i != -1)
	{
		return values[i];
	}

	return 0;
}

int TextureProperties::enumPropertiesHelper::indexK(const QString &key)
{
	return keys.indexOf(key);
}

int TextureProperties::enumPropertiesHelper::indexV(const int &value)
{
	return values.indexOf(value);
}

QStringList TextureProperties::enumPropertiesHelper::keyList()
{
	QStringList ret;

	for(int i = 0; i < keys.count(); ++i)
	{
		ret.append(keys[i]);
	}

	return ret;
}