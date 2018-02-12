#ifndef __RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__
#define __RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__

#include "DAVAEngine.h"

#include <QTreeWidget>

class QLabel;

class TileTexturePreviewWidgetItemDelegate;
class TileTexturePreviewWidget : public QTreeWidget
{
    Q_OBJECT

public:
    static const DAVA::int32 TEXTURE_PREVIEW_WIDTH = 180;
    static const DAVA::int32 TEXTURE_PREVIEW_HEIGHT = 32;

    explicit TileTexturePreviewWidget(QWidget* parent = 0);
    ~TileTexturePreviewWidget();

    void AddTexture(DAVA::Image* previewTexture);

    DAVA::uint32 GetSelectedTexture();
    void SetSelectedTexture(DAVA::uint32 number);

    void Clear();

protected:
    virtual bool eventFilter(QObject* obj, QEvent* ev);

signals:
    void SelectionChanged(int selectedTexture);
    void TileColorChanged(DAVA::int32 tileNumber, DAVA::Color color);

private slots:
    void OnCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void OnItemChanged(QTreeWidgetItem* item, int column);

private:
    static const DAVA::int32 COLOR_PREVIEW_COLUMN = 1;
    static const DAVA::int32 COLOR_SELECT_BUTTON_COLUMN = 2;
    static const DAVA::int32 DEF_TILE_TEXTURES_COUNT = 4;

    DAVA::int32 selectedTexture;

    DAVA::Vector<DAVA::Image*> images;

    QRegExpValidator* validator;

    void ConnectToSignals();

    void UpdateImage(DAVA::uint32 number);
    void UpdateSelection();

    void Init();

    DAVA::Image* MultiplyImageWithColor(DAVA::Image* image, const DAVA::Color& color);
    TileTexturePreviewWidgetItemDelegate* itemDelegate = nullptr;
};

#endif /* defined(__RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__) */
