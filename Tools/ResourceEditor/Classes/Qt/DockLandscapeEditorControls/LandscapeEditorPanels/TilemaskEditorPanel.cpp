#include "TilemaskEditorPanel.h"
#include "../../Scene/SceneSignals.h"
#include "../../Scene/SceneEditor2.h"
#include "../../SliderWidget/SliderWidget.h"
#include "Constants.h"
#include "TextureBrowser/TextureConvertor.h"
#include "../LandscapeEditorShortcutManager.h"

#include <QLayout>
#include <QComboBox>

TilemaskEditorPanel::TilemaskEditorPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
,	sliderWidgetBrushSize(NULL)
,	sliderWidgetStrength(NULL)
,	comboBrushImage(NULL)
,	comboTileTexture(NULL)
{
	InitUI();
	ConnectToSignals();
}

TilemaskEditorPanel::~TilemaskEditorPanel()
{
}

bool TilemaskEditorPanel::GetEditorEnabled()
{
	return GetActiveScene()->tilemaskEditorSystem->IsLandscapeEditingEnabled();
}

void TilemaskEditorPanel::SetWidgetsState(bool enabled)
{
	sliderWidgetBrushSize->setEnabled(enabled);
	sliderWidgetStrength->setEnabled(enabled);
	comboBrushImage->setEnabled(enabled);
	comboTileTexture->setEnabled(enabled);
}

void TilemaskEditorPanel::BlockAllSignals(bool block)
{
	sliderWidgetBrushSize->blockSignals(block);
	sliderWidgetStrength->blockSignals(block);
	comboBrushImage->blockSignals(block);
	comboTileTexture->blockSignals(block);
}

void TilemaskEditorPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	sliderWidgetBrushSize = new SliderWidget(this);
	sliderWidgetStrength = new SliderWidget(this);
	comboBrushImage = new QComboBox(this);
	comboTileTexture = new QComboBox(this);

	QLabel* labelBrushImageDesc = new QLabel(this);
	QLabel* labelTileTextureDesc = new QLabel(this);
	QFrame* frameBrushImage = new QFrame(this);
	QFrame* frameTileTexture = new QFrame(this);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout* layoutBrushImage = new QHBoxLayout();
	QHBoxLayout* layoutTileTexture = new QHBoxLayout();

	layoutBrushImage->addWidget(labelBrushImageDesc);
	layoutBrushImage->addWidget(comboBrushImage);
	layoutTileTexture->addWidget(labelTileTextureDesc);
	layoutTileTexture->addWidget(comboTileTexture);

	frameBrushImage->setLayout(layoutBrushImage);
	frameTileTexture->setLayout(layoutTileTexture);

	layout->addWidget(sliderWidgetBrushSize);
	layout->addWidget(frameBrushImage);
	layout->addWidget(sliderWidgetStrength);
	layout->addWidget(frameTileTexture);
	layout->addSpacerItem(spacer);

	setLayout(layout);

	SetWidgetsState(false);
	BlockAllSignals(true);

	comboBrushImage->setMinimumHeight(44);
	comboTileTexture->setMinimumHeight(44);
	labelBrushImageDesc->setText(ResourceEditor::TILEMASK_EDITOR_BRUSH_IMAGE_CAPTION.c_str());
	labelBrushImageDesc->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	labelTileTextureDesc->setText(ResourceEditor::TILEMASK_EDITOR_TILE_TEXTURE_CAPTION.c_str());
	labelTileTextureDesc->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	sliderWidgetBrushSize->Init(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_CAPTION.c_str(),
								false, DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);
	sliderWidgetStrength->Init(ResourceEditor::TILEMASK_EDITOR_STRENGTH_CAPTION.c_str(),
							   false, DEF_STRENGTH_MAX_VALUE, DEF_STRENGTH_MIN_VALUE, DEF_STRENGTH_MIN_VALUE);

	layoutBrushImage->setContentsMargins(0, 0, 0, 0);
	layoutTileTexture->setContentsMargins(0, 0, 0, 0);

	InitBrushImages();
}

