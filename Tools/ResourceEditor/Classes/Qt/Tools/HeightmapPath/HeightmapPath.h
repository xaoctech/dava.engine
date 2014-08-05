#ifndef __HEIGHTMAPPATH_TOOL_H__
#define __HEIGHTMAPPATH_TOOL_H__

#include "DAVAEngine.h"

#include <QWidget>
#include <QScopedPointer>


namespace Ui { class HeightmapPath; }


class HeightmapPath
    : public QWidget
{
    Q_OBJECT

public:
    explicit HeightmapPath(QWidget *p = NULL);
    ~HeightmapPath();

    void SetDefaultDir(const QString& path);
    void SetOutputTemplate(const QString& prefix, const QString& suffix);

private slots:
    void OnBrowse();
    void OnRun();
    void OnValueChanged();

private:
    double GetThresholdInMeters(double unitSize);

    QScopedPointer<Ui::HeightmapPath> ui;
    QString defaultDir;
    QString outTemplate;
    QString inPath;
    QString outPath;
    QString outName;
};


#endif // __HEIGHTMAPPATH_TOOL_H__
