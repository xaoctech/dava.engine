#ifndef PATHBROWSER_H
#define PATHBROWSER_H


#include "LineEditEx.h"


class PathBrowser
    : public LineEditEx
{
    Q_OBJECT

public:
    explicit PathBrowser(QWidget *parent = NULL);
    ~PathBrowser();

    void SetHint(const QString& hint);
    void SetDefaultFolder(const QString& path);
    void SetPath(const QString& path);
    void SetFilter(const QString& filter);

    QSize sizeHint() const;

protected:
    QString DefaultBrowsePath();

private slots:
    void OnBrowse();

private:
    void InitActions();

    QAbstractButton * CreateButton( const QAction *action );
    QSize ButtonSizeHint(const QAction *action) const;

    QString hintText;
    QString defaultFolder;
    QString path;
    QString filter;
};


#endif // PATHBROWSER_H
