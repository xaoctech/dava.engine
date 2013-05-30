/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/GlobalEnum.h"
#include "Render/TextureDescriptor.h"

#include "TextureBrowser/TextureProperties.h"
#include "QtPropertyEditor/QtPropertyItem.h"

TextureProperties::TextureProperties( QWidget *parent /*= 0*/ )
	: QtPropertyEditor(parent)
	, curTextureDescriptor(NULL)
{ }

TextureProperties::~TextureProperties()
{ }

void TextureProperties::setTextureDescriptor(DAVA::TextureDescriptor *descriptor)
{
	// TODO:
	// Save();

	DAVA::SafeRelease(curTextureDescriptor);

	curTextureDescriptor = DAVA::SafeRetain(descriptor);
	origImageSize = QSize(0, 0);

	if(NULL != curTextureDescriptor)
	{
		// enable this widget
		setEnabled(true);

		// reset mimmap sizes
		// we don't know avaliable mipmap sizes for newly set texture until setOriginalSize() method will be called by user
		MipMapSizesReset();

		// reload all properties for current gpu and from current descriptor 
		ReloadProperties();
	}
	else
	{
		// no texture - disable this widget
		setEnabled(false);
	}
}

void TextureProperties::setTextureGPU(DAVA::eGPUFamily gpu)
{
	if(curGPU != gpu)
	{
		curGPU = gpu;
		ReloadProperties();
	}
}

void TextureProperties::setOriginalImageSize(const QSize &size)
{
	origImageSize = size;

	// Init mimmap sizes based on original image size
	MipMapSizesInit(size.width(), size.height());
}

const DAVA::TextureDescriptor* TextureProperties::getTextureDescriptor()
{
	return curTextureDescriptor;
}

void TextureProperties::MipMapSizesInit(int baseWidth, int baseHeight)
{
	int level = 0;

	MipMapSizesReset();
	while(baseWidth > 1 && baseHeight > 1)
	{
		QSize size(baseWidth, baseHeight);
		QString shownKey;

		shownKey.sprintf("%d - %dx%d", level, baseWidth, baseHeight);
		enumSizes.Register(level, shownKey.toAscii());

		level++;
		baseWidth = baseWidth >> 1;
		baseHeight = baseHeight >> 1;
	}
}

void TextureProperties::MipMapSizesReset()
{
	enumSizes.UnregistelAll();
}

void TextureProperties::ReloadProperties()
{
	RemovePropertyAll();

	propMipMap = NULL;
	propWrapModeS = NULL;
	propWrapModeT = NULL;
	propMinFilter = NULL;
	propMagFilter = NULL;
	propFormat = NULL;
	propSizes = NULL;

	if(NULL != curTextureDescriptor &&
		curGPU >= 0 &&
		curGPU < DAVA::GPU_FAMILY_COUNT)
	{
		QtPropertyItem *header;
		DAVA::BaseObject *object;

		object = &curTextureDescriptor->settings;

		// add common texture settings
		header = AddHeader("Texture settings");
		propMipMap = AddPropertyItem("generateMipMaps", object, header);
		propMipMap->SetFlags(QtPropertyData::FLAG_IS_CHECKABLE | QtPropertyData::FLAG_IS_NOT_EDITABLE);
		propWrapModeS = AddPropertyItem("wrapModeS", object, header);
		propWrapModeT = AddPropertyItem("wrapModeT", object, header);
		propMinFilter = AddPropertyItem("minFilter", object, header);
		propMagFilter = AddPropertyItem("magFilter", object, header);

		object = &curTextureDescriptor->compression[curGPU];

		// add per-gpu settings
		header = AddHeader(GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curGPU));
		propFormat = AddPropertyItem("format", object, header);

		QObject::connect(propMipMap, SIGNAL(ValueChanged()), this, SLOT(PropMipMapChanged()));

		MipMapSizesReset();
		ReloadEnumFormats();
		ReloadEnumWrap();
		ReloadEnumFilters();

		SetPropertyItemValidValues(propWrapModeS, &enumWpar);
		SetPropertyItemValidValues(propWrapModeT, &enumWpar);
		SetPropertyItemValidValues(propMinFilter, &enumFiltersMin);
		SetPropertyItemValidValues(propMagFilter, &enumFiltersMag);
		SetPropertyItemValidValues(propFormat, &enumFormats);

		expandAll();
	}
}

