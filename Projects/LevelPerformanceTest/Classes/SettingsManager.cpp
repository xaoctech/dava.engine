#include "SettingsManager.h"

#define H 10.f
#define ANGLE_SPEED 80.f
#define VELOCITY 40.f
#define LANDSCAPE_NODE_NAME "Landscape"
#define X 3
#define Y 4
#define MIN_FPS 15
#define MIN_FPS_SECTOR_COUNT 10
#define COLOR_TRANSPARENCY 127

SettingsManager sm;

void SettingsManager::InitWithFile(const FilePath &filename)
{
	landscapeNodeName = LANDSCAPE_NODE_NAME;
	landscapePartitioning.x = X;
	landscapePartitioning.y = Y;
	cameraElevation = H;
	cameraRotationSpeed = ANGLE_SPEED;
	cameraMovementSpeed = VELOCITY;
	minFps = MIN_FPS;
	minFpsSectorCount = MIN_FPS_SECTOR_COUNT;
	colorTransparency = COLOR_TRANSPARENCY;

	fpsList.push_back(-std::numeric_limits<float32>::infinity());
	colorList.push_back(Color(255.f, 255.f, 255.f, COLOR_TRANSPARENCY) / 255.f);

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
			
			node = rootNode->Get("minFps");
			if(node)
			{
				minFps = node->AsFloat();
			}
			
			node = rootNode->Get("minFpsSectorCount");
			if(node)
			{
				minFpsSectorCount = node->AsInt();
			}
			
			node = rootNode->Get("resultTextureColorTransparency");
			if(node)
			{
				colorTransparency = node->AsInt();
			}
			
			node = rootNode->Get("list");
			if(node)
			{
				int32 listCount = node->GetCount();
				for(int32 i = 0; i < listCount; ++i)
				{
					YamlNode* listElement = node->Get(i);
					if(listElement == 0 || listElement->GetCount() != 2)
						continue;

					YamlNode* fpsNode = listElement->Get(0);
					YamlNode* colorNode = listElement->Get(1);
					
					if(fpsNode == 0 || colorNode == 0 || colorNode->GetCount() != 3)
						continue;

					float32 fpsValue = fpsNode->AsFloat();
					Color color((float32)colorNode->Get(0)->AsInt(),
								(float32)colorNode->Get(1)->AsInt(),
								(float32)colorNode->Get(2)->AsInt(),
								colorTransparency);
					color /= 255.f;
					
					fpsList.push_back(fpsValue);
					colorList.push_back(color);
				}
			}
        }
    }

	fpsList.push_back(std::numeric_limits<float32>::infinity());

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

float32 SettingsManager::GetMinFps() const
{
	return minFps;
}

uint32 SettingsManager::GetMinFpsSectorCount() const
{
	return minFpsSectorCount;
}

uint8 SettingsManager::GetColorTransparency() const
{
	return colorTransparency;
}

Color SettingsManager::GetColorByFps(float32 fps) const
{
	for(uint32 i = 0; i < fpsList.size() - 1; ++i)
    {
		if(fps >= fpsList[i] && fps < fpsList[i + 1])
        {
			return colorList[i];
		}
	}
	return colorList[0];
}
