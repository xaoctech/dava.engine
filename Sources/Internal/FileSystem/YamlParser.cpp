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
#include "FileSystem/YamlParser.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"
#include <sstream>
#include "yaml/yaml.h"
#include "Utils/UTF8Utils.h"
#include "FileSystem/FileSystem.h"
#include "VariantType.h"
#include "KeyedArchive.h"
#include "Utils.h"

namespace DAVA 
{

YamlNode::YamlNode(eType _type)
{
	type = _type;
	mapIndex = 0;
	mapCount = 0;
}

YamlNode::~YamlNode()
{
	for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != objectMap.end(); ++t)
	{
		YamlNode * object = t->second;
		SafeRelease(object);
	}
	objectMap.clear();
	
	int32 size = (int32)objectArray.size();
	for (int32 k = 0; k < size; ++k)
	{
		SafeRelease(objectArray[k]);
	}
	objectArray.clear();
}
	
void YamlNode::Print(int32 identation)
{
	if (type == TYPE_STRING)
	{
		const char * str = nwStringValue.c_str();
		printf("%s", str);
	}else if (type == TYPE_ARRAY)
	{
		printf("[");
		for (int32 k = 0; k < (int32)objectArray.size(); ++k)
		{
			objectArray[k]->Print(identation + 1);
			printf(",");
		}
		printf("]");
	}else if (type == TYPE_MAP)
	{
		printf("{");
		for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != objectMap.end(); ++t)
		{
			printf("key(%s, %d): ", t->first.c_str(), t->second->mapIndex);
			t->second->Print(identation + 1);
			printf(",");
		}
		printf("}");
	}
}

void YamlNode::PrintToFile(DAVA::File* file, uint32 identationDepth)
{
    if (type == TYPE_STRING)
    {
        file->WriteString(nwStringValue);
    }
    else if (type == TYPE_ARRAY)
    {
        file->WriteString("[");
        
        for (int32 k = 0; k < (int32)objectArray.size(); ++k)
        {
            objectArray[k]->PrintToFile(file, identationDepth);
            if(k != ((int32)objectArray.size() - 1) )
            {
                file->WriteString(", ");
            }
        }
        file->WriteString("]");
    }
    else if (type == TYPE_MAP)
    {
    	const int32 IDENTATION_SPACES_COUNT = 4;
    	int32 spacesCount = identationDepth * IDENTATION_SPACES_COUNT;
  
		char8* spacesBuffer = new char8[spacesCount];
		memset(spacesBuffer, 0x20, spacesCount);
 		String spaces(spacesBuffer, spacesCount);
		delete[] spacesBuffer;

        file->WriteString("\r\n" + spaces + "{ ");
        for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != objectMap.end(); )
        {

            String strToFile( t->first + " : ");
            file->WriteString(strToFile);
            t->second->PrintToFile(file, ++identationDepth);
            if( ++t != objectMap.end())
            {
                file->WriteString(",");
                
            }
        }
        file->WriteString(" }");
	}
}
	
int32 YamlNode::GetCount()
{
	switch (type) 
	{
		case TYPE_MAP: return (int32)objectMap.size();
		case TYPE_ARRAY: return (int32)objectArray.size();
        default: break;
	}
	return 1;
}
int32  YamlNode::AsInt()
{
    return AsInt32();
}

int32 YamlNode::AsInt32()
{
	int32 ret;
	sscanf(nwStringValue.c_str(), "%d", &ret);
	return ret;
}

uint32 YamlNode::AsUInt32()
{    
    uint32 ret;
    sscanf(nwStringValue.c_str(), "%u", &ret);
    return ret;
}
    
int64 YamlNode::AsInt64()
{
    int64 ret;
    sscanf(nwStringValue.c_str(), "%lld", &ret);
    return ret;
}

uint64 YamlNode::AsUInt64()
{
    uint64 ret;
    sscanf(nwStringValue.c_str(), "%llu", &ret);
    return ret;
}
    
float32	YamlNode::AsFloat()
{
	float32 ret;
	sscanf(nwStringValue.c_str(), "%f", &ret);
	return ret;
}

const String & YamlNode::AsString()
{
	return nwStringValue;
}

