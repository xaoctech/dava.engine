#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "ui_BeastDialog.h"

#include <Beast/BeastConstants.h>

#include <QWidget>
#include <QScopedPointer>
#include <QPointer>

namespace DAVA
{
class SceneEditor2;
} // namespace DAVA
class QEventLoop;

class BeastDialog : public QWidget
{
    Q_OBJECT

public:
    BeastDialog(QWidget* parent = 0);
    ~BeastDialog();

    void SetScene(DAVA::SceneEditor2* scene);
    bool Exec(QWidget* parent = 0);
    QString GetPath() const;
    Beast::eBeastMode GetMode() const;

private slots:
    void OnStart();
    void OnCancel();
    void OnBrowse();
    void OnTextChanged();
    void OnLightmapMode(bool checked);
    void OnSHMode(bool checked);
    void OnPreviewMode(bool checked);

private:
    void closeEvent(QCloseEvent* event);
    QString GetDefaultPath() const;
    void SetPath(const QString& path);

    QScopedPointer<Ui::BeastDialog> ui;
    QPointer<QEventLoop> loop;
    DAVA::SceneEditor2* scene;
    bool result;

    Beast::eBeastMode beastMode = Beast::eBeastMode::MODE_LIGHTMAPS;
};

#endif //#if defined(__DAVAENGINE_BEAST__)
