#ifndef __IMAGE_SPLITTER_H__
#define __IMAGE_SPLITTER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ImageSplitter
{
public:

    static bool SplitImage(const String &pathname, Set<String> &errorLog);
    static bool MergeImages(const String &folder, Set<String> &errorLog);
    
private:
    
    static void SaveImage(Image *image, const String &pathname);
    static Image * LoadImage(const String &pathname);
    
    static void ReleaseImages(Image *r, Image *g, Image *b, Image *a);
};



#endif // __IMAGE_SPLITTER_H__