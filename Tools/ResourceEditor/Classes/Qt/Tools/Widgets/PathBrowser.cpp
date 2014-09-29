#include "PathBrowser.h"

#include <QAction>
#include <QAbstractButton>
#include <QDebug>
#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>


namespace
{
    const QSize cButtonSize = QSize(24, 24);
}


PathBrowser::PathBrowser(QWidget* parent)
    : LineEditEx(parent)
{
    InitActions();

    QBoxLayout *l = qobject_cast<QBoxLayout *>( layout() );
    if ( l != NULL )
    {
        l->setContentsMargins( 2, 2, 2, 2 );
    }
}

PathBrowser::~PathBrowser()
{
}

void PathBrowser::SetHint(const QString& hint)
{
    hintText = hint;
    setPlaceholderText(hintText);
}

void PathBrowser::SetDefaultFolder(const QString& _path)
{
    defaultFolder = _path;
}

void PathBrowser::SetPath(const QString& _path)
{
    path = _path;
}

void PathBrowser::SetFilter(const QString& _filter)
{
    filter = _filter;
}

QSize PathBrowser::sizeHint() const
{
    QSize hint = LineEditEx::sizeHint();
    const bool hasActions = !actions().isEmpty();

    if (hasActions)
    {
        hint.rheight() = cButtonSize.height();

        QBoxLayout *l = qobject_cast<QBoxLayout *>( layout() );
        if ( l != NULL )
        {
            int top, left, right, bottom;
            l->getContentsMargins( &left, &top, &right, &bottom );
            hint.rheight() += top + bottom;
        }
    }

    return hint;
}

QString PathBrowser::DefaultBrowsePath()
{
    const QFileInfo pathInfo(path);
    const QFileInfo defaultInfo(defaultFolder);

    if (pathInfo.isFile())
        return path;

    if (defaultInfo.isDir())
        return defaultFolder;

    return QString();
}

void PathBrowser::OnBrowse()
{
    const QString newPath = QFileDialog::getOpenFileName( this, hintText, DefaultBrowsePath(), filter, NULL, 0 );
}

void PathBrowser::InitActions()
{
    QAction *browse = new QAction(this);
    browse->setToolTip("Browse...");
    browse->setIcon(QIcon(":/QtIcons/openscene.png"));
    connect( browse, SIGNAL( triggered() ), SLOT( OnBrowse() ) );

    addAction(browse);
}

QAbstractButton* PathBrowser::CreateButton(QAction const* action)
{
    QAbstractButton *btn = LineEditEx::CreateButton(action);
    return btn;
}

QSize PathBrowser::ButtonSizeHint(const QAction * action) const
{
    Q_UNUSED(action);
    return cButtonSize;
}
