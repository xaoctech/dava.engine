#include "TextureDialog/TextureProperties.h"
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "QtPropertyBrowser/qteditorfactory.h"

TextureProperties::TextureProperties(QWidget *parent /* = 0 */)
	: QtGroupBoxPropertyBrowser(parent)//QtTreePropertyBrowser(parent)
{
	QStringList listPVRFormats;
	QStringList listDXTFormats;
	QStringList listWrapModes;

	listPVRFormats.append("None");
	listPVRFormats.append("format 1");
	listPVRFormats.append("format 2");

	listDXTFormats.append("None");
	listDXTFormats.append("format 3");
	listDXTFormats.append("format 4");
	listDXTFormats.append("format 5");

	listWrapModes.append("Clamp");
	listWrapModes.append("Repeat");

	oneForAllParent = new QWidget();
	
	propertiesGroup = new QtGroupPropertyManager(oneForAllParent);

	propertiesInt = new QtIntPropertyManager(oneForAllParent);
	propertiesBool = new QtBoolPropertyManager(oneForAllParent);
	propertiesEnum = new QtEnumPropertyManager(oneForAllParent);
	propertiesString = new QtStringPropertyManager(oneForAllParent);

	editorInt = new QtSpinBoxFactory(oneForAllParent);
	editorBool = new QtCheckBoxFactory(oneForAllParent);
	editorString = new QtLineEditFactory(oneForAllParent);
	editorEnum = new QtEnumEditorFactory(oneForAllParent);

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
	propertiesEnum->setEnumNames(enumPVRFormat, listPVRFormats);

	boolPVRFlipVertical = propertiesBool->addProperty("Flip vertical");

	groupPVR->addSubProperty(enumPVRFormat);
	groupPVR->addSubProperty(boolPVRFlipVertical);
	addProperty(groupPVR);

	// DXT group
	enumDXTFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumDXTFormat, listDXTFormats);

	groupDXT->addSubProperty(enumDXTFormat);
	addProperty(groupDXT);

	// Common group

	// wrapmode t
	enumWrapModeS = propertiesEnum->addProperty("Wrap mode S");
	propertiesEnum->setEnumNames(enumWrapModeS, listWrapModes);
	groupCommon->addSubProperty(enumWrapModeS);

	// wrapmode s
	enumWrapModeT = propertiesEnum->addProperty("Wrap mode T");
	propertiesEnum->setEnumNames(enumWrapModeT, listWrapModes);
	groupCommon->addSubProperty(enumWrapModeT);

	// base mipmap
	intBaseMipmapLevel = propertiesInt->addProperty("Base Mipmap level");
	groupCommon->addSubProperty(intBaseMipmapLevel);

	addProperty(groupCommon);

	QObject::connect(propertiesEnum, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(enumPropertyChanged(QtProperty *)));
	QObject::connect(propertiesInt, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(intPropertyChanged(QtProperty *)));
	QObject::connect(propertiesBool, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(boolPropertyChanged(QtProperty *)));
	QObject::connect(propertiesString, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(stringPropertyChanged(QtProperty *)));
}

TextureProperties::~TextureProperties()
{
	delete oneForAllParent;
}

void TextureProperties::setTexture(DAVA::Texture *texture)
{

}

void TextureProperties::enumPropertyChanged(QtProperty * property)
{
	if(property == enumPVRFormat)
	{
		bool emptyFormat = ("None" == enumPVRFormat->valueText());
		emit formatChangedPVR(emptyFormat);
	}
	else if(property == enumDXTFormat)
	{
		bool emptyFormat = ("None" == enumDXTFormat->valueText());
		emit formatChangedDXT(emptyFormat);
	}
}

void TextureProperties::boolPropertyChanged(QtProperty * property)
{

}

void TextureProperties::intPropertyChanged(QtProperty * property)
{

}

void TextureProperties::stringPropertyChanged(QtProperty * property)
{

}
