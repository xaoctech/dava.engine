/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
* OBJ reader
*/ 
#ifndef OBJREADER_H
#define OBJREADER_H

#include "utils.h"
#include "vecmath.h"
#include <string>
#include <vector>

namespace bex
{
class OBJReader
{
public:
    OBJReader()
        : m_vertices(0)
        ,
        m_vertexNormals(0)
        ,
        m_UVs(0)
        ,
        m_currentGroup(0)
    {
        m_dummyMaterial.diffuse = bex::ColorRGBA(0.9f, 0.9f, 0.9f, 1.0f);
        m_materialMap["dummy"] = m_dummyMaterial;
        m_currentMaterial = &m_dummyMaterial;
        m_currentMaterialName = "dummy";
    }

    struct Material
    {
        Material()
            : shininess(0.0f)
        {
        }
        bex::ColorRGBA diffuse;
        bex::ColorRGBA emissive;
        bex::ColorRGBA specular;
        float shininess;
    };

    struct Group
    {
        Material material;
        std::string materialName;
        std::vector<int> vertexIndexList;
        std::vector<int> normalIndexList;
        std::vector<int> uvIndexList;
    };

    void loadObjFile(const std::wstring file)
    {
        size_t pos = file.find_last_of(L"/");
        m_path = file.substr(0, pos + 1);
        loadFile(file);
    }
    void getMeshNames(std::vector<std::string>& meshNames)
    {
        std::map<std::string, Group>::const_iterator iter;
        iter = m_groupMap.begin();
        while (iter != m_groupMap.end())
        {
            meshNames.push_back(iter->first);
            iter++;
        }
    }

    const std::map<std::string, Material>& getMaterials()
    {
        return m_materialMap;
    }

    void getMeshData(const std::string& meshName, std::vector<bex::Vec3>& vertices, std::vector<bex::Vec3>& normals, std::vector<bex::Vec2>& UVs, std::string& materialName)
    {
        if (m_groupMap.find(meshName) != m_groupMap.end())
        {
            Group& tempGroup = m_groupMap.find(meshName)->second;
            for (unsigned int i = 0; i < tempGroup.vertexIndexList.size(); i++)
            {
                vertices.push_back(m_vertices[tempGroup.vertexIndexList[i] - 1]);
                normals.push_back(m_vertexNormals[tempGroup.normalIndexList[i] - 1]);
                UVs.push_back(m_UVs[tempGroup.uvIndexList[i] - 1]);
            }
            materialName = tempGroup.materialName;
        }
    }

private:
    std::vector<std::string> tokenizeString(const std::string& input, char breakChar)
    {
        std::vector<std::string> result;
        std::string::size_type firstPos = input.size();
        for (std::string::size_type lastPos = 0; lastPos <= input.size(); ++lastPos)
        {
            if (lastPos == input.size() || input[lastPos] == breakChar)
            {
                // In separator.
                if (firstPos < lastPos)
                {
                    result.push_back(input.substr(firstPos, lastPos - firstPos));
                    firstPos = input.size();
                }
            }
            else
            {
                // In token.
                if (firstPos > lastPos)
                {
                    firstPos = lastPos;
                }
            }
        }
        return result;
    }

    void processTokens(const std::vector<std::string>& tokens)
    {
        if (tokens.size() == 0)
        {
            return;
        }
        else if (tokens[0].compare("v") == 0)
        {
            m_vertices.push_back(bex::Vec3((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str())));
        }
        else if (tokens[0].compare("mtllib") == 0)
        {
            loadFile(m_path + bex::string2tstring(tokens[1]));
        }
        else if (tokens[0].compare("vn") == 0)
        {
            m_vertexNormals.push_back(bex::Vec3((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str())));
        }
        else if (tokens[0].compare("vt") == 0)
        {
            m_UVs.push_back(bex::Vec2((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str())));
        }
        else if (tokens[0].compare("f") == 0)
        {
            //No group, no face lists!
            if (m_currentGroup)
            {
                for (int i = 1; i < 4; i++)
                {
                    std::vector<std::string> faces = tokenizeString(tokens[i], '/');
                    m_currentGroup->vertexIndexList.push_back(atoi(faces[0].c_str()));
                    m_currentGroup->uvIndexList.push_back(atoi(faces[1].c_str()));
                    m_currentGroup->normalIndexList.push_back(atoi(faces[2].c_str()));
                }
            }
        }
        else if (tokens[0].compare("g") == 0)
        {
            std::string groupName = tokens[1];
            for (std::vector<std::string>::const_iterator it = tokens.begin() + 2; it != tokens.end(); ++it)
            {
                groupName += "." + *it;
            }
            if (m_groupMap.find(groupName) == m_groupMap.end())
            {
                Group g;
                g.material = *m_currentMaterial;
                g.materialName = m_currentMaterialName;
                m_groupMap[groupName] = g;
                m_currentGroup = &(m_groupMap.find(groupName)->second);
            }
            else
            {
                m_currentGroup = &(m_groupMap.find(groupName)->second);
            }
        }

        else if (tokens[0].compare("usemtl") == 0)
        {
            //No group, no material name!

            if (m_currentGroup)
            {
                std::map<std::string, Material>::iterator it;
                it = m_materialMap.find(tokens[1]);
                if (it == m_materialMap.end())
                {
                    m_currentGroup->material = m_dummyMaterial;
                    m_currentGroup->materialName = "dummy";
                }
                else
                {
                    m_currentMaterial = &it->second;
                    m_currentMaterialName = it->first;
                    m_currentGroup->material = it->second;
                    m_currentGroup->materialName = it->first;
                }
            }
        }
        else if (tokens[0].compare("newmtl") == 0)
        {
            m_materialMap[tokens[1]] = m_dummyMaterial;
            m_currentMaterial = &(m_materialMap.find(tokens[1])->second);
            m_currentMaterialName = tokens[1];
        }
        else if (tokens[0].compare("Kd") == 0)
        {
            m_currentMaterial->diffuse = bex::ColorRGBA((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str()), 1.0f);
        }
        else if (tokens[0].compare("Ke") == 0)
        {
            m_currentMaterial->emissive = bex::ColorRGBA((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str()), 1.0f);
        }
        else if (tokens[0].compare("Ks") == 0)
        {
            m_currentMaterial->specular = bex::ColorRGBA((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str()), 1.0f);
        }
        else if (tokens[0].compare("Ns") == 0)
        {
            m_currentMaterial->shininess = (float)atof(tokens[1].c_str());
        }
    }

    void loadFile(const std::wstring file)
    {
        std::ifstream fileStream;
        fileStream.open(file.c_str());

        char row[256];
        while (fileStream.good())
        {
            fileStream.getline(row, 256);
            std::string str = row;
            std::vector<std::string> tokens = tokenizeString(str, ' ');
            processTokens(tokens);
        }
    }

    std::vector<bex::Vec3> m_vertices;
    std::vector<bex::Vec3> m_vertexNormals;
    std::vector<bex::Vec2> m_UVs;
    std::map<std::string, Group> m_groupMap;
    std::map<std::string, Material> m_materialMap;
    std::wstring m_path;
    Material m_dummyMaterial;
    Group* m_currentGroup;
    Material* m_currentMaterial;
    std::string m_currentMaterialName;
};
}


#endif //OBJREADER_H
