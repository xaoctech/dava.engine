#include "ParticleEmitterPropertiesWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include <QLabel>
#include <QEvent>

ParticleEmitterPropertiesWidget::ParticleEmitterPropertiesWidget(QWidget* parent) :
	QWidget(parent)
{
	mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);

	QHBoxLayout* emitterTypeHBox = new QHBoxLayout();
	emitterTypeHBox->addWidget(new QLabel("type"));
	emitterType = new QComboBox(this);
	emitterType->addItem("Point");
	emitterType->addItem("Line");
	emitterType->addItem("Rect");
	emitterType->addItem("Oncircle");
	emitterTypeHBox->addWidget(emitterType);
	mainLayout->addLayout(emitterTypeHBox);
	connect(emitterType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnValueChanged()));

	emitterEmissionAngle = new TimeLineWidget(this);
	InitWidget(emitterEmissionAngle);

	emitterEmissionRange = new TimeLineWidget(this);
	InitWidget(emitterEmissionRange);

	emitterEmissionVector = new TimeLineWidget(this);
	InitWidget(emitterEmissionVector);

	emitterRadius = new TimeLineWidget(this);
	InitWidget(emitterRadius);

	emitterColorWidget = new GradientPickerWidget(this);
	InitWidget(emitterColorWidget);

	emitterSize = new TimeLineWidget(this);
	InitWidget(emitterSize);

	QHBoxLayout *emitterLifeHBox = new QHBoxLayout();
	emitterLifeHBox->addWidget(new QLabel("life"));
	emitterLife = new QDoubleSpinBox(this);
	emitterLife->setMinimum(0.f);
	emitterLife->setMaximum(std::numeric_limits<float32>::infinity());
	emitterLifeHBox->addWidget(emitterLife);
	mainLayout->addLayout(emitterLifeHBox);
	connect(emitterLife, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
	
	Q_FOREACH( QAbstractSpinBox * sp, findChildren<QAbstractSpinBox*>() ) {
        sp->installEventFilter( this );
    }
	
	blockSignals = false;
}

ParticleEmitterPropertiesWidget::~ParticleEmitterPropertiesWidget()
{
}

void ParticleEmitterPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEmitterPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;

	DVASSERT(emitter != 0);

	DVASSERT(emitterType->currentIndex() != -1);
	ParticleEmitter::eType type = (ParticleEmitter::eType)emitterType->currentIndex();

	PropLineWrapper<float32> emissionAngle;
	if(!emitterEmissionAngle->GetValue(0, emissionAngle.GetPropsPtr()))
		return;

	PropLineWrapper<float32> emissionRange;
	if(!emitterEmissionRange->GetValue(0, emissionRange.GetPropsPtr()))
		return;

	PropLineWrapper<Vector3> emissionVector;
	if(!emitterEmissionVector->GetValues(emissionVector.GetPropsPtr()))
		return;

	PropLineWrapper<float32> radius;
	if(!emitterRadius->GetValue(0, radius.GetPropsPtr()))
		return;

	PropLineWrapper<Color> colorOverLife;
	if(!emitterColorWidget->GetValues(colorOverLife.GetPropsPtr()))
		return;

	PropLineWrapper<Vector3> size;
	if(!emitterSize->GetValues(size.GetPropsPtr()))
		return;

	float32 life = emitterLife->value();
	
	CommandUpdateEmitter* commandUpdateEmitter = new CommandUpdateEmitter(emitter);
	commandUpdateEmitter->Init(type,
							   emissionAngle.GetPropLine(),
							   emissionRange.GetPropLine(),
							   emissionVector.GetPropLine(),
							   radius.GetPropLine(),
							   colorOverLife.GetPropLine(),
							   size.GetPropLine(),
							   life);
	CommandsManager::Instance()->Execute(commandUpdateEmitter);
	SafeRelease(commandUpdateEmitter);
	
	Init(emitter, false);
	emit ValueChanged();
}

void ParticleEmitterPropertiesWidget::Init(DAVA::ParticleEmitter *emitter, bool updateMinimize)
{
	DVASSERT(emitter != 0);
	this->emitter = emitter;

	blockSignals = true;

	float32 emitterLifeTime = emitter->GetLifeTime();

	emitterType->setCurrentIndex(emitter->type);

	emitterEmissionAngle->Init(0.f, emitterLifeTime, updateMinimize);
	emitterEmissionAngle->AddLine(0, PropLineWrapper<float32>(emitter->emissionAngle).GetProps(), Qt::blue, "emission angle");

	emitterEmissionRange->Init(0.f, emitterLifeTime, updateMinimize);
	emitterEmissionRange->AddLine(0, PropLineWrapper<float32>(emitter->emissionRange).GetProps(), Qt::blue, "emission range");

	emitterEmissionVector->Init(0.f, emitterLifeTime, updateMinimize, true);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::blue); vectorColors.push_back(Qt::darkGreen); vectorColors.push_back(Qt::red);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("emission vector: x"); vectorLegends.push_back("emission vector: y"); vectorLegends.push_back("emission vector: z");
	emitterEmissionVector->AddLines(PropLineWrapper<Vector3>(emitter->emissionVector).GetProps(), vectorColors, vectorLegends);

	emitterRadius->Init(0.f, emitterLifeTime, updateMinimize);
	emitterRadius->AddLine(0, PropLineWrapper<float32>(emitter->radius).GetProps(), Qt::blue, "radius");

	emitterColorWidget->Init(0.f, emitterLifeTime, "color over life");
	emitterColorWidget->SetValues(PropLineWrapper<Color>(emitter->colorOverLife).GetProps());

	emitterSize->Init(0.f, emitterLifeTime, updateMinimize, true);
	emitterSize->SetMinLimits(0);
	Vector<QColor> sizeColors;
	sizeColors.push_back(Qt::blue); sizeColors.push_back(Qt::darkGreen); sizeColors.push_back(Qt::red);
	Vector<QString> sizeLegends;
	sizeLegends.push_back("size: x"); sizeLegends.push_back("size: y"); sizeLegends.push_back("size: z");
	emitterSize->AddLines(PropLineWrapper<Vector3>(emitter->size).GetProps(), sizeColors, sizeLegends);
	emitterSize->EnableLock(true);
	
	emitterLife->setValue(emitterLifeTime);

	blockSignals = false;
}

void ParticleEmitterPropertiesWidget::Update()
{
	Init(emitter, false);
}

bool ParticleEmitterPropertiesWidget::eventFilter(QObject * o, QEvent * e)
{
    if (e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}