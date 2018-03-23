#pragma once

#include <TArc/Qt/QtString.h>

#include <QUrl>
#include <QMenuBar>

namespace DAVA
{
extern const QString ProjectMenuName;
extern const QString SceneMenuName;

QUrl CreateREMenuPoint(const QString& menuName);
void BuildMenuPrositionsFromLegacy(QMenuBar* menuBar);
} // namespace DAVA
