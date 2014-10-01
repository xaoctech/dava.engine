//
//  Project.h
//  UIEditor
//
//  Created by Dmitry Belsky on 11.9.14.
//
//

#ifndef __UI_EDITOR_PROJECT_H__
#define __UI_EDITOR_PROJECT_H__

#include <QObject>
#include "Base/BaseTypes.h"

namespace DAVA {
    class UIPackage;
    class LegacyControlData;
}

class Project : QObject
{
    Q_OBJECT
public:
    Project();
    virtual ~Project();

    bool Open(const QString &path);

    DAVA::UIPackage *NewPackage(const QString &path){return NULL;}
    DAVA::UIPackage *OpenPackage(const QString &path);
    bool SavePackage(DAVA::UIPackage *package);
    bool SaveAsPackage(DAVA::UIPackage *package){return false;}

signals:
    void ProjectOpened();

private:
    DAVA::LegacyControlData *GetLegacyControlData(const DAVA::String &fwPath);

private:
    QString projectFile;
    QString projectDir;
    
    DAVA::Map<DAVA::String, DAVA::LegacyControlData> legacyDataMap;
};

#endif // __UI_EDITOR_PROJECT_H__