bool YamlNode::AsBool()
{
	return ("true" == nwStringValue);
}

const WideString & YamlNode::AsWString()
{
	return stringValue;
}
	
Vector2	YamlNode::AsPoint()
{
	Vector2 result;
	if (type == TYPE_ARRAY)
	{
		YamlNode * x = Get(0);
		if (x)result.x = x->AsFloat();
		YamlNode * y = Get(1);
		if (y)result.y = y->AsFloat();
	}
	return result;
}

Vector3 YamlNode::AsVector3()
{
	Vector3 result(0, 0, 0);
	if (type == TYPE_ARRAY)
	{
		YamlNode * x = Get(0);
		if (x)
            result.x = x->AsFloat();
        
		YamlNode * y = Get(1);
		if (y)
            result.y = y->AsFloat();
        
		YamlNode * z = Get(2);
		if (z)
            result.z = z->AsFloat();
	}
	return result;        
}
    
Vector4 YamlNode::AsVector4()
{
    Vector4 result(0, 0, 0, 0);
    if (type == TYPE_ARRAY)
    {
        YamlNode * x = Get(0);
        if (x)
            result.x = x->AsFloat();
        
        YamlNode * y = Get(1);
        if (y)
            result.y = y->AsFloat();
        
        YamlNode * z = Get(2);
        if (z)
            result.z = z->AsFloat();

        YamlNode * w = Get(3);
        if (w)
            result.w = w->AsFloat();
    }
    return result;        
}

Vector2 YamlNode::AsVector2()
{
	return AsPoint();
}
	
Rect	YamlNode::AsRect()
{
	Rect result;
	if (type == TYPE_ARRAY)
	{
		YamlNode * x = Get(0);
		if (x)result.x = x->AsFloat();
		YamlNode * y = Get(1);
		if (y)result.y = y->AsFloat();
		YamlNode * dx = Get(2);
		if (dx)result.dx = dx->AsFloat();
		YamlNode * dy = Get(3);
		if (dy)result.dy = dy->AsFloat();
	}
	return result;
}

