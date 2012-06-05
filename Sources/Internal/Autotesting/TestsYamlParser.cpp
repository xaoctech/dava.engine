#include "Autotesting/TestsYamlParser.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{

TestsYamlParser::TestsYamlParser() : BaseObject()
{
}

TestsYamlParser::~TestsYamlParser()
{
}

void TestsYamlParser::ParseYaml(const String &yamlFilePath)
{
    YamlParser* parser = YamlParser::Create(yamlFilePath);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            YamlNode* actionsNode = rootNode->Get("actions");
            if(actionsNode)
            {
                Vector<YamlNode*> actionNodes = actionsNode->AsVector();
                for(int32 i = 0; i < actionNodes.size(); ++i)
                {
                    YamlNode* actionNode = actionNodes[i];
                    YamlNode* actionNameNode = actionNodes[i]->Get("action");
                    if(actionNode && actionNameNode)
                    {                        
                        String actionName = actionNameNode->AsString();
                        Logger::Debug("TestsYamlParser::ParseYaml action=%s", actionName.c_str());

                        if(actionName == "Click")
                        {
                            YamlNode* idNode = actionNode->Get("id");
                            YamlNode* pointNode = actionNode->Get("point");
                            if(pointNode)
                            {
                                if(idNode)
                                {
                                    AutotestingSystem::Instance()->Click(pointNode->AsVector2(), idNode->AsInt());
                                }
                                else
                                {
                                    AutotestingSystem::Instance()->Click(pointNode->AsVector2());
                                }
                            }
                            else
                            {
                                YamlNode* controlNameNode = actionNode->Get("controlName");
                                if(controlNameNode)
                                {
                                    if(idNode)
                                    {
                                        AutotestingSystem::Instance()->Click(controlNameNode->AsString(), idNode->AsInt());
                                    }
                                    else
                                    {
                                        AutotestingSystem::Instance()->Click(controlNameNode->AsString());
                                    }
                                }
                                else
                                {
                                    Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                                }
                            }
                        }
                        else if(actionName == "TouchDown")
                        {
                            YamlNode* idNode = actionNode->Get("id");
                            YamlNode* pointNode = actionNode->Get("point");
                            if(pointNode)
                            {
                                if(idNode)
                                {
                                    AutotestingSystem::Instance()->TouchDown(pointNode->AsVector2(), idNode->AsInt());
                                }
                                else
                                {
                                    AutotestingSystem::Instance()->TouchDown(pointNode->AsVector2());
                                }
                            }
                            else
                            {
                                YamlNode* controlNameNode = actionNode->Get("controlName");
                                if(controlNameNode)
                                {
                                    if(idNode)
                                    {
                                        AutotestingSystem::Instance()->TouchDown(controlNameNode->AsString(), idNode->AsInt());
                                    }
                                    else
                                    {
                                        AutotestingSystem::Instance()->TouchDown(controlNameNode->AsString());
                                    }
                                }
                                else
                                {
                                    Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                                }
                            }
                        }
                        else if(actionName == "TouchUp")
                        {
                            YamlNode* idNode = actionNode->Get("id");
                            if(idNode)
                            {
                                AutotestingSystem::Instance()->TouchUp(idNode->AsInt());
                            }
                            else
                            {
                                AutotestingSystem::Instance()->TouchUp();
                            }
                        }
                        else if(actionName == "TouchMove")
                        {
                            YamlNode* idNode = actionNode->Get("id");
                            YamlNode* pointNode = actionNode->Get("point");
                            YamlNode* timeNode = actionNode->Get("time");
                            if(pointNode)
                            {
                                float32 time = 0.0f;
                                if(timeNode)
                                {
                                    time = timeNode->AsFloat();
                                }
                                if(idNode)
                                {
                                    AutotestingSystem::Instance()->TouchMove(pointNode->AsVector2(), time, idNode->AsInt());
                                }
                                else
                                {
                                    AutotestingSystem::Instance()->TouchMove(pointNode->AsVector2(), time);
                                }
                            }
                            else
                            {
                                Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else if(actionName == "SetText")
                        {
                            YamlNode* controlNameNode = actionNode->Get("controlName");
                            YamlNode* textNode = actionNode->Get("text");
                            if(controlNameNode)
                            {
                                if(textNode)
                                {
                                    AutotestingSystem::Instance()->SetText(controlNameNode->AsString(), textNode->AsWString());
                                }
                                else
                                {
                                    AutotestingSystem::Instance()->SetText(controlNameNode->AsString(), L"");
                                }
                            }
                            else
                            {
                                Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else if(actionName == "Wait")
                        {
                            YamlNode* timeNode = actionNode->Get("time");
                            if(timeNode)
                            {
                                AutotestingSystem::Instance()->Wait(timeNode->AsFloat());
                            }
                            else
                            {
                                Logger::Warning("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else if(actionName == "WaitForUI")
                        {
                            YamlNode* controlNameNode = actionNode->Get("controlName");
                            if(controlNameNode)
                            {
                                AutotestingSystem::Instance()->WaitForUI(controlNameNode->AsString());
                            }
                            else
                            {
                                Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else if(actionName == "KeyPress")
                        {
                            YamlNode* keyNode = actionNode->Get("key");
                            if(keyNode)
                            {
                                //TODO: test conversion from int to char16
                                AutotestingSystem::Instance()->KeyPress((char16)keyNode->AsInt());
                            }
                            else
                            {
                                Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else if(actionName == "KeyboardInput")
                        {
                            YamlNode* textNode = actionNode->Get("text");
                            if(textNode)
                            {
                                AutotestingSystem::Instance()->KeyboardInput(textNode->AsWString());
                            }
                            else
                            {
                                Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                            }
                        }
                        else
                        {
                            //TODO: asserts, getters

                            Logger::Error("TestsYamlParser::ParseYaml %s action %s ignored", yamlFilePath.c_str(), actionName.c_str());
                        }
                    }
                    else
                    {
                        Logger::Warning("TestsYamlParser::ParseYaml %s action ignored", yamlFilePath.c_str());
                    }
                }
            }
            else
            {
                Logger::Error("TestsYamlParser::ParseYaml %s failed - no actions node", yamlFilePath.c_str());
            }
        }
        else
        {
            Logger::Error("TestsYamlParser::ParseYaml %s failed - no root node", yamlFilePath.c_str());
        }
    }
    SafeRelease(parser);
}

};

#endif //__DAVAENGINE_AUTOTESTING__