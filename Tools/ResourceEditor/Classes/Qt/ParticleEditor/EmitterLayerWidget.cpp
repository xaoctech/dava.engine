//
//  EmitterLayerWidget.cpp
//  ResourceEditorQt
//
//  Created by adebt on 11/26/12.
//
//

#include "EmitterLayerWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include "TextureBrowser/TextureConvertor.h"
#include "SceneEditor/EditorSettings.h"

#include <QHBoxLayout>
#include <QGraphicsWidget>
#include <QFileDialog>
#include <QFile>

#define SPRITE_SIZE 60

EmitterLayerWidget::EmitterLayerWidget(QWidget *parent) :
	QWidget(parent)
{
	mainBox = new QVBoxLayout;
	this->setLayout(mainBox);
	
	QHBoxLayout* spriteHBox = new QHBoxLayout;
	spriteLabel = new QLabel(this);
	spriteLabel->setMinimumSize(SPRITE_SIZE, SPRITE_SIZE);
	spriteHBox->addWidget(spriteLabel);
	mainBox->addLayout(spriteHBox);
	QVBoxLayout* spriteVBox = new QVBoxLayout;
	spriteHBox->addLayout(spriteVBox);
	QPushButton* spriteBtn = new QPushButton("Set sprite", this);
	spriteBtn->setMinimumHeight(30);
	spritePathLabel = new QLineEdit(this);
	spritePathLabel->setReadOnly(true);
	spriteVBox->addWidget(spriteBtn);
	spriteVBox->addWidget(spritePathLabel);
	connect(spriteBtn,
			SIGNAL(clicked(bool)),
			this,
			SLOT(OnSpriteBtn()));
	
	lifeTimeLine = new TimeLineWidget(this);
	InitWidget(lifeTimeLine);
	numberTimeLine = new TimeLineWidget(this);
	InitWidget(numberTimeLine);
	sizeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeTimeLine);
	sizeVariationTimeLine = new TimeLineWidget(this);
	InitWidget(sizeVariationTimeLine);
	sizeOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeOverLifeTimeLine);
	velocityTimeLine = new TimeLineWidget(this);
	InitWidget(velocityTimeLine);
	spinTimeLine = new TimeLineWidget(this);
	InitWidget(spinTimeLine);
	motionTimeLine = new TimeLineWidget(this);
	InitWidget(motionTimeLine);
	bounceTimeLine = new TimeLineWidget(this);
	InitWidget(bounceTimeLine);
	
	colorRandomGradient = new GradientPickerWidget(this);
	InitWidget(colorRandomGradient);
	colorOverLifeGradient = new GradientPickerWidget(this);
	InitWidget(colorOverLifeGradient);
	alphaOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(alphaOverLifeTimeLine);
	
	QHBoxLayout* alignToMotionLayout = new QHBoxLayout;
	mainBox->addLayout(alignToMotionLayout);
	alignToMotionLayout->addWidget(new QLabel("alignToMotion", this));
	alignToMotionSpin = new QDoubleSpinBox();
	alignToMotionLayout->addWidget(alignToMotionSpin);
	connect(alignToMotionSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	
	QHBoxLayout* startTimeHBox = new QHBoxLayout;
	startTimeHBox->addWidget(new QLabel("startTime", this));
	startTimeSpin = new QDoubleSpinBox(this);
	startTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	startTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	startTimeHBox->addWidget(startTimeSpin);
	mainBox->addLayout(startTimeHBox);
	connect(startTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));

	QHBoxLayout* endTimeHBox = new QHBoxLayout;
	endTimeHBox->addWidget(new QLabel("endTime", this));
	endTimeSpin = new QDoubleSpinBox(this);
	endTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	endTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	endTimeHBox->addWidget(endTimeSpin);
	mainBox->addLayout(endTimeHBox);
	connect(endTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	
	
	sprite = NULL;
	blockSignals = false;
}

EmitterLayerWidget::~EmitterLayerWidget()
{
	
}

