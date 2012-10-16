/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "TextureDescriptorTest.h"
#include "TextureUtils.h"
#include "Render/TextureDescriptor.h"

TextureDescriptorTest::TextureDescriptorTest()
: TestTemplate<TextureDescriptorTest>("TextureDescriptorTest")
{
    String testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/TextureDescriptorTest/Text");
    FileSystem::Instance()->CreateDirectory(testFolder, true);
    testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/TextureDescriptorTest/Bin");
    FileSystem::Instance()->CreateDirectory(testFolder, true);

    RegisterFunction(this, &TextureDescriptorTest::LoadPngFromTextDescriptor, String("LoadPngFromTextDescriptor"), NULL);
    RegisterFunction(this, &TextureDescriptorTest::LoadPngFromBinaryDescriptor, String("LoadPngFromBinaryDescriptor"), NULL);
    RegisterFunction(this, &TextureDescriptorTest::LoadPvrFromBinaryDescriptor, String("LoadPvrFromBinaryDescriptor"), NULL);
}

void TextureDescriptorTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(20);
    font->SetColor(Color::White());

    SafeRelease(font);
}


void TextureDescriptorTest::UnloadResources()
{
    RemoveAllControls();
}


void TextureDescriptorTest::LoadPngFromTextDescriptor(TestTemplate<TextureDescriptorTest>::PerfFuncData *data)
{
    String sourcePathname = String("~res:/TestData/TextureDescriptorTest/source.png");
    String descriptorPathname = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/TextureDescriptorTest/Text/PNG.tex");
    
#if !defined TEXTURE_SPLICING_ENABLED
    String destinationPathname = FileSystem::Instance()->ReplaceExtension(descriptorPathname, ".png");
    FileSystem::Instance()->CopyFile(sourcePathname, destinationPathname);
#endif //#if !defined TEXTURE_SPLICING_ENABLED
    
    //Create descriptor
    TextureDescriptor *descriptor = Texture::CreateDescriptorForTexture(sourcePathname);
    descriptor->SaveAsText(descriptorPathname);
    SafeRelease(descriptor);
    
    Compare(sourcePathname, descriptorPathname, data, FORMAT_RGBA8888);
}

void TextureDescriptorTest::LoadPngFromBinaryDescriptor(TestTemplate<TextureDescriptorTest>::PerfFuncData *data)
{
    String sourcePathname = String("~res:/TestData/TextureDescriptorTest/source.png");
    String descriptorPathname = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/TextureDescriptorTest/Bin/PNG.tex");
    
#if !defined TEXTURE_SPLICING_ENABLED
    String destinationPathname = FileSystem::Instance()->ReplaceExtension(descriptorPathname, ".png");
    FileSystem::Instance()->CopyFile(sourcePathname, destinationPathname);
#endif //#if !defined TEXTURE_SPLICING_ENABLED
    
    //Create descriptor
    TextureDescriptor *descriptor = Texture::CreateDescriptorForTexture(sourcePathname);
    descriptor->textureFileFormat = TextureDescriptor::PNG_FILE;
    
    //Safe as Binary
    descriptor->SaveAsBinary(descriptorPathname, sourcePathname);
    SafeRelease(descriptor);

    Compare(sourcePathname, descriptorPathname, data, FORMAT_RGBA8888);
}

void TextureDescriptorTest::LoadPvrFromBinaryDescriptor(TestTemplate<TextureDescriptorTest>::PerfFuncData *data)
{
    String sourcePathname = String("~res:/TestData/TextureDescriptorTest/source.pvr");
    String descriptorPathname = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/TextureDescriptorTest/Bin/PVR.tex");
    
#if !defined TEXTURE_SPLICING_ENABLED
    String destinationPathname = FileSystem::Instance()->ReplaceExtension(descriptorPathname, ".pvr");
    FileSystem::Instance()->CopyFile(sourcePathname, destinationPathname);
#endif //#if !defined TEXTURE_SPLICING_ENABLED
    
    //Create descriptor
    TextureDescriptor *descriptor = Texture::CreateDescriptorForTexture(sourcePathname);
    descriptor->textureFileFormat = TextureDescriptor::PVR_FILE;
    
    //Safe as Binary
    descriptor->SaveAsBinary(descriptorPathname, sourcePathname);
    SafeRelease(descriptor);

    Compare(sourcePathname, descriptorPathname, data, FORMAT_PVR2);
}

void TextureDescriptorTest::Compare(const String &sourcePathname, const String &descriptorPathname, PerfFuncData * data, PixelFormat format)
{
    //Compare Textures
    Sprite *sourceTexture = TextureUtils::CreateSpriteFromTexture(sourcePathname);
    Sprite *exportedTexture = TextureUtils::CreateSpriteFromTexture(descriptorPathname);

    TextureUtils::CompareResult result = TextureUtils::CompareSprites(sourceTexture, exportedTexture, format);
    float32 differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
    
    data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%",
                                    result.difference, differencePersentage, 100.f - differencePersentage);
    
    Logger::Debug((data->testData.name + String(" ") + data->testData.message).c_str());
    
    TEST_VERIFY(0 == differencePersentage);
    
    SafeRelease(sourceTexture);
    SafeRelease(exportedTexture);
}


void TextureDescriptorTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    TestTemplate<TextureDescriptorTest>::Draw(geometricData);
}




