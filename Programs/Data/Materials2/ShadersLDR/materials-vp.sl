#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"

#define INPUT_VERTEX_COLOR (VERTEX_BAKED_AO || VERTEX_COLOR || VERTEX_BLEND_TEXTURES)
#define EMIT_VERTEX_COLOR (VERTEX_COLOR)
#define USE_TEXCOORD_1 (USE_BAKED_LIGHTING || ALBEDO_TINT_BLEND_MODE != 0)

#include "include/forward.materials.vertex.h"