void TextureProperties::ReloadEnumFormats()
{
	const EnumMap *globalFormats = GlobalEnumMap<DAVA::PixelFormat>::Instance();

	enumFormats.UnregistelAll();
	switch(curGPU)
	{
	case DAVA::GPU_POVERVR_IOS:
		enumFormats.Register(DAVA::FORMAT_RGBA8888, globalFormats->ToString(DAVA::FORMAT_RGBA8888));
		enumFormats.Register(DAVA::FORMAT_RGBA5551, globalFormats->ToString(DAVA::FORMAT_RGBA5551));
		enumFormats.Register(DAVA::FORMAT_RGBA4444, globalFormats->ToString(DAVA::FORMAT_RGBA4444));
		enumFormats.Register(DAVA::FORMAT_RGB565, globalFormats->ToString(DAVA::FORMAT_RGB565));
		enumFormats.Register(DAVA::FORMAT_A8, globalFormats->ToString(DAVA::FORMAT_A8));
		enumFormats.Register(DAVA::FORMAT_PVR4, globalFormats->ToString(DAVA::FORMAT_PVR4));
		enumFormats.Register(DAVA::FORMAT_PVR2, globalFormats->ToString(DAVA::FORMAT_PVR2));
		enumFormats.Register(DAVA::FORMAT_ETC1, globalFormats->ToString(DAVA::FORMAT_ETC1));
		break;
	case DAVA::GPU_POVERVR_ANDROID:
		enumFormats.Register(DAVA::FORMAT_RGBA8888, globalFormats->ToString(DAVA::FORMAT_RGBA8888));
		enumFormats.Register(DAVA::FORMAT_DXT1, globalFormats->ToString(DAVA::FORMAT_DXT1));
		enumFormats.Register(DAVA::FORMAT_DXT1NM, globalFormats->ToString(DAVA::FORMAT_DXT1NM));
		enumFormats.Register(DAVA::FORMAT_DXT1A, globalFormats->ToString(DAVA::FORMAT_DXT1A));
		enumFormats.Register(DAVA::FORMAT_DXT3, globalFormats->ToString(DAVA::FORMAT_DXT3));
		enumFormats.Register(DAVA::FORMAT_DXT5, globalFormats->ToString(DAVA::FORMAT_DXT5));
		enumFormats.Register(DAVA::FORMAT_ATC_RGB, globalFormats->ToString(DAVA::FORMAT_ATC_RGB));
		enumFormats.Register(DAVA::FORMAT_ATC_RGBA_EXPLICIT_ALPHA, globalFormats->ToString(DAVA::FORMAT_ATC_RGBA_EXPLICIT_ALPHA));
		enumFormats.Register(DAVA::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, globalFormats->ToString(DAVA::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA));
		break;
	case DAVA::GPU_TEGRA:
		enumFormats.Register(DAVA::FORMAT_RGBA8888, globalFormats->ToString(DAVA::FORMAT_RGBA8888));
		break;
	case DAVA::GPU_MALI:
		enumFormats.Register(DAVA::FORMAT_RGBA8888, globalFormats->ToString(DAVA::FORMAT_RGBA8888));
		break;
	case DAVA::GPU_ADRENO:
		enumFormats.Register(DAVA::FORMAT_RGBA8888, globalFormats->ToString(DAVA::FORMAT_RGBA8888));
		break;
	default:
		break;
	}
}