VariantType YamlNode::AsVariantType()
{
    VariantType retValue;
    
    Map<String, YamlNode*> & mapFromNode = AsMap();
        
    for(Map<String, YamlNode*>::iterator it = mapFromNode.begin(); it != mapFromNode.end(); ++it)
    {
        String innerTypeName = it->first;
        
        if(innerTypeName == TYPENAME_BOOLEAN)
        {
            retValue.SetBool(it->second->AsBool());
        }
        if(innerTypeName == TYPENAME_INT32)
        {
            retValue.SetInt32(it->second->AsInt32());
        }
        if(innerTypeName == TYPENAME_UINT32)
        {
            retValue.SetUInt32(it->second->AsUInt32());
        }
        if(innerTypeName == TYPENAME_INT64)
        {
            retValue.SetInt64(it->second->AsInt64());
        }
        if(innerTypeName == TYPENAME_UINT64)
        {
            retValue.SetUInt64(it->second->AsUInt64());
        }
        if(innerTypeName == TYPENAME_FLOAT)
        {
            retValue.SetFloat(it->second->AsFloat());
        }
        if(innerTypeName == TYPENAME_STRING)
        {
            retValue.SetString(it->second->AsString());
        }
        if(innerTypeName == TYPENAME_WIDESTRING)
        {
            retValue.SetWideString(it->second->AsWString());
        }
        if(innerTypeName == TYPENAME_BYTE_ARRAY)
        {
            Vector<YamlNode*> byteArrayNoodes = it->second->AsVector();
            int32 size = byteArrayNoodes.size();
            uint8 innerArray[size];
            for (int32 i = 0; i < size; ++i )
            {
                int val = 0;
                int retCode = sscanf(byteArrayNoodes[i]->AsString().c_str(), "%x", &val);
                if(val > CHAR_MAX || retCode == 0)
                {
                    return retValue;
                }
                innerArray[i] = static_cast<uint8>(val);
            }
            retValue.SetByteArray(innerArray, size);
        }
        if(innerTypeName == TYPENAME_KEYED_ARCHIVE)
        {
            KeyedArchive innerArch;
            innerArch.LoadFromYamlNode(this);
            retValue.SetKeyedArchive(&innerArch);
        }
        if(innerTypeName == TYPENAME_VECTOR2)
        {
            retValue.SetVector2(it->second->AsVector2());
        }
        if(innerTypeName == TYPENAME_VECTOR3)
        {
            retValue.SetVector3(it->second->AsVector3());
        }
        if(innerTypeName == TYPENAME_VECTOR4)
        {
            retValue.SetVector4(it->second->AsVector4());
        }
        if(innerTypeName == TYPENAME_MATRIX2)
        {
            YamlNode* firstRowNode  = it->second->Get(0);
            YamlNode* secondRowNode = it->second->Get(1);
            if(NULL == firstRowNode || NULL == secondRowNode )
            {
                return retValue;
            }
            Vector2 fRowVect = firstRowNode->AsVector2();
            Vector2 sRowVect = secondRowNode->AsVector2();
            retValue.SetMatrix2(Matrix2(fRowVect.x,fRowVect.y,sRowVect.x,sRowVect.y));
        }
        if(innerTypeName == TYPENAME_MATRIX3)
        {
            YamlNode* firstRowNode  = it->second->Get(0);
            YamlNode* secondRowNode = it->second->Get(1);
            YamlNode* thirdRowNode  = it->second->Get(2);
            
            if(NULL == firstRowNode  ||
               NULL == secondRowNode ||
               NULL == thirdRowNode )
            {
                return retValue;
            }
            Vector3 fRowVect = firstRowNode ->AsVector3();
            Vector3 sRowVect = secondRowNode->AsVector3();
            Vector3 tRowVect = thirdRowNode ->AsVector3();
            
            retValue.SetMatrix3(Matrix3(fRowVect.x,fRowVect.y,fRowVect.z,
                               sRowVect.x,sRowVect.y,sRowVect.z,
                               tRowVect.x,tRowVect.y,tRowVect.z));
        }
        if(innerTypeName == TYPENAME_MATRIX4)
        {
            YamlNode* firstRowNode  = it->second->Get(0);
            YamlNode* secondRowNode = it->second->Get(1);
            YamlNode* thirdRowNode  = it->second->Get(2);
            YamlNode* fourthRowNode = it->second->Get(3);
            
            if(NULL == firstRowNode || NULL == secondRowNode ||
               NULL == thirdRowNode || NULL == fourthRowNode)
            {
                return retValue;
            }
            Vector4 fRowVect  = firstRowNode ->AsVector4();
            Vector4 sRowVect  = secondRowNode->AsVector4();
            Vector4 tRowVect  = thirdRowNode ->AsVector4();
            Vector4 foRowVect = fourthRowNode->AsVector4();
            
            retValue.SetMatrix4(Matrix4(fRowVect.x,fRowVect.y,fRowVect.z,fRowVect.w,
                                        sRowVect.x,sRowVect.y,sRowVect.z,sRowVect.w,
                                        tRowVect.x,tRowVect.y,tRowVect.z,tRowVect.w,
                                        foRowVect.x,foRowVect.y,foRowVect.z,foRowVect.w));
        }
    }
    
    return retValue;
}
	
Vector<YamlNode*> & YamlNode::AsVector()
{
	return objectArray;
}

Map<String, YamlNode*> & YamlNode::AsMap()
{
	return objectMap;
}

	

YamlNode * YamlNode::Get(int32 index)
{
	if (type == TYPE_ARRAY)
	{
		return objectArray[index];
	}else if (type == TYPE_MAP)
	{
		return objectArray[index];
		/*Map<String, YamlNode*>::const_iterator end = objectMap.end();
		for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != end; ++t)
		{	
			YamlNode * n = t->second;
			if (n->mapIndex == index)return n;
		}*/
	}
	return 0;
}
	
static String emptyString = String("");
		
const String &	YamlNode::GetItemKeyName(int32 index)
{
	if (type == TYPE_MAP)
	{
		Map<String, YamlNode*>::const_iterator end = objectMap.end();
		for (Map<String, YamlNode*>::iterator t = objectMap.begin(); t != end; ++t)
		{	
			YamlNode * n = t->second;
			if (n->mapIndex == index)return t->first;
		}
	}	
	return emptyString;
}
	
