//
//  ParticleTimeLineWidget.cpp
//  ResourceEditorQt
//
//  Created by adebt on 1/2/13.
//
//

#include "ParticleTimeLineWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include "ParticleEditorCommands.h"
#include "CommandsManager.h"

#include "ParticlesEditorController.h"

#define LEFT_INDENT 10
#define TOP_INDENT 5
#define BOTTOM_INDENT 18
#define LINE_STEP 16
#define RECT_SIZE 3
#define LINE_WIDTH 3

ParticleTimeLineWidget::ParticleTimeLineWidget(QWidget *parent/* = 0*/) :
	QWidget(parent),
	selectedPoint(-1, -1),
	emitterNode(NULL),
	effectNode(NULL)
{
	backgroundBrush.setColor(Qt::white);
	backgroundBrush.setStyle(Qt::SolidPattern);
	
	connect(ParticlesEditorController::Instance(),
			SIGNAL(EmitterSelected(ParticleEmitterNode*)),
			this,
			SLOT(OnNodeSelected(ParticleEmitterNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(LayerSelected(ParticleEmitterNode*, ParticleLayer*)),
			this,
			SLOT(OnNodeSelected(ParticleEmitterNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(ForceSelected(ParticleEmitterNode*, ParticleLayer*, int32)),
			this,
			SLOT(OnNodeSelected(ParticleEmitterNode*)));

	connect(ParticlesEditorController::Instance(),
			SIGNAL(EffectSelected(ParticleEffectNode*)),
			this,
			SLOT(OnEffectNodeSelected(ParticleEffectNode*)));
	
	Init(0, 0);
	
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	
	/*Init(0, 4.0f);
	AddLine(0, 0.5f, 1.5f, Qt::blue, "12341234");
	AddLine(1, 2.5f, 3.5f, Qt::blue, "12341234");
	AddLine(2, 0, 4, Qt::red, "12341234");*/
}

ParticleTimeLineWidget::~ParticleTimeLineWidget()
{
	
}

void ParticleTimeLineWidget::OnNodeSelected(ParticleEmitterNode* node)
{
	emitterNode = node;
	effectNode = NULL;
	
	float32 minTime = 0;
	float32 maxTime = 0;
	ParticleEmitter* emitter = NULL;
	if (node)
	{
		emitter = node->GetEmitter();
		if (emitter)
			maxTime = emitter->GetLifeTime();
	}
	Init(minTime, maxTime);
	if (emitter)
	{
		QColor colors[3] = {Qt::blue, Qt::darkGreen, Qt::red};
		const Vector<ParticleLayer*> & layers = emitter->GetLayers();
		for (uint32 i = 0; i < layers.size(); ++i)
		{
			float32 startTime = Max(minTime, layers[i]->startTime);
			float32 endTime = Min(maxTime, layers[i]->endTime);
			AddLine(i, startTime, endTime, colors[i % 3], tr("Layer %1").arg(i + 1), layers[i]);
		}
	}
	
	UpdateSizePolicy();
	if (lines.size())
	{
		emit ChangeVisible(true);
		update();
	}
	else
	{
		emit ChangeVisible(false);
	}
}

void ParticleTimeLineWidget::OnEffectNodeSelected(ParticleEffectNode* node)
{
	emitterNode = NULL;
	effectNode = node;
	
	float32 minTime = 0;
	float32 maxTime = 0;
	if (node)
	{
		int32 count = node->GetChildrenCount();
		for (int32 i = 0; i < count; ++i)
		{
			ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(node->GetChild(i));
			if (emitterNode)
			{
				ParticleEmitter* emitter = emitterNode->GetEmitter();
				if (emitter)
					maxTime = Max(maxTime, emitter->GetLifeTime());
			}
		}
	}
	Init(minTime, maxTime);
	if (node)
	{
		QColor colors[3] = {Qt::blue, Qt::darkGreen, Qt::red};
		int32 count = node->GetChildrenCount();
		int32 iLines = 0;
		for (int32 iEmitter = 0; iEmitter < count; ++iEmitter)
		{
			ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(node->GetChild(iEmitter));
			if (emitterNode)
			{
				ParticleEmitter* emitter = emitterNode->GetEmitter();
				if (emitter)
				{
					const Vector<ParticleLayer*> & layers = emitter->GetLayers();
					for (uint32 iLayer = 0; iLayer < layers.size(); ++iLayer)
					{
						float32 startTime = Max(minTime, layers[iLayer]->startTime);
						float32 endTime = Min(maxTime, layers[iLayer]->endTime);
						AddLine(iLines, startTime, endTime, colors[iLines % 3], tr("Layer %1").arg(iLayer + 1), layers[iLayer]);
						iLines++;
					}
				}
			}
		}
	}
	
	UpdateSizePolicy();
	if (lines.size())
	{
		emit ChangeVisible(true);
		update();
	}
	else
	{
		emit ChangeVisible(false);
	}
}

void ParticleTimeLineWidget::Init(float32 minTime, float32 maxTime)
{
	this->minTime = minTime;
	this->maxTime = maxTime;
	
	lines.clear();
}

void ParticleTimeLineWidget::AddLine(uint32 lineId, float32 startTime, float32 endTime, const QColor& color, const QString& legend, ParticleLayer* layer)
{
	LINE line;
	line.startTime = startTime;
	line.endTime = endTime;
	line.color = color;
	line.legend = legend;
	line.layer = layer;
	lines[lineId] = line;
}

void ParticleTimeLineWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	
	QFont font("Courier", 11, QFont::Normal);
	painter.setFont(font);
	
	painter.fillRect(this->rect(), backgroundBrush);
	
	QRect graphRect = GetGraphRect();
	
	//draw grid
	{
		//painter.setPen(Qt::lightGray);
		painter.setPen(Qt::gray);
		
		float step = 18;
		float steps = graphRect.width() / step;
		float valueStep = (maxTime - minTime) / steps;
		bool drawText = false;
		for (int i = 0; i <= steps; i++)
		{
			int x = graphRect.left() + i * step;
			painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
			drawText = !drawText;
			if (!drawText)
				continue;
			
			float value = minTime + i * valueStep;
			QString strValue;
			if (value < 10)
				strValue = "%.2f";
			else if (value < 100)
				strValue = "%.1f";
			else
				strValue = "%.0f";
			strValue.sprintf(strValue.toAscii(), value);
			int textWidth = painter.fontMetrics().width(strValue);
			QRect textRect(x - textWidth / 2, graphRect.bottom(), textWidth, BOTTOM_INDENT);
			painter.drawText(textRect, Qt::AlignCenter, strValue);
		}
	}
	
	painter.setFont(QFont("Courier", 12, QFont::Normal));
	
	painter.setPen(Qt::black);
	painter.drawRect(graphRect);

	uint32 i = 0;
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter, ++i)
	{
		const LINE& line = iter->second;
		
		QRect startRect;
		QRect endRect;
		GetLineRect(iter->first, startRect, endRect);
		painter.setPen(QPen(line.color, 1));
		painter.drawLine(QPoint(graphRect.left(), startRect.center().y()), QPoint(graphRect.right(), startRect.center().y()));
		painter.drawText(QPoint(LEFT_INDENT, startRect.bottom()), line.legend);
	
		painter.setPen(QPen(line.color, LINE_WIDTH));
		if (selectedPoint.x() == iter->first)
		{
			QBrush brush(line.color);
			if (selectedPoint.y() == 0)
				painter.fillRect(startRect, brush);
			else
				painter.fillRect(endRect, brush);
		}
		painter.drawRect(startRect);
		painter.drawRect(endRect);
		
		QPoint startPoint(startRect.center());
		startPoint.setX(startPoint.x() + 3);
		QPoint endPoint(endRect.center());
		endPoint.setX(endPoint.x() - 3);
		painter.drawLine(startPoint, endPoint);
	}
}

bool ParticleTimeLineWidget::GetLineRect(uint32 id, QRect& startRect, QRect& endRect) const
{
	uint32 i = 0;
	QRect grapRect = GetGraphRect();
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter, ++i)
	{
		if (iter->first != id)
			continue;
		
		const LINE& line = iter->second;
		
		QPoint startPoint(grapRect.left() + line.startTime / (maxTime - minTime) * grapRect.width(), grapRect.top() + (i + 1) * LINE_STEP);
		QPoint endPoint(grapRect.left() + line.endTime / (maxTime - minTime) * grapRect.width(), grapRect.top() + (i + 1) * LINE_STEP);
		startRect = QRect(startPoint - QPoint(RECT_SIZE, RECT_SIZE), startPoint + QPoint(RECT_SIZE, RECT_SIZE));
		endRect = QRect(endPoint - QPoint(RECT_SIZE, RECT_SIZE), endPoint + QPoint(RECT_SIZE, RECT_SIZE));
		return true;
	}
	return false;
}

QRect ParticleTimeLineWidget::GetGraphRect() const
{
	QRect rect;
	rect.setTopLeft(QPoint(LEFT_INDENT + 80, TOP_INDENT));
	rect.setBottomRight(QPoint(width() - LEFT_INDENT - 5, height() - BOTTOM_INDENT));
	return rect;
}

void ParticleTimeLineWidget::UpdateSizePolicy()
{
	int height = (lines.size() + 1) * LINE_STEP + BOTTOM_INDENT + TOP_INDENT;
	setMinimumHeight(height);
}

void ParticleTimeLineWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (selectedPoint.x() == -1)
		return;
	
	LINE_MAP::iterator iter = lines.find(selectedPoint.x());
	if (iter == lines.end())
		return;
	
	LINE& line = iter->second;
	
	QRect graphRect = GetGraphRect();
	float32 value = (event->pos().x() - graphRect.left()) / (float32)graphRect.width() * (maxTime - minTime) + minTime;
	value = Max(minTime, Min(maxTime, value));
	if (selectedPoint.y() == 0) //start point selected
	{
		line.startTime = Min(value, line.endTime);
	}
	else
	{
		line.endTime = Max(value, line.startTime);
	}
	update();
}

