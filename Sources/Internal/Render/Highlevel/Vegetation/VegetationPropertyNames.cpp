//
//  VegetationPropertyNames.cpp
//  Framework
//
//  Created by Valentine Ivanov on 4/24/14.
//
//

#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"

namespace DAVA
{
const FastName VegetationPropertyNames::UNIFORM_TILEPOS("tilePos");
const FastName VegetationPropertyNames::UNIFORM_WORLD_SIZE("worldSize");
const FastName VegetationPropertyNames::UNIFORM_CLUSTER_SCALE_DENSITY_MAP("clusterScaleDensityMap[0]");
const FastName VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE("heightmapScale");
const FastName VegetationPropertyNames::UNIFORM_SWITCH_LOD_SCALE("lodSwitchScale");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE("perturbationForce");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_POINT("perturbationPoint");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE("perturbationForceDistance");
const FastName VegetationPropertyNames::UNIFORM_BILLBOARD_DIRECTION("billboardDirection");
    
const FastName VegetationPropertyNames::FLAG_FRAMEBUFFER_FETCH("FRAMEBUFFER_FETCH");
const FastName VegetationPropertyNames::FLAG_BILLBOARD_DRAW("MATERIAL_GRASS_BILLBOARD");
const FastName VegetationPropertyNames::FLAG_GRASS_TRANSFROM("MATERIAL_GRASS_TRANSFORM");
    
const FastName VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH("HIGH");
const FastName VegetationPropertyNames::VEGETATION_QUALITY_NAME_LOW("LOW");
const FastName VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME("Vegetation");
    
const FastName VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP("vegetationmap");
};