YamlNode * YamlNode::Get(const String & name)
{
	if (type == TYPE_MAP)
	{
		Map<String, YamlNode*>::iterator t;
		if ((t = objectMap.find(name)) != objectMap.end())
		{
			return t->second;
		}
	}	
	return 0;
}


void  YamlNode::InitFromVariantType(VariantType* varType)
{
    type = TYPE_MAP;
    
    //create key     
    String variantName = VariantType::variantNamesMap[varType->type].variantName;

    //create value node
    YamlNode* valueNode = new YamlNode(YamlNode::TYPE_STRING);
    valueNode->FillContentAccordingToVariantTypeValue(varType);
    objectMap[variantName]=valueNode;
}
    
    
void  YamlNode::FillContentAccordingToVariantTypeValue(VariantType* varType)
{
    type = TYPE_STRING;
    char str[30];
    str[0]='\0';
    switch(varType->type)
    {
        case VariantType::TYPE_BOOLEAN:
        {
            nwStringValue = varType->AsBool() ? "true" : "false";
        }
            break;
        case VariantType::TYPE_INT32:
        {
            sprintf(str, "%d", varType->AsInt32());
        }
            break;
        case VariantType::TYPE_FLOAT:
        {
            sprintf(str, "%f", varType->AsFloat());
        }
            break;
        case VariantType::TYPE_STRING:
        {
            nwStringValue = String(varType->AsString());
            stringValue = StringToWString(varType->AsString());
        }
            break;
        case VariantType::TYPE_WIDE_STRING:
        {
            stringValue = WideString(varType->AsWideString());
            nwStringValue = String(WStringToString(varType->AsWideString()));
        }
            break;
        case VariantType::TYPE_UINT32:
        {
            sprintf(str, "%u", varType->AsUInt32());
        }
            break;
        case VariantType::TYPE_INT64:
        {
            sprintf(str, "%lld", varType->AsInt64());
        }
            break;
        case VariantType::TYPE_UINT64:
        {
            sprintf(str, "%llu", varType->AsUInt64());
        }
            break;
        case VariantType::TYPE_BYTE_ARRAY:
        {
            //! need to create subnodes
            type = TYPE_ARRAY;
            const uint8* byteArray = varType->AsByteArray();
            int32 byteArraySize = varType->AsByteArraySize();
            String arrayRepresentation = "";
            for (int32 i = 0; i < byteArraySize; ++i)
            {
                char letter[10];
                sprintf(letter, "%x",byteArray[i]);
                String letterRepresentation(letter);
                YamlNode* innerNode = new YamlNode(TYPE_STRING);
                innerNode->nwStringValue = letterRepresentation;
                innerNode->stringValue = StringToWString(letterRepresentation);
                objectArray.push_back(innerNode);
            }
        }
            break;
        case VariantType::TYPE_KEYED_ARCHIVE:
        {
            KeyedArchive* archive = varType->AsKeyedArchive();
            type = TYPE_ARRAY;            
            
            //creation array with variables
            const Map<String, VariantType*> & innerArchiveMap =  archive->GetArchieveData();
            for (Map<String, VariantType*>::const_iterator it = innerArchiveMap.begin(); it != innerArchiveMap.end(); ++it)
            {
                YamlNode* arrayElementNode = new YamlNode(TYPE_MAP);
                YamlNode* arrayElementNodeValue = new YamlNode(TYPE_MAP);
                
                arrayElementNodeValue->InitFromVariantType(it->second);
                arrayElementNode->objectMap[it->first] = arrayElementNodeValue;
                
                objectArray.push_back(arrayElementNode);
            }

        }
            break;
        case VariantType::TYPE_VECTOR2:
        {
            type = TYPE_ARRAY;
            const Vector2 & vector = varType->AsVector2();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_VECTOR3:
        {
            type = TYPE_ARRAY;
            const Vector3& vector = varType->AsVector3();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_VECTOR4:
        {
            type = TYPE_ARRAY;
            const Vector4& vector = varType->AsVector4();
            ProcessVector(vector.data,COUNT_OF(vector.data));
        }
            break;
        case VariantType::TYPE_MATRIX2:
        {
            type = TYPE_ARRAY;
            const Matrix2& matrix = varType->AsMatrix2();
            uint32 dimension = sqrt(COUNT_OF(matrix.data));
            const float32* array = &matrix._data[0][0];
            ProcessMatrix(array, dimension);
        }
            break;
        case VariantType::TYPE_MATRIX3:
        {
            type = TYPE_ARRAY;
            const Matrix3& matrix = varType->AsMatrix3();
            uint32 dimension = sqrt(COUNT_OF(matrix.data));
            const float32* array = &matrix._data[0][0];
            ProcessMatrix( array, dimension );
        }
            break;
        case VariantType::TYPE_MATRIX4:
        {
            type = TYPE_ARRAY;
            const Matrix4& matrix = varType->AsMatrix4();
            uint32 dimension = sqrt(COUNT_OF(matrix.data));
            const float32* array = &matrix._data[0][0];
            ProcessMatrix( array, dimension );
        }
            break;
        default:
            break;
    }
    
    if(str[0] != '\0')
    {
        String value(str);
        nwStringValue = value;
        stringValue = StringToWString(value);
    }
}

void YamlNode::ProcessMatrix(const float32* array,uint32 dimension)
{
    YamlNode* rowNode;
    for (uint32 i = 0; i < dimension; ++i)
    {
        rowNode = new YamlNode(TYPE_ARRAY);
        YamlNode* columnNode = NULL;
        for (uint32 j = 0; j < dimension; ++j)
        {
            char letter[10];
            const float32* elementOfArray = array + ((i*dimension)+j);
            sprintf(letter, "%f",*elementOfArray);
            String letterRepresentation(letter);
            columnNode = new YamlNode(TYPE_STRING);
            columnNode->nwStringValue = letterRepresentation;
            columnNode->stringValue = StringToWString(letterRepresentation);
            rowNode->objectArray.push_back(columnNode);
        }
        objectArray.push_back(rowNode);
    }
}
 
void YamlNode::ProcessVector(const float32* array,uint32 dimension)
{
    for (uint32 i = 0; i < dimension; ++i)
    {
        char letter[10];
        const float32* elementOfArray = array + i;
        sprintf(letter, "%f", *elementOfArray);
        String letterRepresentation(letter);
        YamlNode* innerNode = new YamlNode(TYPE_STRING);
        innerNode->nwStringValue = letterRepresentation;
        innerNode->stringValue = StringToWString(letterRepresentation);
        objectArray.push_back(innerNode);
    }
}
    
void YamlNode::InitFromKeyedArchive(KeyedArchive* archive)
{
    type = TYPE_MAP;

    
    //creation array with variables
    YamlNode* arrayContentNode = new YamlNode(YamlNode::TYPE_ARRAY);
    const Map<String, VariantType*> & innerArchiveMap =  archive->GetArchieveData();
    for (Map<String, VariantType*>::const_iterator it = innerArchiveMap.begin(); it != innerArchiveMap.end(); ++it)
    {
        YamlNode* arrayElementNode = new YamlNode(TYPE_MAP);
        YamlNode* arrayElementNodeValue = new YamlNode(TYPE_MAP);

        arrayElementNodeValue->InitFromVariantType(it->second);
        arrayElementNode->objectMap[it->first] = arrayElementNodeValue;
        
        arrayContentNode->objectArray.push_back(arrayElementNode);
    }
    
    objectMap[TYPENAME_KEYED_ARCHIVE] = arrayContentNode;
     
}
    
/*******************************************************/
	
YamlParser * YamlParser::Create(const String & fileName)
{
	YamlParser * parser = new YamlParser();
	if (parser)
	{
		bool parseResult = parser->Parse(fileName);
		if(!parseResult)
		{
			SafeRelease(parser);
			return 0;
		}
	}
	return parser;
}
	
bool YamlParser::Parse(const String & pathName)
{
	yaml_parser_t parser;
	yaml_event_t event;
	
	int done = 0;
	
	/* Create the Parser object. */
	yaml_parser_initialize(&parser);
	
	yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
	
	/* Set a string input. */
	//yaml_parser_set_input_string(&parser, (const unsigned char*)pathName.c_str(), pathName.length());
	

	File * yamlFile = File::Create(pathName, File::OPEN | File::READ);
    if (!yamlFile)
    {
    Logger::Error("[YamlParser::Parse] Can't create file: %s", pathName.c_str());
        return false;
    }
    
	dataHolder.fileSize = yamlFile->GetSize();
	dataHolder.data = new uint8[dataHolder.fileSize];
	dataHolder.dataOffset = 0;
	yamlFile->Read(dataHolder.data, dataHolder.fileSize);
	SafeRelease(yamlFile);
	
	yaml_parser_set_input(&parser, read_handler, &dataHolder);

	YamlNode * mapKey = 0;
//	YamlNode * mapValue = 0;

	/* Read the event sequence. */
	while (!done) 
	{
		
		/* Get the next event. */
		if (!yaml_parser_parse(&parser, &event))
		{
			Logger::Error("[YamlParser::Parse] error: type: %d %s line: %d pos: %d", parser.error, parser.problem, parser.problem_mark.line, parser.problem_mark.column);
			break;
		}
		
		/*if (event.encoding != YAML_UTF8_ENCODING)
		{
			Logger::Debug("wrong encoding");
		}*/
		
		switch(event.type)
		{
		case YAML_ALIAS_EVENT:
			Logger::Debug("alias: %s", event.data.alias.anchor);
			break;
		
		case YAML_SCALAR_EVENT:
			{
				YamlNode * node = new YamlNode(YamlNode::TYPE_STRING);

				/*CFStringRef s = CFStringCreateWithBytes(NULL, event.data.scalar.value, event.data.scalar.length, kCFStringEncodingUTF8, false);
				int32 length = CFStringGetLength(s); 
				node->stringValue.resize(length); 
				for (int i = 0; i < length; i++) 
				{
					UniChar uchar = CFStringGetCharacterAtIndex(s, i);
					node->stringValue[i] = (wchar_t)uchar;
				}
				CFRelease(s);
				node->nwStringValue = String((const char*)event.data.scalar.value); */
				node->nwStringValue = String((const char*)event.data.scalar.value);
				UTF8Utils::EncodeToWideString((uint8*)event.data.scalar.value, (int32)event.data.scalar.length, node->stringValue);
				
				
				if (objectStack.size() == 0)
				{
					rootObject = node;
				}else
				{
					YamlNode * topContainer = objectStack.top();
					if (topContainer->type == YamlNode::TYPE_MAP)
					{
						if (mapKey == 0)mapKey = node;
						else
						{
							if (topContainer->Get(mapKey->nwStringValue))
							{
								Logger::Error("[YamlParser::Parse] error in %s: attempt to create duplicate map node: %s", pathName.c_str(), mapKey->nwStringValue.c_str());
							}
							
							node->mapIndex = topContainer->mapCount ++;
							topContainer->objectMap[mapKey->nwStringValue] = node;
							topContainer->objectArray.push_back(SafeRetain(node)); // duplicate in array
							SafeRelease(mapKey);
						}
					}else if (topContainer->type == YamlNode::TYPE_ARRAY)
					{
						topContainer->objectArray.push_back(node);
					}
				}
				
//				NSLog()
//				wprintf(L"scalar: %s %S\n", event.data.scalar.value, node->stringValue.c_str());
//				Logger::Debug("scalar: %s %d", event.data.scalar.value, length);
//				CFIndex length = CFStringGetLength(s);
//				UniChar *buffer = malloc(length * sizeof(UniChar));
//				CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
//				node->stringValue = (char buffer;
//				free(buffer);
				
				//node->stringValue = event.data.scalar.value;
			}
			break;
		
		case YAML_DOCUMENT_START_EVENT:
			//Logger::Debug("document start:");
			break;
		
		case YAML_DOCUMENT_END_EVENT:
			//Logger::Debug("document end:");
			break;

		case YAML_SEQUENCE_START_EVENT:
			{
//				printf("[");
				YamlNode * node = new YamlNode(YamlNode::TYPE_ARRAY);
				if (objectStack.size() == 0)
					rootObject = node;
				else
				{
					YamlNode * topContainer = objectStack.top();
					if (topContainer->type == YamlNode::TYPE_MAP)
					{
						if (mapKey == 0)
						{
							printf("Something wrong");
						}
						else
						{
//							String s = String(mapKey->stringValue.begin(), mapKey->stringValue.end());
//							printf("put to map: %s\n", s.c_str());
							
							if (topContainer->Get(mapKey->nwStringValue))
							{
								Logger::Error("[YamlParser::Parse] error in %s: attempt to create duplicate map node: %s", pathName.c_str(), mapKey->nwStringValue.c_str());
							}
							
							node->mapIndex = topContainer->mapCount ++;
							topContainer->objectMap[mapKey->nwStringValue] = node;
							topContainer->objectArray.push_back(SafeRetain(node));
							SafeRelease(mapKey);
						}
					}else if (topContainer->type == YamlNode::TYPE_ARRAY)
					{
						topContainer->objectArray.push_back(node);
					}
				}
				objectStack.push(node);
			}break;
				
		case YAML_SEQUENCE_END_EVENT:
			{
//				printf("]");
				objectStack.pop();
			}break;
		
		case YAML_MAPPING_START_EVENT:
			{
//				printf("{");
				YamlNode * node = new YamlNode(YamlNode::TYPE_MAP);
				if (objectStack.size() == 0)
					rootObject = node;
				else
				{
					YamlNode * topContainer = objectStack.top();
					
					if (topContainer->type == YamlNode::TYPE_MAP)
					{
						if (mapKey == 0)
						{
//							printf("Something wrong");
						}
						else
						{
							//String s = String(mapKey->stringValue.begin(), mapKey->stringValue.end());
//							printf("put to map: %s\n", s.c_str());

							if (topContainer->Get(mapKey->nwStringValue))
							{
								Logger::Error("[YamlParser::Parse] error in %s: attempt to create duplicate map node: %s", pathName.c_str(), mapKey->nwStringValue.c_str());
							}
							
							node->mapIndex = topContainer->mapCount ++;
							topContainer->objectMap[mapKey->nwStringValue] = node;
							node->stringValue = mapKey->stringValue;
							node->nwStringValue = mapKey->nwStringValue;
							topContainer->objectArray.push_back(SafeRetain(node));
							SafeRelease(mapKey);
						}
					}else if (topContainer->type == YamlNode::TYPE_ARRAY)
					{
						topContainer->objectArray.push_back(node);
					}
				}
				objectStack.push(node);
			}
			break;
				
		case YAML_MAPPING_END_EVENT:
			{
//				printf("}");
				objectStack.pop();
			}
			break;
        default:
            break;
		};

		/* Are we finished? */
		done = (event.type == YAML_STREAM_END_EVENT);
		
		/* The application is responsible for destroying the event object. */
		yaml_event_delete(&event);
		
	}
	
	//rootObject->Print(0);
	
	//printf("%s (%d events)\n", (error ? "FAILURE" : "SUCCESS"), count);
	
	/* Destroy the Parser object. */
	yaml_parser_delete(&parser);
//	fclose(input);

	SafeDeleteArray(dataHolder.data);
    
    DVASSERT(objectStack.size() == 0);
	
	return true;
}
	

	
YamlParser::YamlParser()
{
	rootObject = 0;
	dataHolder.data = 0;
}

YamlParser::~YamlParser()
{
	SafeRelease(rootObject);
	SafeDeleteArray(dataHolder.data);
}

std::vector<String> split(const String& s, const String& delim, const bool keep_empty = true) 
{
	std::vector<String> result;
	if (delim.empty()) {
		result.push_back(s);
		return result;
	}
	String::const_iterator substart = s.begin(), subend;
	while (true) {
		subend = std::search(substart, s.end(), delim.begin(), delim.end());
		String temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			result.push_back(temp);
		}
		if (subend == s.end()) {
			break;
		}
		substart = subend + delim.size();
	}
	return result;
}
YamlNode * YamlParser::GetNodeByPath(const String & path)
{
// 	std::vector<String> result = split(path, ".");
// 	if (result.size() > 0)
// 	{
// 		//int pos = 0;
// 		for (int pos = 0; pos < result.size(); ++pos)
// 		{
// 			
// 		}
// 	}
	return 0;
}
	
YamlNode * YamlParser::GetRootNode()
{
	return rootObject;
}


}