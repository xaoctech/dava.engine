#ifndef __IMAGE_SPLITTER_H__
#define __IMAGE_SPLITTER_H__

#include "DAVAEngine.h"

class ImageSplitter
{
public:

    static bool SplitImage(const DAVA::FilePath &pathname, DAVA::Set<DAVA::String> &errorLog);
    static bool MergeImages(const DAVA::FilePath &folder, DAVA::Set<DAVA::String> &errorLog);
    
private:
    
    static void SaveImage(DAVA::Image *image, const DAVA::FilePath &pathname);
    static DAVA::Image * LoadImage(const DAVA::FilePath &pathname);
    
    static void ReleaseImages(DAVA::Image *r, DAVA::Image *g, DAVA::Image *b, DAVA::Image *a);
};



#endif // __IMAGE_SPLITTER_H__