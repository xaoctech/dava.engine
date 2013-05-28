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
#include "QtPropertyEditor/QtProperyData/QtPropertyDataMetaObject.h"

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
		reloadProperties();

		/*
		// set loaded descriptor to current properties
		{
			// pvr
			QSize curPVRSize(curTextureDescriptor->pvrCompression.compressToWidth, curTextureDescriptor->pvrCompression.compressToHeight);
			propertiesEnum->setValue(enumPVRFormat, helperPVRFormats.indexV(curTextureDescriptor->pvrCompression.format));
			propertiesEnum->setEnumNames(enumBasePVRMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBasePVRMipmapLevel, helperMipMapSizes.indexV(curPVRSize));

			// dxt
			QSize curDXTSize(curTextureDescriptor->dxtCompression.compressToWidth, curTextureDescriptor->dxtCompression.compressToHeight);
			propertiesEnum->setValue(enumDXTFormat, helperDXTFormats.indexV(curTextureDescriptor->dxtCompression.format));
			propertiesEnum->setEnumNames(enumBaseDXTMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBaseDXTMipmapLevel, helperMipMapSizes.indexV(curDXTSize));

			// mipmap
			propertiesBool->setValue(boolGenerateMipMaps, curTextureDescriptor->generateMipMaps);

			// wrap mode
			propertiesEnum->setValue(enumWrapModeS, helperWrapModes.indexV(curTextureDescriptor->wrapModeS));
			propertiesEnum->setValue(enumWrapModeT, helperWrapModes.indexV(curTextureDescriptor->wrapModeT));

			// min gl filter
			MinFilterCustomSetup();

			// mag lg filter
			propertiesEnum->setValue(enumMagGL, helperMagGLModes.indexV(curTextureDescriptor->magFilter));

		}
		*/
	}
	else
	{
		// no texture - disable this widget
		setEnabled(false);
	}
}

void TextureProperties::setTextureGPU(DAVA::eGPUFamily gpu)
{
	curGPU = gpu;
	reloadProperties();
}

void TextureProperties::setOriginalImageSize(const QSize &size)
{
	origImageSize = size;

	// Init mimmap sizes based on original image size
	MipMapSizesInit(size.width(), size.height());

	if(NULL != curTextureDescriptor)
	{
		// reload values into PVR property
		/*
		QSize curPVRSize(curTextureDescriptor->pvrCompression.compressToWidth, curTextureDescriptor->pvrCompression.compressToHeight);
		propertiesEnum->setEnumNames(enumBasePVRMipmapLevel, helperMipMapSizes.keyList());
		propertiesEnum->setValue(enumBasePVRMipmapLevel, helperMipMapSizes.indexV(curPVRSize));

		// reload values into DXT property
		QSize curDXTSize(curTextureDescriptor->dxtCompression.compressToWidth, curTextureDescriptor->dxtCompression.compressToHeight);
		propertiesEnum->setEnumNames(enumBaseDXTMipmapLevel, helperMipMapSizes.keyList());
		propertiesEnum->setValue(enumBaseDXTMipmapLevel, helperMipMapSizes.indexV(curDXTSize));
		*/
	}
}

const DAVA::TextureDescriptor* TextureProperties::getTextureDescriptor()
{
	return curTextureDescriptor;
}

void TextureProperties::MipMapSizesInit(int baseWidth, int baseHeight)
{
	int level = 0;

	//helperMipMapSizes.clear();
	/*
	while(baseWidth > 1 && baseHeight > 1)
	{
		QSize size(baseWidth, baseHeight);
		QString shownKey;

		shownKey.sprintf("%d - %dx%d", level, baseWidth, baseHeight);
		helperMipMapSizes.push_back(shownKey, size);

		level++;
		baseWidth = baseWidth >> 1;
		baseHeight = baseHeight >> 1;
	}
	*/
}

void TextureProperties::MipMapSizesReset()
{
	//helperMipMapSizes.clear();
}

void TextureProperties::reloadProperties()
{
	RemovePropertyAll();

	if(NULL != curTextureDescriptor &&
		curGPU >= 0 &&
		curGPU < DAVA::GPU_FAMILY_COUNT)
	{

		// add header item for common texture settings
		QPair<QtPropertyItem*, QtPropertyItem*> propHeader = AppendProperty("Texture settings", NULL);

		QFont boldFont = propHeader.first->font();
		boldFont.setBold(true);

		propHeader.first->setFont(boldFont);
		propHeader.first->setBackground(QBrush(QColor(Qt::lightGray)));
		propHeader.second->setBackground(QBrush(QColor(Qt::lightGray)));

		// add common texture settings
		const DAVA::IntrospectionInfo* info = curTextureDescriptor->settings.GetTypeInfo();
		void *object = &curTextureDescriptor->settings;
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);

			QtPropertyDataMetaObject *data = new QtPropertyDataMetaObject(member->Data(object), member->Type());

			if(member->Desc())


			AppendProperty(member->Name(), data, propHeader.first);
		}

		// add header for GPU settings
		propHeader = AppendProperty(GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curGPU), NULL);
		propHeader.first->setFont(boldFont);
		propHeader.first->setBackground(QBrush(QColor(Qt::lightGray)));
		propHeader.second->setBackground(QBrush(QColor(Qt::lightGray)));

		// add per-GPU texture settings
		info = curTextureDescriptor->compression[curGPU].GetTypeInfo();
		object = &curTextureDescriptor->compression[curGPU];
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);

			QtPropertyDataMetaObject *data = new QtPropertyDataMetaObject(member->Data(object), member->Type());

			data->AddAllowedValue(DAVA::VariantType(1), "Value 1");
			data->AddAllowedValue(DAVA::VariantType(2), "Value 2");
			data->AddAllowedValue(DAVA::VariantType(3), "Value 3");
			data->AddAllowedValue(DAVA::VariantType(4), "Value 4");

			AppendProperty(member->Name(), data, propHeader.first);
		}

		expandAll();
	}
}
