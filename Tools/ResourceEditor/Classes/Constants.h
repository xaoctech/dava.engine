#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

namespace ResourceEditor 
{

enum eExportFormat
{
    FORMAT_PNG = 0,
    FORMAT_PVR,
    FORMAT_DXT,
    
    FORMAT_COUNT
};
    
enum eNodeType
{
    NODE_LANDSCAPE  = 0,
    NODE_LIGHT,
    NODE_SERVICE_NODE,
    NODE_BOX,
    NODE_SPHERE,
    NODE_CAMERA,
    NODE_IMPOSTER,
    NODE_PARTICLE_EMITTER,
    NODE_USER_NODE,
    
    NODE_COUNT
};
    
enum eViewportType
{
    VIEWPORT_IPHONE = 0,
    VIEWPORT_RETINA,
    VIEWPORT_IPAD,
    VIEWPORT_DEFAULT,
    
    VIEWPORT_COUNT
};
  
    
};




#endif //#ifndef __CONSTANTS_H__