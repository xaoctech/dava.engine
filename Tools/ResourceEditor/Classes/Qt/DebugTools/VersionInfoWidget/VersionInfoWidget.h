#ifndef DEBUG_VERSION_INFO_WIDGET_H
#define DEBUG_VERSION_INFO_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include "Scene3D/SceneFile/VersionInfo.h"


namespace Ui {
    class VersionInfoWidget;
} // namespace Ui


class VersionInfoWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit VersionInfoWidget(QWidget *parent);
    ~VersionInfoWidget();

private:
    QScopedPointer<Ui::VersionInfoWidget> ui;
};


#endif