void EmitterLayerWidget::InitWidget(QWidget* widget)
{
	mainBox->addWidget(widget);
	connect(widget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void EmitterLayerWidget::Init(ParticleEmitter* emitter, DAVA::ParticleLayer *layer, bool updateMinimized)
{
	if (!emitter || !layer)
		return;
	
	blockSignals = true;
	
	this->emitter = emitter;
	this->layer = layer;
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);
	
	//LAYER_SPRITE = 0,
	sprite = layer->GetSprite();
	Sprite* renderSprite = Sprite::CreateAsRenderTarget(SPRITE_SIZE, SPRITE_SIZE, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(renderSprite);
	if (sprite)
	{
		sprite->SetScaleSize(SPRITE_SIZE, SPRITE_SIZE);
		sprite->Draw();
	}

	RenderManager::Instance()->RestoreRenderTarget();
	Texture* texture = renderSprite->GetTexture();
	Image* image = texture->CreateImageFromMemory();
	spriteLabel->setPixmap(QPixmap::fromImage(TextureConvertor::fromDavaImage(image)));
	SafeRelease(image);
	SafeRelease(renderSprite);

	QString spriteName = sprite ? QString::fromStdString(sprite->GetName()) : "<none>";
	spritePathLabel->setText(spriteName);

	//LAYER_LIFE, LAYER_LIFE_VARIATION,
	lifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	lifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->life).GetProps(), Qt::blue, "life");
	lifeTimeLine->AddLine(1, PropLineWrapper<float32>(layer->lifeVariation).GetProps(), Qt::green, "VA");

	//LAYER_NUMBER, LAYER_NUMBER_VARIATION,
	numberTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	numberTimeLine->AddLine(0, PropLineWrapper<float32>(layer->number).GetProps(), Qt::blue, "number");
	numberTimeLine->AddLine(1, PropLineWrapper<float32>(layer->numberVariation).GetProps(), Qt::green, "VA");

	//LAYER_SIZE, LAYER_SIZE_VARIATION, LAYER_SIZE_OVER_LIFE,
	Vector<QColor> colors;
	colors.push_back(Qt::blue); colors.push_back(Qt::green);
	Vector<QString> legends;
	legends.push_back("size X"); legends.push_back("Y");
	sizeTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeTimeLine->AddLines(PropLineWrapper<Vector2>(layer->size).GetProps(), colors, legends);
	
	legends.clear();
	legends.push_back("size Variation X"); legends.push_back("Y");
	sizeVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeVariationTimeLine->AddLines(PropLineWrapper<Vector2>(layer->sizeVariation).GetProps(), colors, legends);
	
	sizeOverLifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	sizeOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->sizeOverLife).GetProps(), Qt::blue, "size over life");


	//LAYER_VELOCITY, LAYER_VELOCITY_VARIATION, LAYER_VELOCITY_OVER_LIFE,
	velocityTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	velocityTimeLine->AddLine(0, PropLineWrapper<float32>(layer->velocity).GetProps(), Qt::blue, "velocity");
	velocityTimeLine->AddLine(1, PropLineWrapper<float32>(layer->velocityVariation).GetProps(), Qt::green, "VA");
	velocityTimeLine->AddLine(2, PropLineWrapper<float32>(layer->velocityOverLife).GetProps(), Qt::red, "OL");

	//LAYER_FORCES, LAYER_FORCES_VARIATION, LAYER_FORCES_OVER_LIFE,

	//LAYER_SPIN, LAYER_SPIN_VARIATION, LAYER_SPIN_OVER_LIFE,
	spinTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	spinTimeLine->AddLine(0, PropLineWrapper<float32>(layer->spin).GetProps(), Qt::blue, "spin");
	spinTimeLine->AddLine(1, PropLineWrapper<float32>(layer->spinVariation).GetProps(), Qt::green, "VA");
	spinTimeLine->AddLine(2, PropLineWrapper<float32>(layer->spinOverLife).GetProps(), Qt::red, "OL");
	
	//LAYER_MOTION_RANDOM, LAYER_MOTION_RANDOM_VARIATION, LAYER_MOTION_RANDOM_OVER_LIFE,
	motionTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	motionTimeLine->AddLine(0, PropLineWrapper<float32>(layer->motionRandom).GetProps(), Qt::blue, "motionRandom");
	motionTimeLine->AddLine(1, PropLineWrapper<float32>(layer->motionRandomVariation).GetProps(), Qt::green, "VA");
	motionTimeLine->AddLine(2, PropLineWrapper<float32>(layer->motionRandomOverLife).GetProps(), Qt::red, "OL");

	//LAYER_BOUNCE, LAYER_BOUNCE_VARIATION,	LAYER_BOUNCE_OVER_LIFE,
	bounceTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	bounceTimeLine->AddLine(0, PropLineWrapper<float32>(layer->bounce).GetProps(), Qt::blue, "bounce");
	bounceTimeLine->AddLine(1, PropLineWrapper<float32>(layer->bounceVariation).GetProps(), Qt::green, "VA");
	bounceTimeLine->AddLine(2, PropLineWrapper<float32>(layer->bounceOverLife).GetProps(), Qt::red, "OL");
	
	//LAYER_COLOR_RANDOM, LAYER_ALPHA_OVER_LIFE, LAYER_COLOR_OVER_LIFE,
	colorRandomGradient->Init(0, 1, "Random color");
	colorRandomGradient->SetValues(PropLineWrapper<Color>(layer->colorRandom).GetProps());
	
	colorOverLifeGradient->Init(0, 1, "Color over life");
	colorOverLifeGradient->SetValues(PropLineWrapper<Color>(layer->colorOverLife).GetProps());

	alphaOverLifeTimeLine->Init(0, 1, updateMinimized);
	alphaOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->alphaOverLife).GetProps(), Qt::blue, "alpha over life");
	
	//LAYER_ALIGN_TO_MOTION,
	alignToMotionSpin->setValue(layer->alignToMotion);
	
	//LAYER_START_TIME, LAYER_END_TIME
	startTimeSpin->setValue(layer->startTime);
	endTimeSpin->setValue(layer->endTime);
	
	//, LAYER_IS_LONG
	
	blockSignals = false;
}

