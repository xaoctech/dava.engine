#include "ParticleEmitterPropertiesWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include <QLabel>
#include <QLineEdit>
#include <QEvent>

#define EMISSION_RANGE_MIN_LIMIT_DEGREES 0.0f
#define EMISSION_RANGE_MAX_LIMIT_DEGREES 180.0f

ParticleEmitterPropertiesWidget::ParticleEmitterPropertiesWidget(QWidget* parent) :
	QWidget(parent)
{
	mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);

	emitterYamlPath = new QLineEdit(this);
	emitterYamlPath->setReadOnly(true);
	mainLayout->addWidget(emitterYamlPath);
	connect(emitterYamlPath, SIGNAL(textChanged(const QString&)), this, SLOT(OnEmitterYamlPathChanged(const QString&)));

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
	emitterLife->setMaximum(10000000);
	emitterLifeHBox->addWidget(emitterLife);
	mainLayout->addLayout(emitterLifeHBox);
	connect(emitterLife, SIGNAL(valueChanged(double)), this, SLOT(OnValueChanged()));
	
	Q_FOREACH( QAbstractSpinBox * sp, findChildren<QAbstractSpinBox*>() ) {
        sp->installEventFilter( this );
    }
	emitterYamlPath->installEventFilter(this);

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
							   emissionRange.GetPropLine(),
							   emissionVector.GetPropLine(),
							   radius.GetPropLine(),
							   colorOverLife.GetPropLine(),
							   size.GetPropLine(),
							   life);
	CommandsManager::Instance()->ExecuteAndRelease(commandUpdateEmitter);

	Init(emitter, false);
	emit ValueChanged();
}

void ParticleEmitterPropertiesWidget::Init(DAVA::ParticleEmitter *emitter, bool updateMinimize)
{
	DVASSERT(emitter != 0);
	this->emitter = emitter;

	blockSignals = true;

	float32 emitterLifeTime = emitter->GetLifeTime();
	float minTime = 0.f;
	float maxTime = emitterLifeTime;
	emitterYamlPath->setText(QString::fromStdString(emitter->GetConfigPath()));
	emitterType->setCurrentIndex(emitter->emitterType);

	if(NULL != emitterEmissionAngle)
	{
		minTime = emitterEmissionAngle->GetMinBoundary();
		maxTime = emitterEmissionAngle->GetMaxBoundary();
	}
	emitterEmissionAngle->Init(minTime, maxTime, 0.0, emitterLifeTime, updateMinimize);
	emitterEmissionAngle->AddLine(0, PropLineWrapper<float32>(emitter->emissionAngle).GetProps(), Qt::blue, "emission angle");

	if(NULL != emitterEmissionRange)
	{
		minTime = emitterEmissionRange->GetMinBoundary();
		maxTime = emitterEmissionRange->GetMaxBoundary();
	}
	emitterEmissionRange->Init(minTime, maxTime, 0.0, emitterLifeTime, updateMinimize);
	emitterEmissionRange->AddLine(0, PropLineWrapper<float32>(emitter->emissionRange).GetProps(), Qt::blue, "emission range");
	emitterEmissionRange->SetMinLimits(EMISSION_RANGE_MIN_LIMIT_DEGREES);
	emitterEmissionRange->SetMaxLimits(EMISSION_RANGE_MAX_LIMIT_DEGREES);
	emitterEmissionRange->SetYLegendMark(DEGREE_MARK_CHARACTER);

	if(NULL != emitterEmissionVector)
	{
		minTime = emitterEmissionVector->GetMinBoundary();
		maxTime = emitterEmissionVector->GetMaxBoundary();
	}
	emitterEmissionVector->Init(minTime, maxTime, 0.0, emitterLifeTime, updateMinimize, true);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::blue); vectorColors.push_back(Qt::darkGreen); vectorColors.push_back(Qt::red);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("emission vector: x"); vectorLegends.push_back("emission vector: y"); vectorLegends.push_back("emission vector: z");
	emitterEmissionVector->AddLines(PropLineWrapper<Vector3>(emitter->emissionVector).GetProps(), vectorColors, vectorLegends);

	if(NULL != emitterRadius)
	{
		minTime = emitterRadius->GetMinBoundary();
		maxTime = emitterRadius->GetMaxBoundary();
	}
	emitterRadius->Init(minTime, maxTime, 0.0, emitterLifeTime, updateMinimize);
	emitterRadius->AddLine(0, PropLineWrapper<float32>(emitter->radius).GetProps(), Qt::blue, "radius");
	// Radius cannot be negative.
	emitterRadius->SetMinLimits(0.0f);

	emitterColorWidget->Init(0.f, emitterLifeTime, "color over life");
	emitterColorWidget->SetValues(PropLineWrapper<Color>(emitter->colorOverLife).GetProps());

	if(NULL != emitterSize)
	{
		minTime = emitterSize->GetMinBoundary();
		maxTime = emitterSize->GetMaxBoundary();
	}
	emitterSize->Init(minTime, maxTime, 0.0, emitterLifeTime, updateMinimize, true);
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

void ParticleEmitterPropertiesWidget::OnEmitterYamlPathChanged(const QString& newPath)
{
	UpdateTooltip();
}

void ParticleEmitterPropertiesWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	emitterEmissionRange->SetVisualState(visualStateProps->GetArchive("EMITTER_EMISSION_RANGE_PROPS"));
	emitterEmissionVector->SetVisualState(visualStateProps->GetArchive("EMITTER_EMISSION_VECTOR_PROPS"));
	emitterRadius->SetVisualState(visualStateProps->GetArchive("EMITTER_RADIUS_PROPS"));
	emitterSize->SetVisualState(visualStateProps->GetArchive("EMITTER_SIZE_PROPS"));
}

void ParticleEmitterPropertiesWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	props->DeleteAllKeys();
	emitterEmissionRange->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_EMISSION_RANGE_PROPS", props);

	props->DeleteAllKeys();
	emitterEmissionVector->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_EMISSION_VECTOR_PROPS", props);

	props->DeleteAllKeys();
	emitterRadius->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_RADIUS_PROPS", props);

	props->DeleteAllKeys();
	emitterSize->GetVisualState(props);
	visualStateProps->SetArchive("EMITTER_SIZE_PROPS", props);

	SafeRelease(props);
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

	if (e->type() == QEvent::Resize && qobject_cast<QLineEdit*>(o))
	{
		UpdateTooltip();
		return true;
	}

    return QWidget::eventFilter(o, e);
}

void ParticleEmitterPropertiesWidget::UpdateTooltip()
{
	QFontMetrics fm = emitterYamlPath->fontMetrics();
	if (fm.width(emitterYamlPath->text()) >= emitterYamlPath->width())
	{
		emitterYamlPath->setToolTip(emitterYamlPath->text());
	}
	else
	{
		emitterYamlPath->setToolTip("");
	}
}