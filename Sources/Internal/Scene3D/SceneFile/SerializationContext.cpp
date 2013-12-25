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

#include "Utils/StringFormat.h"

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
					if(mat->GetAlphablend())
                    {
                        name = FastName("~res:/Materials/Legacy/Textured.Alphablend.material");
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/Textured.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/Textured.Opaque.material");
                    }
                    
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/TextureLightmap.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/TextureLightmap.Opaque.material");
                    }

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DECAL:
				{
					if(mat->GetAlphablend())
                    {
                        name = FastName("~res:/Materials/Legacy/Decal.Alphablend.material");
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/Decal.Alphatest.material");
                    }
					else
					{
						name = FastName("~res:/Materials/Legacy/Decal.Opaque.material");
					}

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DETAIL:
				{
					if(mat->GetAlphablend())
                    {
                        name = FastName("~res:/Materials/Legacy/Detail.Alphablend.material");
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/Detail.Alphatest.material");
                    }
					else
					{
						name = FastName("~res:/Materials/Legacy/Detail.Opaque.material");
					}
                    
					break;
				}
					
				case Material::MATERIAL_VERTEX_LIT_TEXTURE:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/VertexLit.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/VertexLit.Opaque.material");
                    }
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.Opaque.material");
                    }

					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.Specular.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.Specular.Opaque.material");
                    }

					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
				{
					if(mat->GetAlphatest())
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.SpecularMap.Alphatest.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/PixelLit.SpecularMap.Opaque.material");
                    }

					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED:
				{
					if(mat->GetAlphablend())
                    {
                        name = FastName("~res:/Materials/Legacy/VertexColor.Alphablend.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/VertexColor.Opaque.material");
                    }

					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND:
				{
					if(mat->GetAlphablend())
                    {
                        name = FastName("~res:/Materials/Legacy/VertexColor.FrameBlend.Alphablend.material");
                    }
                    else
                    {
                        name = FastName("~res:/Materials/Legacy/VertexColor.FrameBlend.Opaque.material");
                    }
					
					break;

				}
					
				case Material::MATERIAL_SKYBOX:
				{
					name = FastName("~res:/Materials/Legacy/Skybox.material");
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

		for(Map<uint64, NMaterial*>::iterator it = serializationMaterialMap.begin();
			it != serializationMaterialMap.end();
			++it)
		{
			SafeRelease(it->second);
			//VI: TODO: make sure there's only 1 reference to the material at this point
		}
	}

	NMaterial* SerializationContext::ConvertOldMaterialToNewMaterial(Material* oldMaterial,
																	 InstanceMaterialState* oldMaterialState,
																	 uint64 oldMaterialId)
	{
		//VI: need to build the following material structure:
		//VI:     INSTANCE_WITH_COMMON_PROPS_AND_TEXTURES
		//VI:     (this instance has
		//VI:     same name as old material) ->
		//VI:                                  OBJECT_INSTANCE
		//VI:                                  (this instance is
		//VI:                                  assigned to object
		//VI:                                  and has specific
		//VI:                                  properties set)
		
		NMaterial* material = GetMaterial(oldMaterialId);
		if(NULL == material)
		{
			FastName newMaterialName = MaterialNameMapper::MapName(oldMaterial);
			DVASSERT(newMaterialName.IsValid());
			
			material = MaterialSystem::CreateMaterial(FastName(oldMaterial->GetName()),
													  newMaterialName,
													  GetDefaultMaterialQuality());
			
			
			if(oldMaterial->IsFogEnabled())
			{
				material->SetFlag(NMaterial::FLAG_VERTEXFOG, NMaterial::FlagOn);
			}
						
			if(oldMaterial->IsTextureShiftEnabled())
			{
				material->SetFlag(NMaterial::FLAG_TEXTURESHIFT, NMaterial::FlagOn);
			}
			
			if(oldMaterial->IsFlatColorEnabled())
			{
				material->SetFlag(NMaterial::FLAG_FLATCOLOR, NMaterial::FlagOn);
			}
			
			Material::eViewOptions viewOptions = oldMaterial->GetViewOption();
			switch(viewOptions)
			{
				case Material::MATERIAL_VIEW_TEXTURE_ONLY:
				{
					material->SetFlag(NMaterial::FLAG_TEXTUREONLY, NMaterial::FlagOn);
					break;
				}
				case Material::MATERIAL_VIEW_LIGHTMAP_ONLY:
				{
					material->SetFlag(NMaterial::FLAG_LIGHTMAPONLY, NMaterial::FlagOn);
					break;
				}

				default:
					break;
			}
			
			if(oldMaterial->GetSetupLightmap())
			{
				material->SetFlag(NMaterial::FLAG_SETUPLIGHTMAP, NMaterial::FlagOn);
			}
			
			if (Material::MATERIAL_UNLIT_TEXTURE_DECAL == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(oldMaterial->textures[Material::TEXTURE_DECAL]);
				material->SetTexture(NMaterial::TEXTURE_DECAL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			else if(Material::MATERIAL_UNLIT_TEXTURE_DETAIL == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(oldMaterial->textures[Material::TEXTURE_DETAIL]);
				material->SetTexture(NMaterial::TEXTURE_DETAIL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			
			if (Material::MATERIAL_FLAT_COLOR != oldMaterial->type)
			{
				Texture* tex = PrepareTexture(oldMaterial->textures[Material::TEXTURE_DIFFUSE]);
				material->SetTexture(NMaterial::TEXTURE_ALBEDO, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			
			if(Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(oldMaterial->textures[Material::TEXTURE_NORMALMAP]);
				material->SetTexture(NMaterial::TEXTURE_NORMAL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}

			//VI: should not retain material here. it will be released in the context's destructor
			//VI: if the material still has children it will survive that.
			SetMaterial(oldMaterialId, material);
		}
		
		NMaterial* instanceMaterial = MaterialSystem::CreateMaterialInstance();
		
		if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
		{
			if(oldMaterialState)
			{
				instanceMaterial->GetIlluminationParams()->lightmapSize = oldMaterialState->GetLightmapSize();
			}
			
			Texture* tex = PrepareTexture(oldMaterialState ? oldMaterialState->GetLightmap() : NULL);
			instanceMaterial->SetTexture(NMaterial::TEXTURE_LIGHTMAP, tex);
			
			if(tex->isPink)
			{
				SafeRelease(tex);
			}
		}
		
		if(oldMaterial->isFlatColorEnabled)
		{
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterialState->GetFlatColor());
		}
		
		if(oldMaterial->isTexture0ShiftEnabled)
		{
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetTextureShift());
		}
		
		if(oldMaterialState)
		{
			if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
			{
				instanceMaterial->SetPropertyValue(NMaterial::PARAM_UV_OFFSET, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVOffset());
				instanceMaterial->SetPropertyValue(NMaterial::PARAM_UV_SCALE, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVScale());
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
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS, Shader::UT_FLOAT, 1, &oldMaterial->shininess);
			
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->ambientColor);
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->diffuseColor);
			instanceMaterial->SetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterial->specularColor);
		}

		
		material->AddChild(instanceMaterial);
		
		//VI: need to retain instance material before adding to context
		//VI: in other case it will be deleted in the context's destructor
		SetMaterial(instanceMaterial->GetMaterialKey(), SafeRetain(instanceMaterial));
		
		return instanceMaterial;
	}
			
	Texture* SerializationContext::PrepareTexture(Texture* tx)
	{
		return (tx) ? tx : Texture::CreatePink();
	}
}