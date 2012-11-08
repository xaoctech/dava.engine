#include "SettingsManager.h"

#define H 10.f
#define ANGLE_SPEED 80.f
#define VELOCITY 40.f
#define LANDSCAPE_NODE_NAME "Landscape"
#define X 3
#define Y 4

SettingsManager sm;

void SettingsManager::Init()
{
	landscapeNodeName = LANDSCAPE_NODE_NAME;
	landscapePartitioning.x = X;
	landscapePartitioning.y = Y;
	cameraElevation = H;
	cameraRotationSpeed = ANGLE_SPEED;
	cameraMovementSpeed = VELOCITY;
}

void SettingsManager::InitWithFile(const String &filename)
{
    Init();
    
    YamlParser* parser = YamlParser::Create(filename);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            YamlNode* node;
            
            node = rootNode->Get("landscapeNodeName");
            if(node)
            {
                landscapeNodeName = node->AsString();
            }
            
            node = rootNode->Get("landscapePartitioning");
            if(node && node->GetCount() == 2)
            {
                YamlNode* x = node->Get(0);
                YamlNode* y = node->Get(1);
                
                if(x && y)
                {
                    landscapePartitioning.x = x->AsInt();
                    landscapePartitioning.y = y->AsInt();
                }
            }
            
            node = rootNode->Get("cameraElevation");
            if(node)
            {
                cameraElevation = node->AsFloat();
            }
            
            node = rootNode->Get("cameraRotationSpeed");
            if(node)
            {
                cameraRotationSpeed = node->AsFloat();
            }
            
            node = rootNode->Get("cameraSpeed");
            if(node)
            {
                cameraMovementSpeed = node->AsFloat();
            }
        }
    }
    
    SafeRelease(parser);
}

Point2i SettingsManager::GetLandscapePartitioning() const
{
	return landscapePartitioning;
}

const String SettingsManager::GetLandscapeNodeName() const
{
	return landscapeNodeName;
}

float32 SettingsManager::GetCameraElevation() const
{
	return cameraElevation;
}

float32 SettingsManager::GetCameraRotationSpeed() const
{
	return cameraRotationSpeed;
}

float32 SettingsManager::GetCameraMovementSpeed() const
{
	return cameraMovementSpeed;
}