void TextureProperties::ReloadEnumFilters()
{
	const EnumMap *globalFormats = GlobalEnumMap<DAVA::Texture::TextureFilter>::Instance();

	enumFiltersMag.UnregistelAll();
	enumFiltersMin.UnregistelAll();

	// Mag
	enumFiltersMag.Register(DAVA::Texture::FILTER_NEAREST, globalFormats->ToString(DAVA::Texture::FILTER_NEAREST));
	enumFiltersMag.Register(DAVA::Texture::FILTER_LINEAR, globalFormats->ToString(DAVA::Texture::FILTER_LINEAR));

	// Min
	enumFiltersMin.Register(DAVA::Texture::FILTER_NEAREST, globalFormats->ToString(DAVA::Texture::FILTER_NEAREST));
	enumFiltersMin.Register(DAVA::Texture::FILTER_LINEAR, globalFormats->ToString(DAVA::Texture::FILTER_LINEAR));

	if(NULL != propMipMap)
	{
		if(propMipMap->GetValue().toBool())
		{
			enumFiltersMin.Register(DAVA::Texture::FILTER_NEAREST_MIPMAP_NEAREST, globalFormats->ToString(DAVA::Texture::FILTER_NEAREST_MIPMAP_NEAREST));
			enumFiltersMin.Register(DAVA::Texture::FILTER_LINEAR_MIPMAP_NEAREST, globalFormats->ToString(DAVA::Texture::FILTER_LINEAR_MIPMAP_NEAREST));
			enumFiltersMin.Register(DAVA::Texture::FILTER_NEAREST_MIPMAP_LINEAR, globalFormats->ToString(DAVA::Texture::FILTER_NEAREST_MIPMAP_LINEAR));
			enumFiltersMin.Register(DAVA::Texture::FILTER_LINEAR_MIPMAP_LINEAR, globalFormats->ToString(DAVA::Texture::FILTER_LINEAR_MIPMAP_LINEAR));
		}
		else
		{
			// if mipmap is disabled, min filter can only be nearest or linear
			// if it isn't - change it do default nearest
			int curVal = propMinFilter->GetValue().toInt();
			if( curVal != DAVA::Texture::FILTER_NEAREST ||
				curVal != DAVA::Texture::FILTER_LINEAR)
			{
				propMinFilter->SetValue(QVariant(DAVA::Texture::FILTER_NEAREST));
			}
		}
	}
}

void TextureProperties::ReloadEnumWrap()
{
	const EnumMap *globalFormats = GlobalEnumMap<DAVA::Texture::TextureWrap>::Instance();

	enumWpar.Register(DAVA::Texture::WRAP_REPEAT, globalFormats->ToString(DAVA::Texture::WRAP_REPEAT));
	enumWpar.Register(DAVA::Texture::WRAP_CLAMP_TO_EDGE, globalFormats->ToString(DAVA::Texture::WRAP_CLAMP_TO_EDGE));
}

QtPropertyItem* TextureProperties::AddHeader(const char *text)
{
	QPair<QtPropertyItem*, QtPropertyItem*> propHeader;

	propHeader = AppendProperty(text, NULL);

	QFont boldFont = propHeader.first->font();
	boldFont.setBold(true);

	propHeader.first->setFont(boldFont);
	propHeader.first->setBackground(QBrush(QColor(Qt::lightGray)));
	propHeader.second->setBackground(QBrush(QColor(Qt::lightGray)));

	return propHeader.first;
}

QtPropertyDataMetaObject* TextureProperties::AddPropertyItem(const char *name, DAVA::BaseObject *object, QtPropertyItem *parent)
{
	QtPropertyDataMetaObject* ret = NULL;
	const DAVA::InspInfo* info = object->GetTypeInfo();

	if(NULL != info)
	{
		const DAVA::InspMember *member = info->Member(name);
		if(NULL != member)
		{
			ret = new QtPropertyDataMetaObject(member->Data(object), member->Type());
			AppendProperty(member->Name(), ret, parent);
		}
	}

	return ret;
}

void TextureProperties::SetPropertyItemValidValues(QtPropertyDataMetaObject* item, EnumMap *validValues)
{
	if(NULL != item && NULL != validValues)
	{
		for(int i = 0; i < validValues->GetCount(); ++i)
		{
			int v;

			if(validValues->GetValue(i, v))
			{
				item->AddAllowedValue(DAVA::VariantType(v), validValues->ToString(v));
			}
		}
	}
}

void TextureProperties::PropMipMapChanged()
{
	ReloadEnumFilters();
}