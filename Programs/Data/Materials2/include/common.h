#define MAX_JOINTS 64
#define EPSILON 0.00001
#define _PI 3.1415926535897932384626433832795
#define _2PI (3.1415926535897932384626433832795 * 2.0)
#define _4PI (3.1415926535897932384626433832795 * 4.0)
#define MIN_HALF_PRECISION 0.00006103515625 /* 2^(-14) */
#define MIN_HALF_PRECISION_SQ 0.0078125 /* 2^(-7) */
#define MAX_HALF_PRECISION 65504.0
#define MIN_ROUGHNESS 0.05

#define VERTEX_COLOR_MULTIPLY 1
#define VERTEX_COLOR_SOFT_LIGHT 2

#ensuredefined TECH_COMBINE 0
#ensuredefined TECH_LUMINANCE_HISTORY 0
#ensuredefined TECH_BLOOM_THRESHOLD 0
#ensuredefined TECH_GAUSS_BLUR 0
#ensuredefined TECH_MUL_ADD_BLUR 0
#ensuredefined TECH_INIT_LUMINANCE 0
#ensuredefined TECH_LUMINANCE 0
#ensuredefined TECH_FINISH_LUMINANCE 0
#ensuredefined TECH_DEBUG 0
#ensuredefined TECH_DEBUG_R16F 0
#ensuredefined TECH_COPY 0
#ensuredefined TECH_RESET_LUMINANCE_HISTORY 0
#ensuredefined TECH_COMBINE_FORWARD_INPLACE 0
#ensuredefined SPECULAR_CONVOLUTION 0
#ensuredefined DIFFUSE_CONVOLUTION 0
#ensuredefined DIFFUSE_SPHERICAL_HARMONICS 0
#ensuredefined INTEGRATE_BRDF_LOOKUP 0
#ensuredefined DOWNSAMPLING 0
#ensuredefined CONVOLUTION_PASS 0
#ensuredefined TRANSMITTANCE 0
#ensuredefined IB_REFLECTIONS_PREPARE 0
#ensuredefined WRITE_SHADOW_MAP 0
#ensuredefined WRITE_TO_CUBEMAP 0
#ensuredefined DIRECTIONAL_LIGHT 0
#ensuredefined FORWARD_LIGHT 0

#ensuredefined ORIENT_ON_LANDSCAPE 0
#ensuredefined DECORATION_DRAW_LEVELS 0
#ensuredefined LANDSCAPE_PATCHES 0
#ensuredefined LANDSCAPE_USE_INSTANCING 0
#ensuredefined LANDSCAPE_VT_PAGE 0
#ensuredefined LANDSCAPE_MICRO_TESSELLATION 0
#ensuredefined LANDSCAPE_TESSELLATION_COLOR 0
#ensuredefined LANDSCAPE_LOD_MORPHING 0
#ensuredefined LANDSCAPE_MORPHING_COLOR 0
#ensuredefined LANDSCAPE_TOOL 0
#ensuredefined LANDSCAPE_TOOL_MIX 0
#ensuredefined LANDSCAPE_CURSOR 0

#ensuredefined HEIGHTMAP_FLOAT_TEXTURE 0
#ensuredefined LANDSCAPE_PICKING_UV 0
#ensuredefined LANDSCAPE_COVER_TEXTURE 0
#ensuredefined LANDSCAPE_CURSOR_V2 0

#ensuredefined DECORATION 0
#ensuredefined DECORATION_GPU_RANDOMIZATION 0

#ensuredefined VERTEX_COLOR 0

#ensuredefined COPY_GBUFFER_0 0
#ensuredefined COPY_GBUFFER_1 0
#ensuredefined COPY_GBUFFER_2 0
#ensuredefined COPY_GBUFFER_3 0

#ensuredefined EQUIRECTANGULAR_ENVIRONMENT_TEXTURE 0

#ensuredefined COMBINE_INPLACE 0
#ensuredefined FOWARD_WITH_COMBINE 0
#ensuredefined TECH_COMBINED_LUMINANCE 0

#ensuredefined ADVANCED_TONE_MAPPING 0
#ensuredefined ENABLE_COLOR_GRADING 0
#ensuredefined DISPLAY_HEAT_MAP 0
#ensuredefined DISPLAY_LIGHT_METER_MASK 0

#ensuredefined SOFT_SKINNING 0
#ensuredefined HARD_SKINNING 0

#ensuredefined FLIP_BACKFACE_NORMALS 0

#ensuredefined SPEED_TREE_OBJECT 0

#ensuredefined FRAMEBUFFER_FETCH 0

#ensuredefined USE_FRAMEBUFFER_FETCH 0

#ensuredefined FETCH_DISCARD 0

#ensuredefined USE_BAKED_LIGHTING 0

#ensuredefined VERTEX_BAKED_AO 0
#ensuredefined ALBEDO_ALPHA_MASK 0

#ensuredefined VERTEX_BLEND_TEXTURES 0
#ensuredefined VERTEX_BLEND_4_TEXTURES 0

#ensuredefined ALBEDO_TINT_BLEND_MODE 0

#ensuredefined NORMAL_BLEND_MODE 0
#ensuredefined USE_DETAIL_NORMAL_AO 0

#ensuredefined ENABLE_TXAA 0

#ensuredefined USE_DFG_APPROXIMATION 0

#ensuredefined PLACEHOLDER 0
#ensuredefined PLACEHOLDER 0
#ensuredefined PLACEHOLDER 0
