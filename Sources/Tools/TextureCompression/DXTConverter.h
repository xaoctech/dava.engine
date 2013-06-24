#ifndef __DAVAENGINE_DXT_CONVERTER_H__
#define __DAVAENGINE_DXT_CONVERTER_H__

namespace DAVA
{
    
class FilePath;
class TextureDescriptor;
    
class DXTConverter
{
public:
    
    static FilePath ConvertPngToDxt(const FilePath & fileToConvert, const TextureDescriptor &descriptor);
    static FilePath GetDXTOutput(const FilePath &inputDXT);
};
    
};


#endif // __DAVAENGINE_DXT_CONVERTER_H__