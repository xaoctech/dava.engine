#ifndef __IMAGE_SPLITTER_H__
#define __IMAGE_SPLITTER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ImageSplitter
{
public:

    static bool SplitImage(const FilePath &pathname, Set<String> &errorLog);
    static bool MergeImages(const FilePath &folder, Set<String> &errorLog);
    
private:
    
    static void SaveImage(Image *image, const FilePath &pathname);
    static Image * LoadImage(const FilePath &pathname);
    
    static void ReleaseImages(Image *r, Image *g, Image *b, Image *a);
};



#endif // __IMAGE_SPLITTER_H__