void ParticleTimeLineWidget::mousePressEvent(QMouseEvent * event)
{
	selectedPoint = GetPoint(event->pos());
	update();
}

void ParticleTimeLineWidget::mouseReleaseEvent(QMouseEvent *)
{
	if (selectedPoint.x() != -1 &&
		selectedPoint.y() != -1)
	{
		OnValueChanged(selectedPoint.x());
	}
		
	selectedPoint = QPoint(-1, -1);
	update();
}

void ParticleTimeLineWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	QPoint point = GetPoint(event->pos());
	LINE_MAP::iterator iter = lines.find(point.x());
	if (iter != lines.end())
	{
		LINE& line = iter->second;
		float32 value = point.y() == 0 ? line.startTime : line.endTime;
		float32 minValue = point.y() == 0 ? minTime : line.startTime;
		float32 maxValue = point.y() == 0 ? line.endTime : maxTime;
		SetPointValueDlg dlg(value, minValue, maxValue, this);
		if (dlg.exec())
		{
			if (point.y() == 0)
				line.startTime = dlg.GetValue();
			else
				line.endTime = dlg.GetValue();
			
			OnValueChanged(iter->first);
		}
	}
	update();
}

QPoint ParticleTimeLineWidget::GetPoint(const QPoint& pos) const
{
	QPoint point = QPoint(-1, -1);
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		QRect startRect;
		QRect endRect;
		if (!GetLineRect(iter->first, startRect, endRect))
			continue;
		
		//startRect.translate(-LINE_WIDTH, -LINE_WIDTH);
		//endRect.translate(-LINE_WIDTH, -LINE_WIDTH);
		startRect.adjust(-LINE_WIDTH, -LINE_WIDTH, LINE_WIDTH, LINE_WIDTH);
		endRect.adjust(-LINE_WIDTH, -LINE_WIDTH, LINE_WIDTH, LINE_WIDTH);
		point.setX(iter->first);
		if (startRect.contains(pos))
		{
			point.setY(0);
			break;
		}
		
		if (endRect.contains(pos))
		{
			point.setY(1);
			break;
		}
	}
	if (point.y() == -1)
		point.setX(-1);
	return point;
}