void TilemaskEditorPanel::ConnectToSignals()
{
	connect(SceneSignals::Instance(), SIGNAL(TilemaskEditorToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(sliderWidgetStrength, SIGNAL(ValueChanged(int)), this, SLOT(SetStrength(int)));
	connect(comboBrushImage, SIGNAL(currentIndexChanged(int)), this, SLOT(SetToolImage(int)));
	connect(comboTileTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDrawTexture(int)));
}

void TilemaskEditorPanel::StoreState()
{
	KeyedArchive* customProperties = GetActiveScene()->GetCustomProperties();
	if (customProperties)
	{
		customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MIN,
								   (int32)sliderWidgetBrushSize->GetRangeMin());
		customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MAX,
								   (int32)sliderWidgetBrushSize->GetRangeMax());
		customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MIN,
								   (int32)sliderWidgetStrength->GetRangeMin());
		customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MAX,
								   (int32)sliderWidgetStrength->GetRangeMax());
	}
}

void TilemaskEditorPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();

	bool enabled = sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled();
	int32 brushSize = BrushSizeSystemToUI(sceneEditor->tilemaskEditorSystem->GetBrushSize());
	int32 strength = StrengthSystemToUI(sceneEditor->tilemaskEditorSystem->GetStrength());
	uint32 tileTexture = sceneEditor->tilemaskEditorSystem->GetTileTextureIndex();
	int32 toolImage = sceneEditor->tilemaskEditorSystem->GetToolImage();

	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;
	int32 strRangeMin = DEF_STRENGTH_MIN_VALUE;
	int32 strRangeMax = DEF_STRENGTH_MAX_VALUE;

	KeyedArchive* customProperties = sceneEditor->GetCustomProperties();
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MIN, DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MAX, DEF_BRUSH_MAX_SIZE);
		strRangeMin = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MIN, DEF_STRENGTH_MIN_VALUE);
		strRangeMax = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MAX, DEF_STRENGTH_MAX_VALUE);
	}

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	sliderWidgetBrushSize->SetValue(brushSize);

	sliderWidgetStrength->SetRangeMin(strRangeMin);
	sliderWidgetStrength->SetRangeMax(strRangeMax);
	sliderWidgetStrength->SetValue(strength);

	comboTileTexture->setCurrentIndex(tileTexture);
	comboBrushImage->setCurrentIndex(toolImage);
	BlockAllSignals(!enabled);
}

void TilemaskEditorPanel::OnEditorEnabled()
{
	UpdateTileTextures();

	SetToolImage(comboBrushImage->currentIndex());
}

void TilemaskEditorPanel::InitBrushImages()
{
	comboBrushImage->clear();

	QSize iconSize = comboBrushImage->iconSize();
	iconSize = iconSize.expandedTo(QSize(32, 32));
	comboBrushImage->setIconSize(iconSize);

	FilePath toolsPath(ResourceEditor::TILEMASK_EDITOR_TOOLS_PATH);
	FileList *fileList = new FileList(toolsPath);
	for(int32 iFile = 0; iFile < fileList->GetCount(); ++iFile)
	{
		String filename = fileList->GetFilename(iFile);
		if(fileList->GetPathname(iFile).IsEqualToExtension(".png"))
		{
			String fullname = fileList->GetPathname(iFile).GetAbsolutePathname();

			FilePath f = fileList->GetPathname(iFile);
			f.ReplaceExtension("");

			QString qFullname = QString::fromStdString(fullname);
			QIcon toolIcon(qFullname);
			comboBrushImage->addItem(toolIcon, f.GetFilename().c_str(), QVariant(qFullname));
		}
	}

	SafeRelease(fileList);
}