void EmitterLayerWidget::OnSpriteBtn()
{
	String projectPath1 = EditorSettings::Instance()->GetProjectPath();
	QString projectPath = QString::fromStdString(projectPath1);
	projectPath += "/Data/Particles/";
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle sprite"), projectPath, QString("Effect File (*.txt)"));
	if (filePath.isEmpty())
		return;
	
	filePath.remove(filePath.size() - 4, 4);
	Sprite* sprite = Sprite::Create(filePath.toStdString());
	if (!sprite)
		return;
	
	this->sprite = sprite;
	OnValueChanged();
}

void EmitterLayerWidget::OnValueChanged()
{
	if (blockSignals)
		return;
	
	PropLineWrapper<float32> propLife;
	PropLineWrapper<float32> propLifeVariation;
	lifeTimeLine->GetValue(0, propLife.GetPropsPtr());
	lifeTimeLine->GetValue(1, propLifeVariation.GetPropsPtr());
	
	PropLineWrapper<float32> propNumber;
	PropLineWrapper<float32> propNumberVariation;
	numberTimeLine->GetValue(0, propNumber.GetPropsPtr());
	numberTimeLine->GetValue(1, propNumberVariation.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSize;
	sizeTimeLine->GetValues(propSize.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSizeVariation;
	sizeVariationTimeLine->GetValues(propSizeVariation.GetPropsPtr());

	PropLineWrapper<float32> propsizeOverLife;
	sizeOverLifeTimeLine->GetValue(0, propsizeOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propVelocity;
	PropLineWrapper<float32> propVelocityVariation;
	PropLineWrapper<float32> propVelocityOverLife;
	velocityTimeLine->GetValue(0, propVelocity.GetPropsPtr());
	velocityTimeLine->GetValue(1, propVelocityVariation.GetPropsPtr());
	velocityTimeLine->GetValue(2, propVelocityOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propSpin;
	PropLineWrapper<float32> propSpinVariation;
	PropLineWrapper<float32> propSpinOverLife;
	spinTimeLine->GetValue(0, propSpin.GetPropsPtr());
	spinTimeLine->GetValue(1, propSpinVariation.GetPropsPtr());
	spinTimeLine->GetValue(2, propSpinOverLife.GetPropsPtr());

	PropLineWrapper<float32> propMotion;
	PropLineWrapper<float32> propMotionVariation;
	PropLineWrapper<float32> propMotionOverLife;
	motionTimeLine->GetValue(0, propMotion.GetPropsPtr());
	motionTimeLine->GetValue(1, propMotionVariation.GetPropsPtr());
	motionTimeLine->GetValue(2, propMotionOverLife.GetPropsPtr());

	PropLineWrapper<float32> propBounce;
	PropLineWrapper<float32> propBounceVariation;
	PropLineWrapper<float32> propBounceOverLife;
	bounceTimeLine->GetValue(0, propBounce.GetPropsPtr());
	bounceTimeLine->GetValue(1, propBounceVariation.GetPropsPtr());
	bounceTimeLine->GetValue(2, propBounceOverLife.GetPropsPtr());

	PropLineWrapper<Color> propColorRandom;
	colorRandomGradient->GetValues(propColorRandom.GetPropsPtr());

	PropLineWrapper<Color> propColorOverLife;
	colorOverLifeGradient->GetValues(propColorOverLife.GetPropsPtr());

	PropLineWrapper<float32> propAlphaOverLife;
	alphaOverLifeTimeLine->GetValue(0, propAlphaOverLife.GetPropsPtr());
	
	CommandUpdateParticleLayer* updateLayerCmd = new CommandUpdateParticleLayer(layer);
	updateLayerCmd->Init(sprite,
						 propLife.GetPropLine(),
						 propLifeVariation.GetPropLine(),
						 propNumber.GetPropLine(),
						 propNumberVariation.GetPropLine(),
						 propSize.GetPropLine(),
						 propSizeVariation.GetPropLine(),
						 propsizeOverLife.GetPropLine(),
						 propVelocity.GetPropLine(),
						 propVelocityVariation.GetPropLine(),
						 propVelocityOverLife.GetPropLine(),
						 propSpin.GetPropLine(),
						 propSpinVariation.GetPropLine(),
						 propSpinOverLife.GetPropLine(),
						 propMotion.GetPropLine(),
						 propMotionVariation.GetPropLine(),
						 propMotionOverLife.GetPropLine(),
						 propBounce.GetPropLine(),
						 propBounceVariation.GetPropLine(),
						 propBounceOverLife.GetPropLine(),
						 propColorRandom.GetPropLine(),
						 propAlphaOverLife.GetPropLine(),
						 propColorOverLife.GetPropLine(),
						 (float32)alignToMotionSpin->value(),
						 (float32)startTimeSpin->value(),
						 (float32)endTimeSpin->value());
	
	CommandsManager::Instance()->Execute(updateLayerCmd);
	SafeRelease(updateLayerCmd);
	
	Init(this->emitter, this->layer, false);
}