float32 ParticleTimeLineWidget::SetPointValueDlg::GetValue() const
{
	return valueSpin->value();
}

void ParticleTimeLineWidget::OnValueChanged(int lineId)
{
	LINE_MAP::iterator iter = lines.find(lineId);
	if (iter == lines.end())
		return;
	
	CommandUpdateParticleLayerTime* cmd = new CommandUpdateParticleLayerTime(iter->second.layer);
	cmd->Init(iter->second.startTime, iter->second.endTime);
	CommandsManager::Instance()->Execute(cmd);
	SafeRelease(cmd);
	
	emit ValueChanged();
}

void ParticleTimeLineWidget::OnUpdate()
{
	if (emitterNode)
		OnNodeSelected(emitterNode);
	else if (effectNode)
		OnEffectNodeSelected(effectNode);
}

ParticleTimeLineWidget::SetPointValueDlg::SetPointValueDlg(float32 value, float32 minValue, float32 maxValue, QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle("Set time");
	
	QVBoxLayout* mainBox = new QVBoxLayout;
	setLayout(mainBox);
	
	valueSpin = new QDoubleSpinBox(this);
	mainBox->addWidget(valueSpin);
	
	QHBoxLayout* btnBox = new QHBoxLayout;
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnBox->addWidget(btnCancel);
	btnBox->addWidget(btnOk);
	mainBox->addLayout(btnBox);
	
	valueSpin->setMinimum(minValue);
	valueSpin->setMaximum(maxValue);
	valueSpin->setValue(value);
	valueSpin->setSingleStep(0.01);
	
	connect(btnOk,
			SIGNAL(clicked(bool)),
			this,
			SLOT(accept()));
	connect(btnCancel,
			SIGNAL(clicked(bool)),
			this,
			SLOT(reject()));
	
	btnOk->setDefault(true);
	valueSpin->setFocus();
	valueSpin->selectAll();	
}