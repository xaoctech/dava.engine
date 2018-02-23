#include "TileTexturePreviewWidget.h"
#include "TileTexturePreviewWidgetItemDelegate.h"

#include "Classes/Qt/TextureBrowser/TextureConvertor.h"

#include <REPlatform/Global/StringConstants.h>
#include <REPlatform/Scene/Utils/ImageTools.h>

#include <TArc/Controls/ColorPicker/ColorPickerDialog.h>
#include <TArc/Core/Deprecated.h>
#include <TArc/Utils/Utils.h>

#include <QHeaderView>
#include <QLabel>
#include <QEvent>

TileTexturePreviewWidget::TileTexturePreviewWidget(QWidget* parent)
    : QTreeWidget(parent)
    , selectedTexture(0)
    , validator(NULL)
{
    images.reserve(DEF_TILE_TEXTURES_COUNT);

    Init();
    ConnectToSignals();

    validator = new QRegExpValidator();
    validator->setRegExp(QRegExp(TileTexturePreviewWidgetItemDelegate::TILE_COLOR_VALIDATE_REGEXP, Qt::CaseInsensitive));
}

TileTexturePreviewWidget::~TileTexturePreviewWidget()
{
    Clear();

    if (itemDelegate != nullptr)
    {
        setItemDelegate(nullptr);
        DAVA::SafeDelete(itemDelegate);
    }

    DAVA::SafeDelete(validator);
}

void TileTexturePreviewWidget::Clear()
{
    clear();

    for (auto& image : images)
    {
        SafeRelease(image);
    }

    images.clear();
}

void TileTexturePreviewWidget::AddTexture(DAVA::Image* previewTexture)
{
    DVASSERT(previewTexture->GetPixelFormat() == DAVA::FORMAT_RGBA8888);

    bool blocked = signalsBlocked();
    blockSignals(true);

    images.push_back(SafeRetain(previewTexture));

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setCheckState(0, Qt::Unchecked);
    addTopLevelItem(item);

    item->setFlags((item->flags() | Qt::ItemIsUserCheckable) & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable));

    UpdateImage(static_cast<DAVA::uint32>(images.size() - 1));
    UpdateSelection();

    blockSignals(blocked);
}

void TileTexturePreviewWidget::Init()
{
    bool blocked = signalsBlocked();
    blockSignals(true);

    Clear();

    setHeaderHidden(true);
    setColumnCount(1);
    setRootIsDecorated(false);
    setIconSize(QSize(TEXTURE_PREVIEW_WIDTH, TEXTURE_PREVIEW_HEIGHT));

    blockSignals(blocked);
}

void TileTexturePreviewWidget::ConnectToSignals()
{
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(OnCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnItemChanged(QTreeWidgetItem*, int)));
}

DAVA::uint32 TileTexturePreviewWidget::GetSelectedTexture()
{
    return selectedTexture;
}

void TileTexturePreviewWidget::SetSelectedTexture(DAVA::uint32 number)
{
    if (number < static_cast<DAVA::uint32>(images.size()))
    {
        selectedTexture = number;
        UpdateSelection();

        emit SelectionChanged(selectedTexture);
    }
}

void TileTexturePreviewWidget::UpdateImage(DAVA::uint32 number)
{
    DVASSERT(number < static_cast<DAVA::uint32>(images.size()));

    QTreeWidgetItem* item = topLevelItem(number);

    QImage qimg = DAVA::ImageTools::FromDavaImage(images[number]);

    QSize size = QSize(TEXTURE_PREVIEW_WIDTH, TEXTURE_PREVIEW_HEIGHT);
    QImage previewImage = qimg.copy(0, 0, size.width(), size.height());
    item->setIcon(0, QIcon(QPixmap::fromImage(previewImage)));
}

void TileTexturePreviewWidget::UpdateSelection()
{
    for (DAVA::int32 i = 0; i < static_cast<DAVA::int32>(images.size()); ++i)
    {
        QTreeWidgetItem* item = topLevelItem(i);
        item->setCheckState(0, (i == selectedTexture ? Qt::Checked : Qt::Unchecked));
    }
}

void TileTexturePreviewWidget::OnCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    QModelIndex index = currentIndex();
    selectedTexture = index.row();
    UpdateSelection();

    emit SelectionChanged(selectedTexture);
}

void TileTexturePreviewWidget::OnItemChanged(QTreeWidgetItem* item, int column)
{
    DAVA::int32 index = indexOfTopLevelItem(item);

    if (item->checkState(0) == Qt::Checked)
    {
        SetSelectedTexture(index);
    }
    else
    {
        UpdateSelection();
    }
}

bool TileTexturePreviewWidget::eventFilter(QObject* obj, QEvent* ev)
{
    return QObject::eventFilter(obj, ev);
}

DAVA::Image* TileTexturePreviewWidget::MultiplyImageWithColor(DAVA::Image* image, const DAVA::Color& color)
{
    DVASSERT(image->GetPixelFormat() == DAVA::FORMAT_RGBA8888);

    DAVA::Image* newImage = DAVA::Image::Create(image->GetWidth(), image->GetHeight(), image->GetPixelFormat());

    DAVA::uint32* imageData = reinterpret_cast<DAVA::uint32*>(image->GetData());
    DAVA::uint32* newImageData = reinterpret_cast<DAVA::uint32*>(newImage->GetData());

    DAVA::int32 pixelsCount = image->dataSize / sizeof(DAVA::uint32);

    for (DAVA::int32 i = 0; i < pixelsCount; ++i)
    {
        DAVA::uint8* pixelData = reinterpret_cast<DAVA::uint8*>(imageData);
        DAVA::uint8* newPixelData = reinterpret_cast<DAVA::uint8*>(newImageData);

        newPixelData[0] = static_cast<DAVA::uint8>(floorf(pixelData[0] * color.r));
        newPixelData[1] = static_cast<DAVA::uint8>(floorf(pixelData[1] * color.g));
        newPixelData[2] = static_cast<DAVA::uint8>(floorf(pixelData[2] * color.b));
        newPixelData[3] = 255;

        ++imageData;
        ++newImageData;
    }

    return newImage;
}