void TilemaskEditorPanel::SplitImageToChannels(Image* image, Image*& r, Image*& g, Image*& b, Image*& a)
{
	DVASSERT(image->GetPixelFormat() == FORMAT_RGBA8888)

	const int32 CHANNELS_COUNT = 4;

	uint32 width = image->GetWidth();
	uint32 height = image->GetHeight();
	int32 size = width * height;

	Image* images[CHANNELS_COUNT];
	for (int32 i = 0; i < CHANNELS_COUNT; ++i)
	{
		images[i] = Image::Create(width, height, FORMAT_A8);
	}

	int32 pixelSize = Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
	for(int32 i = 0; i < size; ++i)
	{
		int32 offset = i * pixelSize;
		images[0]->data[i] = image->data[offset];
		images[1]->data[i] = image->data[offset + 1];
		images[2]->data[i] = image->data[offset + 2];
		images[3]->data[i] = image->data[offset + 3];
	}

	r = images[0];
	g = images[1];
	b = images[2];
	a = images[3];
}

void TilemaskEditorPanel::UpdateTileTextures()
{
	SceneEditor2* sceneEditor = GetActiveScene();

	comboTileTexture->clear();

	QSize iconSize = comboTileTexture->iconSize();
	iconSize = iconSize.expandedTo(QSize(150, 32));
	comboTileTexture->setIconSize(iconSize);

	int32 count = (int32)sceneEditor->tilemaskEditorSystem->GetTileTextureCount();
	Image** images = new Image*[count];

	if (sceneEditor->landscapeEditorDrawSystem->GetLandscapeTiledShaderMode() == Landscape::TILED_MODE_TILE_DETAIL_MASK)
	{
		Image* image = sceneEditor->tilemaskEditorSystem->GetTileTexture(0)->CreateImageFromMemory();
		image->ResizeCanvas(iconSize.width(), iconSize.height());

		SplitImageToChannels(image, images[0], images[1], images[2], images[3]);
	}
	else
	{
		for (int32 i = 0; i < count; ++i)
		{
			Texture* texture = sceneEditor->tilemaskEditorSystem->GetTileTexture(i);
			images[i] = texture->CreateImageFromMemory();
			images[i]->ResizeCanvas(iconSize.width(), iconSize.height());
		}
	}

	for (int32 i = 0; i < count; ++i)
	{
		QImage img = TextureConvertor::FromDavaImage(images[i]);
		QIcon icon = QIcon(QPixmap::fromImage(img));

		comboTileTexture->addItem(icon, "");

		SafeRelease(images[i]);
	}

	SafeDelete(images);
}

void TilemaskEditorPanel::SetBrushSize(int brushSize)
{
	GetActiveScene()->tilemaskEditorSystem->SetBrushSize(BrushSizeUIToSystem(brushSize));
}

void TilemaskEditorPanel::SetToolImage(int imageIndex)
{
	QString s = comboBrushImage->itemData(imageIndex).toString();
	
	if (!s.isEmpty())
	{
		FilePath fp(s.toStdString());
		GetActiveScene()->tilemaskEditorSystem->SetToolImage(fp, imageIndex);
	}
}

void TilemaskEditorPanel::SetStrength(int strength)
{
	GetActiveScene()->tilemaskEditorSystem->SetStrength(StrengthUIToSystem(strength));
}

void TilemaskEditorPanel::SetDrawTexture(int textureIndex)
{
	GetActiveScene()->tilemaskEditorSystem->SetTileTexture(textureIndex);
}

// these functions are designed to convert values from sliders in ui
// to the values suitable for tilemask editor system
int32 TilemaskEditorPanel::BrushSizeUIToSystem(int32 uiValue)
{
	int32 systemValue = uiValue * ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return systemValue;
}

int32 TilemaskEditorPanel::BrushSizeSystemToUI(int32 systemValue)
{
	int32 uiValue = systemValue / ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return uiValue;
}

float32 TilemaskEditorPanel::StrengthUIToSystem(int32 uiValue)
{
	// strength in tilemask editor is the real number in the range [0 .. 0.5]
	// (by default, upper bound could be different)
	float32 max = 2.0 * DEF_STRENGTH_MAX_VALUE;
	float32 systemValue = uiValue / max;
	return systemValue;
}

