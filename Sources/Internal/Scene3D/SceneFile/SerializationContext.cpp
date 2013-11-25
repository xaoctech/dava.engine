/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/DataNode.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material.h"

namespace DAVA
{
	class MaterialNameMapper
	{
	public:

/*
		static FastName MapName(Material* mat)
		{
			String name = "Global";
			
			switch(mat->type)
			{
				case Material::MATERIAL_UNLIT_TEXTURE:
				{
					name += ".Textured";
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
				{
					name += ".Textured.Lightmap";
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DECAL:
				{
					name += ".Textured.Decal";
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DETAIL:
				{
					name += ".Textured.Detail";
					break;
				}
					
				case Material::MATERIAL_VERTEX_LIT_TEXTURE:
				{
					name += ".Textured.VertexLit";
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
				{
					name += ".Textured.PixelLit";
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
				{
					name += ".Textured.PixelLit.Specular";
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
				{
					name += ".Textured.PixelLit.Specular.Gloss";
					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED:
				{
					name += ".Textured.VertexColor";
					break;
				}
					
				case Material::MATERIAL_SKYBOX:
				{
					name = "Skybox";
					break;
				}
					
				default:
					break;
			};
			
			if(mat->IsTextureShiftEnabled())
			{
				name += ".TextureShift";
			}
			
			if(Material::MATERIAL_FLAT_COLOR == mat->type)
			{
				name += ".Flatcolor";
			}
			
			if(mat->GetAlphablend() ||
			   Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED == mat->type)
			{
				name += ".Alphablend";
			}
			else if(mat->GetAlphatest())
			{
				name += ".Alphatest";
			}
			else
			{
				name += ".Opaque";
			}
			
			FastName fastName = name;
			return fastName;
		}
 */
        
		static FastName MapName(Material* mat)
		{
			FastName name;
			
			switch(mat->type)
			{
				case Material::MATERIAL_UNLIT_TEXTURE:
				{
					if(mat->GetAlphablend() ||
                       Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED == mat->type)
                    {
                        name = FastName("LodAlphablend");
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = FastName("LodAlphatest");
                    }
                    else
                    {
                        name = FastName("LodTextured");
                    }
                    
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("LodLightmapAlphatest");
                    }
                    else
                    {
                        name = FastName("LodLightmap");
                    }

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DECAL:
				{
					if(mat->GetAlphablend() ||
                       Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED == mat->type)
                    {
                        name = FastName("LodDecalAlphablend");
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = FastName("LodDecalAlphatest");
                    }

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DETAIL:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("LodDetailAlphatest");
                    }
                    else
                    {
                        name = FastName("LodDetail");
                    }
                    
					break;
				}
					
				case Material::MATERIAL_VERTEX_LIT_TEXTURE:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("LodVertexLitAlphatest");
                    }
                    else
                    {
                        name = FastName("LodVertexLit");
                    }
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
				{
					name = FastName("LodTextured");
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
				{
					name = FastName("LodTextured");
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
				{
					name = FastName("LodTextured");
					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED:
				{
					name = FastName("LodTextured");
					break;
				}
					
				case Material::MATERIAL_SKYBOX:
				{
					name = FastName("Skybox");
					break;
				}
					
				default:
					break;
			};
			
			return name;
		}
        
	};


	SerializationContext::~SerializationContext()
	{
		for(Map<uint64, DataNode*>::iterator it = dataBlocks.begin();
			it != dataBlocks.end();
			++it)
		{
			SafeRelease(it->second);
		}
	}

	NMaterial* SerializationContext::ConvertOldMaterialToNewMaterial(Material* oldMaterial,
											   InstanceMaterialState* oldMaterialState)
	{
		NMaterial * parentMaterial = 0;
		
		MaterialSystem* matSystem = scene->renderSystem->GetMaterialSystem();
		FastName newMaterialName = MaterialNameMapper::MapName(oldMaterial);
		parentMaterial = matSystem->GetMaterial(newMaterialName);
		DVASSERT(parentMaterial);
				
		NMaterial* resultMaterial = matSystem->CreateChild(parentMaterial);
		uint32 materialStateCount = resultMaterial->GetStateCount();
		
		uint32 materialMultiplexStepCount = (0 == materialStateCount) ? 1 : materialStateCount;
		for(uint32 i = 0; i < materialMultiplexStepCount; ++i)
		{
			NMaterialState* targetState = (0 == materialStateCount) ? resultMaterial : resultMaterial->GetState(i);
			
			if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
			{
				if(oldMaterialState)
				{
					targetState->SetTexture(NMaterial::TEXTURE_LIGHTMAP, oldMaterialState->GetLightmap());
				}
			}
			else if (Material::MATERIAL_UNLIT_TEXTURE_DECAL == oldMaterial->type)
			{
				targetState->SetTexture(NMaterial::TEXTURE_DECAL, oldMaterial->textures[Material::TEXTURE_DECAL]);
			}
			else if(Material::MATERIAL_UNLIT_TEXTURE_DETAIL == oldMaterial->type)
			{
				targetState->SetTexture(NMaterial::TEXTURE_DETAIL, oldMaterial->textures[Material::TEXTURE_DETAIL]);
			}
			
			if (oldMaterial->textures[Material::TEXTURE_DIFFUSE])
			{
				targetState->SetTexture(NMaterial::TEXTURE_ALBEDO, oldMaterial->textures[Material::TEXTURE_DIFFUSE]);
			}
			
			if(Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == oldMaterial->type)
			{
				if (oldMaterial->textures[Material::TEXTURE_NORMALMAP])
				{
					targetState->SetTexture(NMaterial::TEXTURE_NORMAL, oldMaterial->textures[Material::TEXTURE_NORMALMAP]);
				}
			}
						
			if(Material::MATERIAL_VERTEX_LIT_TEXTURE == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_DETAIL == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_DECAL == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_LIGHTMAP == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == oldMaterial->type)
			{
				targetState->SetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS, Shader::UT_FLOAT, 1, &oldMaterial->shininess);
				
				targetState->SetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->ambientColor);
				targetState->SetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->diffuseColor);
				targetState->SetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->specularColor);
			}
			
			//VI: do not set fog property here!
			//targetState->SetPropertyValue(NMaterial::PARAM_FOG_DENSITY, Shader::UT_FLOAT, 1, &oldMaterial->fogDensity);
			//targetState->SetPropertyValue(NMaterial::PARAM_FOG_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->fogColor);
			
			if(oldMaterial->isFlatColorEnabled)
			{
				targetState->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterialState->GetFlatColor());
			}
			
			if(oldMaterial->isTexture0ShiftEnabled)
			{
				targetState->SetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetTextureShift());
			}
			
			if(oldMaterialState)
			{
				if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
				{
					targetState->SetPropertyValue(NMaterial::PARAM_UV_OFFSET, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVOffset());
					targetState->SetPropertyValue(NMaterial::PARAM_UV_SCALE, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVScale());
				}
			}
		}
				
		return resultMaterial;
	}
	
	NMaterial* SerializationContext::GetNewMaterial(const String& name)
	{
		return scene->renderSystem->GetMaterialSystem()->GetMaterial(FastName(name));
	}
	
	MaterialSystem* SerializationContext::GetMaterialSystem()
	{
		return scene->renderSystem->GetMaterialSystem();
	}
}