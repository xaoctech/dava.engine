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
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material/ShaderParser.h"
#include "Debug/DVAssert.h"

namespace DAVA
{


bool Parser::Load(const String & filename)
{
    File * fp = File::Create(pathName, File::OPEN|File::READ);
	if (!fp)
	{
		Logger::Error("Failed to open PVR texture: %s", pathName.c_str());
		return 0;
	}
	textFileInitialSize = fp->GetSize();
	textFile = new char[fileSize];
	uint32 dataRead = fp->Read(textFile, textFileInitialSize);
    DVASSERT(dataRead == textFileInitialSize);
    
    Preprocess();
    
    
    SafeDeleteArray(textFile);
    SafeRelease(fp);
    
}
    
char * Parser::Preprocess()
{
    uint32 resultBufferSize;
    char * resultBuffer = new char[textFileInitialSize * 2];
    
}

bool Parser::IsDefined(const char * define)
{
    
}
    
void Parser::AddDefine(const char * define)
{
    
}
    
void Parser::AddDefine(const char * define, const char * value)
{
    
}

int32 Parser::SkipSpaces(const char * stream)
{
    const char * s = stream;
    while(*s && strchr(" \t\r\n"))s++;
    return (int32)(stream - s);
}
    
int32 Parser::ReadToken(const char * stream, String & token)
{
    const char * s = stream;
    s += SkipSpaces(stream);
    while(*s && !strchr(" \t\r\n"))
    {
        token += *s++;
    }
    return (int32)(s - stream);
}
    
int32 ReadDigit(const char * stream, String & digit)
{
    
}
    
int32 ExpectSymbol(const char * stream, const char symbol)
{
    
}




};

#endif // __DAVAENGINE_MATERIAL_GRAPH_H__