int32 TilemaskEditorPanel::StrengthSystemToUI(float32 systemValue)
{
	int32 uiValue = (int32)(systemValue * 2.f * DEF_STRENGTH_MAX_VALUE);
	return uiValue;
}
// end of convert functions ==========================

void TilemaskEditorPanel::ConnectToShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSizeLarge()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSizeLarge()));

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_SMALL), SIGNAL(activated()),
			this, SLOT(IncreaseStrength()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_SMALL), SIGNAL(activated()),
			this, SLOT(DecreaseStrength()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_LARGE), SIGNAL(activated()),
			this, SLOT(IncreaseStrengthLarge()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_LARGE), SIGNAL(activated()),
			this, SLOT(DecreaseStrengthLarge()));

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_NEXT), SIGNAL(activated()),
			this, SLOT(NextTexture()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_PREV), SIGNAL(activated()),
			this, SLOT(PrevTexture()));

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_IMAGE_NEXT), SIGNAL(activated()),
			this, SLOT(NextTool()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_IMAGE_PREV), SIGNAL(activated()),
			this, SLOT(PrevTool()));
}

void TilemaskEditorPanel::DisconnectFromShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(IncreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(DecreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(IncreaseBrushSizeLarge()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(DecreaseBrushSizeLarge()));

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(IncreaseStrength()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(DecreaseStrength()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_INCREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(IncreaseStrengthLarge()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_STRENGTH_DECREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(DecreaseStrengthLarge()));

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_NEXT), SIGNAL(activated()),
			   this, SLOT(NextTexture()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_PREV), SIGNAL(activated()),
			   this, SLOT(PrevTexture()));

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_IMAGE_NEXT), SIGNAL(activated()),
			   this, SLOT(NextTool()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_IMAGE_PREV), SIGNAL(activated()),
			   this, SLOT(PrevTool()));
}

void TilemaskEditorPanel::IncreaseBrushSize()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void TilemaskEditorPanel::DecreaseBrushSize()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void TilemaskEditorPanel::IncreaseBrushSizeLarge()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void TilemaskEditorPanel::DecreaseBrushSizeLarge()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void TilemaskEditorPanel::IncreaseStrength()
{
	sliderWidgetStrength->SetValue(sliderWidgetStrength->GetValue()
								   + ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void TilemaskEditorPanel::DecreaseStrength()
{
	sliderWidgetStrength->SetValue(sliderWidgetStrength->GetValue()
								   - ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void TilemaskEditorPanel::IncreaseStrengthLarge()
{
	sliderWidgetStrength->SetValue(sliderWidgetStrength->GetValue()
								   + ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void TilemaskEditorPanel::DecreaseStrengthLarge()
{
	sliderWidgetStrength->SetValue(sliderWidgetStrength->GetValue()
								   - ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void TilemaskEditorPanel::PrevTexture()
{
	int32 curIndex = comboTileTexture->currentIndex();
	if (curIndex)
	{
		comboTileTexture->setCurrentIndex(curIndex - 1);
	}
}

void TilemaskEditorPanel::NextTexture()
{
	int32 curIndex = comboTileTexture->currentIndex();
	if (curIndex < comboTileTexture->count() - 1)
	{
		comboTileTexture->setCurrentIndex(curIndex + 1);
	}
}

void TilemaskEditorPanel::PrevTool()
{
	int32 curIndex = comboBrushImage->currentIndex();
	if (curIndex)
	{
		comboBrushImage->setCurrentIndex(curIndex - 1);
	}
}

void TilemaskEditorPanel::NextTool()
{
	int32 curIndex = comboBrushImage->currentIndex();
	if (curIndex < comboBrushImage->count() - 1)
	{
		comboBrushImage->setCurrentIndex(curIndex + 1);
	}
}
