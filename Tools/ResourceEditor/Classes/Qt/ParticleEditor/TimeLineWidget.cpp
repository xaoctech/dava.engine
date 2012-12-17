#include "TimeLineWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include <Base/Introspection.h>

#define POINT_SIZE 4

#define LEGEND_WIDTH 12

TimeLineWidget::TimeLineWidget(QWidget *parent) :
	QWidget(parent)
{
	minValue = std::numeric_limits<float32>::infinity();
	maxValue = -std::numeric_limits<float32>::infinity();
	minTime = 0.0;
	maxTime = 1;
	
	backgroundBrush.setColor(Qt::darkGray);
	backgroundBrush.setStyle(Qt::SolidPattern);

	selectedPoint = -1;
	selectedLine = -1;
	drawLine = -1;
	
	minimized = false;
	updateMinimize = true;
	aliasLinePoint = false;
	allowDeleteLine = true;
	
	setMouseTracking(true);
	UpdateSizePolicy();
}

TimeLineWidget::~TimeLineWidget()
{

}

void TimeLineWidget::paintEvent(QPaintEvent * /*paintEvent*/)
{
	QPainter painter(this);

	QFont font("Courier", 11, QFont::Normal);
	painter.setFont(font);
	
	painter.fillRect(this->rect(), backgroundBrush);
	painter.setPen(Qt::black);
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	QRect graphRect = GetGraphRect();
		
	//draw legend
	if (!IsLegendEmpty())
	{
		QString legend;
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (iter->second.legend.isEmpty())
				continue;
			
			legend += iter->second.legend;
			legend += " ";
		}
		int legenWidth = painter.fontMetrics().width(legend);
		int textPos = (width() - legenWidth) / 2;
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			QString legend = iter->second.legend;
			if (legend.isEmpty())
				continue;

			legend += " ";
			painter.setPen(iter->second.color);
			painter.drawText(QPoint(textPos, LEGEND_WIDTH), legend);
			textPos += painter.fontMetrics().width(legend);
		}
	}
	
	//draw minimizebox
	{
		painter.setPen(Qt::white);
		QRect minimizeRect = GetMinimizeRect();
		painter.drawRect(minimizeRect);
		painter.save();
		painter.translate(minimizeRect.center() + QPoint(1, 1));
		QPolygon polygon;
		
		if (!minimized)
			painter.rotate(180);
		
		polygon.append(QPoint(0, -minimizeRect.height() * 0.25 - 1));
		polygon.append(QPoint(minimizeRect.width() * 0.25 + 1, minimizeRect.height() * 0.25 + 1));
		polygon.append(QPoint(-minimizeRect.width() * 0.25 - 1, minimizeRect.height() * 0.25 + 1));
		
		QPainterPath painterPath;
		painterPath.addPolygon(polygon);
		painter.fillPath(painterPath, Qt::white);
		painter.restore();
	}
	
	//draw limits
	if (!minimized)
	{
		//draw graph border
		painter.setPen(Qt::white);
		painter.drawRect(graphRect);
		
		{
			QString strValue;
			strValue.sprintf("%.1f", minValue);
			painter.save();
			painter.translate(graphRect.x() - 1, graphRect.bottom());
			painter.rotate(-90);
			painter.drawText(0, 0, strValue);
			painter.restore();
		}
		{
			QString strValue;
			strValue.sprintf("%.1f", maxValue);
			painter.save();
			painter.translate(graphRect.x() - 1, graphRect.top());
			painter.rotate(-90);
			painter.drawText(-painter.fontMetrics().width(strValue), 0, strValue);
			painter.restore();
		}
		{
			QString strValue;
			strValue.sprintf("%.1f", minTime);
			painter.drawText(graphRect.left() + 1, graphRect.bottom(), strValue);
		}
		{
			QString strValue;
			strValue.sprintf("%.1f", maxTime);
			painter.drawText(graphRect.right() - painter.fontMetrics().width(strValue), graphRect.bottom(), strValue);
		}
		
		//draw lines
		bool isLineEnable = false;
		uint32 lineCount = 0;
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter, ++lineCount)
		{
			uint32 lineId = iter->first;
			if (drawLine == -1 || drawLine == lineId)
				DrawLine(&painter, lineId);
			
			QPen pen;
			pen.setColor(iter->second.color);
			painter.setPen(pen);

			//draw line enable
			QRect lineEnableRect = GetLineEnableRect(lineId);
			painter.drawRect(lineEnableRect);
			if (iter->second.line.size())
			{
				isLineEnable = true;
			}
			else
			{
				painter.drawLine(lineEnableRect.topLeft(), lineEnableRect.bottomRight());
				painter.drawLine(lineEnableRect.bottomLeft(), lineEnableRect.topRight());
			}
			
			//draw drawed colors
			QRect rect = GetLineDrawRect();
			rect.translate(rect.width() * lineCount / lines.size(), 0);
			rect.setWidth(rect.width() / lines.size());
			if (drawLine == -1)
				painter.fillRect(rect, iter->second.color);
			else
				painter.fillRect(rect, lines[drawLine].color);
		}
		
		if (!isLineEnable)
		{
			painter.setPen(Qt::red);
			painter.drawLine(graphRect.topLeft(), graphRect.bottomRight());
			painter.drawLine(graphRect.bottomLeft(), graphRect.topRight());
		}
	}

	if (selectedPoint == -1 && selectedLine != -1)
	{
		QBrush pointBrush;
		pointBrush.setColor(lines[selectedLine].color);
		pointBrush.setStyle(Qt::SolidPattern);

		painter.fillRect(GetPointRect(GetDrawPoint(newPoint)), pointBrush);
	}
}

void TimeLineWidget::DrawLine(QPainter* painter, uint32 lineId)
{
	if (lines[lineId].line.size() == 0)
		return;

	QBrush pointBrush;
	pointBrush.setColor(lines[lineId].color);
	pointBrush.setStyle(Qt::SolidPattern);
	QPen pen;
	pen.setColor(lines[lineId].color);
	painter->setPen(pen);
	
	QRect graphRect = GetGraphRect();
	
	QPoint prevPoint = GetDrawPoint(lines[lineId].line[0]);
	prevPoint.setX(graphRect.x());
	
	for (uint i = 0; i < lines[lineId].line.size(); ++i)
	{
		QPoint point = GetDrawPoint(lines[lineId].line[i]);
		painter->drawLine(prevPoint, point);
		
		if (selectedPoint == i && selectedLine == lineId)
			painter->fillRect(GetPointRect(point), pointBrush);
		else
			painter->drawRect(GetPointRect(point));
		
		prevPoint = point;
	}
	
	QPoint point = GetDrawPoint(lines[lineId].line[lines[lineId].line.size() - 1]);
	point.setX(graphRect.x() + graphRect.width());
	painter->drawLine(prevPoint, point);
}

QPoint TimeLineWidget::GetDrawPoint(const Vector2& point) const
{
	float32 time = maxTime - minTime;
	float32 value = maxValue - minValue;
	if (time < 0.01f || value < 0.01f)
		return QPoint();
	
	QRect graphRect = GetGraphRect();
	float x = graphRect.x() + graphRect.width() * (point.x - minTime) / time;
	float y = graphRect.bottom() - graphRect.height() * (point.y - minValue) / value;
	
	return QPoint(x, y);
}

Vector2 TimeLineWidget::GetLogicPoint(const QPoint& point) const
{
	QRect graphRect = GetGraphRect();
	
	float32 x = (point.x() - graphRect.x()) / (float32)graphRect.width();
	x = minTime + x * (maxTime - minTime);
	float32 y = (graphRect.bottom() - point.y()) / (float32)graphRect.height();
	y = minValue + y * (maxValue - minValue);
	
	return Vector2(x, y);
}

QRect TimeLineWidget::GetPointRect(const QPoint& point) const
{
	return QRect(point.x() - POINT_SIZE, point.y() - POINT_SIZE, POINT_SIZE * 2, POINT_SIZE * 2);
}

bool TimeLineWidget::SortPoints(const Vector2& i, const Vector2& j)
{
	return (i.x < j.x);
}

void TimeLineWidget::Init(float32 minT, float32 maxT, bool updateMinimize, bool aliasLinePoint, bool allowDeleteLine)
{
	lines.clear();

	this->minTime = minT;
	this->maxTime = maxT;
	
	this->updateMinimize = updateMinimize;
	this->aliasLinePoint = aliasLinePoint;
	this->allowDeleteLine = allowDeleteLine;
}

void TimeLineWidget::AddLine(uint32 lineId, const Vector< PropValue<float32> >& line, const QColor& color, const QString& legend)
{
	LOGIC_POINTS desLine;
	for (uint32 i = 0; i < line.size(); ++i)
		desLine.push_back(Vector2(line[i].t, line[i].v));

	lines[lineId].line = desLine;
	lines[lineId].color = color;
	lines[lineId].legend = legend;
	
	PostAddLine();
}

void TimeLineWidget::AddLines(const Vector< PropValue<Vector2> >& lines, const Vector<QColor>& colors, const Vector<QString>& legends)
{
	if (colors.size() < 2 || legends.size() < 2)
	{
		Logger::Debug("incorrect number of input arguments");
		return;
	}
	
	LOGIC_POINTS desLine[2];
	for (uint32 i = 0; i < lines.size(); ++i)
	{
		desLine[0].push_back(Vector2(lines[i].t, lines[i].v.x));
		desLine[1].push_back(Vector2(lines[i].t, lines[i].v.y));
	}
	
	for (int i = 0; i < 2; i++)
	{
		int id = this->lines.size();
		this->lines[id].line = desLine[i];
		this->lines[id].color = colors[i];
		this->lines[id].legend = legends[i];
	}
	
	PostAddLine();
}

void TimeLineWidget::AddLines(const Vector< PropValue<Vector3> >& lines, const Vector<QColor>& colors, const Vector<QString>& legends)
{
	if (colors.size() < 3 || legends.size() < 3)
	{
		Logger::Debug("incorrect number of input arguments");
		return;
	}
	
	LOGIC_POINTS desLine[3];
	for (uint32 i = 0; i < lines.size(); ++i)
	{
		desLine[0].push_back(Vector2(lines[i].t, lines[i].v.x));
		desLine[1].push_back(Vector2(lines[i].t, lines[i].v.y));
		desLine[2].push_back(Vector2(lines[i].t, lines[i].v.z));
	}
	
	for (int i = 0; i < 3; i++)
	{
		int id = this->lines.size();
		this->lines[id].line = desLine[i];
		this->lines[id].color = colors[i];
		this->lines[id].legend = legends[i];
	}
	
	PostAddLine();
}

void TimeLineWidget::PostAddLine()
{
	if (updateMinimize)
	{
		minimized = true;
		for (LINES_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (iter->second.line.size())
			{
				minimized = false;
				break;
			}
		}
	}
	
	UpdateLimits();
	UpdateSizePolicy();
	update();
}

void TimeLineWidget::UpdateLimits()
{
	float32 newMinValue = std::numeric_limits<float32>::infinity();
	float32 newMaxValue = -std::numeric_limits<float32>::infinity();

	for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		for (uint32 i = 0; i < iter->second.line.size(); ++i)
		{
			newMaxValue = Max(iter->second.line[i].y, newMaxValue);
			newMinValue = Min(iter->second.line[i].y, newMinValue);
			
			maxTime = Max(iter->second.line[i].x, maxTime);
			minTime = Min(iter->second.line[i].x, minTime);
		}
	}
	

	if (newMinValue == std::numeric_limits<float32>::infinity() ||
		newMaxValue == -std::numeric_limits<float32>::infinity())
	{
		newMinValue = newMaxValue = 0;
	}

	float32 limitDelta = 0;
	limitDelta = (newMaxValue - newMinValue) * 0.2f;
	if (limitDelta < 0.01f)
		limitDelta = newMaxValue * 0.2;
	if (limitDelta < 0.01f)
		limitDelta = 1.f;

	if (Abs(maxValue) > Abs(newMaxValue) * 1.2 ||
		Abs(minValue) < Abs(newMinValue) * 1.2 ||
		newMinValue < minValue ||
		newMaxValue > maxValue)
	{
		minValue = newMinValue - limitDelta;
		maxValue = newMaxValue + limitDelta;
	}
}

bool TimeLineWidget::GetValue(uint32 lineId, Vector< PropValue<float32> >* line) const
{
	LINES_MAP::const_iterator iter = lines.find(lineId);
	if (iter == lines.end())
		return false;
	
	for (uint32 i = 0; i < iter->second.line.size(); ++i)
	{
		line->push_back(PropValue<float32>(iter->second.line[i].x, iter->second.line[i].y));
	}
				
	return true;
}

bool TimeLineWidget::GetValues(Vector< PropValue<Vector2> >* lines)
{
	LINES_MAP::const_iterator iter = this->lines.begin();
	if (iter == this->lines.end())
		return false;
	
	for (uint32 i = 0; i < iter->second.line.size(); ++i)
	{
		Vector2 value;
		value.x = this->lines[0].line[i].y;
		value.y = this->lines[1].line[i].y;
		lines->push_back(PropValue<Vector2>(this->lines[0].line[i].x, value));
	}
	return true;
}

bool TimeLineWidget::GetValues(Vector< PropValue<Vector3> >* lines)
{
	LINES_MAP::const_iterator iter = this->lines.begin();
	if (iter == this->lines.end())
		return false;

	for (uint32 i = 0; i < iter->second.line.size(); ++i)
	{
		Vector3 value;
		value.x = this->lines[0].line[i].y;
		value.y = this->lines[1].line[i].y;
		value.z = this->lines[2].line[i].y;
		lines->push_back(PropValue<Vector3>(this->lines[0].line[i].x, value));
	}
	return true;
}

void TimeLineWidget::AddPoint(uint32 lineId, const Vector2& point)
{
	if (aliasLinePoint)
	{
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (iter->first == lineId)
				iter->second.line.push_back(Vector2(point.x, point.y));
			else
			{
				float32 y = GetYFromX(iter->first, point.x);
				iter->second.line.push_back(Vector2(point.x, y));
			}
			std::sort(iter->second.line.begin(), iter->second.line.end(), TimeLineWidget::SortPoints);
		}
	}
	else if (lines.find(lineId) != lines.end())
	{
		lines[lineId].line.push_back(point);
		std::sort(lines[lineId].line.begin(), lines[lineId].line.end(), TimeLineWidget::SortPoints);
	}
	this->update();
}

bool TimeLineWidget::DeletePoint(uint32 lineId, uint32 pointId)
{
	if (!allowDeleteLine &&
		lines[lineId].line.size() < 2)
		return false;
	
	selectedLine = -1;
	if (aliasLinePoint)
	{
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (pointId < iter->second.line.size())
				iter->second.line.erase(iter->second.line.begin() + pointId);
		}
	}
	else if (lines.find(lineId) != lines.end())
	{
		if (pointId < lines[lineId].line.size())
			lines[lineId].line.erase(lines[lineId].line.begin() + pointId);
	}
	this->update();
	return true;
}

float32 TimeLineWidget::GetYFromX(uint32 lineId, float32 x)
{
	LOGIC_POINTS& points = lines.at(lineId).line;

	if (points.empty())
		return 0.f;

	uint32 right = (uint32)-1;
	for (uint32 i = 0; i < points.size(); ++i)
	{
		if(points[i].x > x)
		{
			right = i;
			break;
		}
	}

	Vector2 leftPoint;
	Vector2 rightPoint;
	if (right == (uint32)-1)
	{
		leftPoint = points.back();
		rightPoint = points.back() + Vector2(x, 0);
	}
	else
	{
		rightPoint = points[right];
		if(right > 0)
			leftPoint = points[right - 1];
		else
		{
			leftPoint = rightPoint;
			leftPoint.x = 0;
		}
	}
	float32 y = Interpolation::Linear(leftPoint.y, rightPoint.y, leftPoint.x, x, rightPoint.x);

	return y;
}

QRect TimeLineWidget::GetGraphRect() const
{
	QRect graphRect = this->rect();
	graphRect.setX(graphRect.x() + 1 + LEGEND_WIDTH);
	if (IsLegendEmpty())
		graphRect.setY(graphRect.y() + 5);
	else
		graphRect.setY(graphRect.y() + 2 + LEGEND_WIDTH);
    graphRect.setWidth(graphRect.width() - 5);
	if (minimized)
		graphRect.setHeight(0);
	else
		graphRect.setHeight(graphRect.height() - 20);

	return graphRect;
}

void TimeLineWidget::mousePressEvent(QMouseEvent *event)
{
	QWidget::mousePressEvent(event);
	
	mouseStartPos = event->pos();
		
	//check click on draw color rect
	if (event->button()==Qt::LeftButton)
	{
		if (GetLineDrawRect().contains(event->pos()))
		{
			drawLine++;
			if (drawLine >= (int32)lines.size())
				drawLine = -1;
		}
		else if (GetMinimizeRect().contains(event->pos()))
		{
			minimized = !minimized;
			UpdateSizePolicy();
			
			update();
			return;
		}
		else
		{
			bool emitUpdate = false;
			for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
			{
				QRect rect = GetLineEnableRect(iter->first);
				if (rect.contains(event->pos()))
				{
					if (aliasLinePoint)
					{
						for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
							if (iter->second.line.size())
								iter->second.line.clear(); //clear existing line
							else
								iter->second.line.push_back(Vector2(minTime, (minValue + maxValue) / 2)); //init dafault
					}
					else
					{
						if (iter->second.line.size())
							iter->second.line.clear(); //clear existing line
						else
							iter->second.line.push_back(Vector2(minTime, (minValue + maxValue) / 2)); //init dafault
					}
					emit ValueChanged();
					break;
				}
			}
		}
	}

	if (!minimized)
		GraphRectClick(event);

	update();
}

void TimeLineWidget::GraphRectClick(QMouseEvent *event)
{
	int32 pointId = -1;
	int32 lineId = -1;
	QPoint point = event->pos();

	GetClickedPoint(point, pointId, lineId);
		
	if (event->button()==Qt::LeftButton)
	{
		if (pointId != -1)
		{
			this->selectedPoint = pointId;
			this->selectedLine = lineId;
		}
		else if (selectedLine != -1)
		{
			QRect graphRect = GetGraphRect();
			if (graphRect.contains(point))
			{
				AddPoint(selectedLine, newPoint);
				//find add point
				for (uint32 i = 0; i < lines[selectedLine].line.size(); ++i)
				{
					if (lines[selectedLine].line[i].x == newPoint.x)
					{
						selectedPoint = i;
						break;
					}
				}
			}
		}
	}
	else if (event->button() == Qt::RightButton && pointId != -1)
	{
		DeletePoint(selectedLine, pointId);
		emit ValueChanged();
	}
	
	update();
}

void TimeLineWidget::mouseMoveEvent(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);
	
	if (minimized)
		return;
		
	Vector2 point = GetLogicPoint(event->pos());
	if (selectedPoint == -1)
	{
		selectedLine = -1;
		//get selected line
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			uint32 lineId = iter->first;
			if (drawLine != -1 && drawLine != lineId)
				continue;
			
			const LOGIC_POINTS& line = iter->second.line;
			if (line.size() == 0)
				continue;
			
			Vector2 prevPoint = line[0];
			prevPoint.x = minTime;
			for (uint32 i = 0; i < line.size() + 1; ++i)
			{
				Vector2 nextPoint;
				if (i < line.size())
					nextPoint = line[i];
				else
				{
					nextPoint = prevPoint;
					nextPoint.x = maxTime;
				}
				
				if (prevPoint.x < point.x && point.x < nextPoint.x)
				{
					float32 y = 0;
					if ((nextPoint.x - prevPoint.x) < 0.01f)
						y = prevPoint.y;
					else
						y = (point.x - prevPoint.x) * (nextPoint.y - prevPoint.y) / (nextPoint.x - prevPoint.x) + prevPoint.y;
					
					QRect rect = GetPointRect(GetDrawPoint(Vector2(point.x, y)));
					if (rect.contains(event->pos()))
					{
						newPoint = Vector2(point.x, y);
						selectedLine = lineId;
						break;
					}
				}
				prevPoint = nextPoint;
			}
			
			if (selectedLine != -1)
				break;
		}
	}
	else
	{
		SetPointValue(selectedLine, selectedPoint, point, false);
	}
	
	update();
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent * event)
{
	QWidget::mouseReleaseEvent(event);
	
	if ((mouseStartPos - event->pos()).manhattanLength() > 3)
	{
		if (event->button() == Qt::LeftButton)
		{
			if (selectedLine != -1 && selectedPoint != -1)
			{
				Vector2 point = GetLogicPoint(event->pos());
				SetPointValue(selectedLine, selectedPoint, point, true);
				emit ValueChanged();
			}
		}
	}
	selectedPoint = -1;
	selectedLine = -1;
}

void TimeLineWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	QWidget::mouseDoubleClickEvent(event);
	
	if (event->button() == Qt::LeftButton)
	{
		int32 pointId = -1;
		int32 lineId = -1;
		GetClickedPoint(event->pos(), pointId, lineId);

		if (lineId != -1)
		{
			ChangePointValueDialog(pointId, lineId);
		}
	}
}

void TimeLineWidget::SetPointValue(uint32 lineId, uint32 pointId, Vector2 value, bool deleteSamePoints)
{
	if (pointId > 0)
		value.x = Max(lines[lineId].line[pointId - 1].x, value.x);
	if (pointId < (lines[lineId].line.size() - 1))
		value.x = Min(lines[lineId].line[pointId + 1].x, value.x);
	 
	value.x = Max(minTime, Min(maxTime, value.x));

	if (aliasLinePoint)
	{
		for (LINES_MAP::iterator iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (iter->first == lineId)
				iter->second.line[pointId] = value;
			else
				iter->second.line[pointId].x = value.x;
		}
	}
	else
		lines[lineId].line[pointId] = value;
	
	if (deleteSamePoints)
	{
		//delete same time point
		for (uint32 i = 1; i < lines[lineId].line.size(); ++i)
		{
			float x1 = lines[lineId].line[i - 1].x;
			float x2 = lines[lineId].line[i].x;
			
			if ((x2 - x1) < 0.01f)
			{
				if (i < lines[lineId].line.size() - 1)
				{
					if (Abs(x2 - value.x) < 0.01f)
						lines[lineId].line[i - 1].y = value.y;
					//remove next point
					//lines[lineId].line.erase(lines[lineId].line.begin() + i);
					DeletePoint(lineId, i);
				}
				else
				{
					if (Abs(x1 - value.x) < 0.01f)
						lines[lineId].line[i].y = value.y;
					//remove first point
					//lines[lineId].line.erase(lines[lineId].line.begin() + lines[lineId].line.size() - 2);
					DeletePoint(lineId, lines[lineId].line.size() - 2);
				}
				i = 0;
			}
		}
	}
	
	if (deleteSamePoints)
		UpdateLimits();
	update();
}

void TimeLineWidget::leaveEvent(QEvent *)
{
	selectedLine = -1;
	
	update();
}

QRect TimeLineWidget::GetLineEnableRect(uint32 lineId) const
{
	uint32 lineCount = 0;
	for (LINES_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter, ++lineCount)
	{
		if (iter->first == lineId)
			break;
	}
	
	QRect graphRect = GetGraphRect();
	int rectSize = 10;
	QRect lineEnableRect(0, 0, rectSize, rectSize);
	lineEnableRect.translate(graphRect.left() + 50 + rectSize * 2 * lineCount, graphRect.bottom() + 5);
	return lineEnableRect;
}

QRect TimeLineWidget::GetLineDrawRect() const
{
	QRect graphRect = GetGraphRect();
	QRect lineDrawRect(0, 0, 40, 10);
	lineDrawRect.translate(graphRect.left() + 5, graphRect.bottom() + 5);
	return lineDrawRect;
}

QRect TimeLineWidget::GetMinimizeRect() const
{
	return QRect(this->width() - LEGEND_WIDTH - 2, 2, LEGEND_WIDTH - 2, LEGEND_WIDTH -2);
}

bool TimeLineWidget::IsLegendEmpty() const
{
	for (LINES_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		if (!iter->second.legend.isEmpty())
			return false;
	}
	return true;
}

void TimeLineWidget::UpdateSizePolicy()
{
	if (minimized)
	{
		setMinimumHeight(18);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	}
	else
	{
		setMinimumHeight(150);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
}

void TimeLineWidget::GetClickedPoint(const QPoint& point, int32& pointId, int32& lineId) const
{
	//find point
	for (LINES_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		if (drawLine != -1 && drawLine != iter->first)
			continue;
		
		for (uint32 j = 0; j < iter->second.line.size(); ++j)
		{
			QRect rect = GetPointRect(GetDrawPoint(iter->second.line[j]));
			if (rect.contains(point))
			{
				pointId = j;
				break;
			}
		}
		if (pointId != -1)
		{
			lineId = iter->first;
			return;
		}
	}
	pointId = -1;
	lineId = -1;
}

void TimeLineWidget::ChangePointValueDialog(uint32 pointId, int32 lineId)
{
	LINES_MAP::iterator iter = lines.find(lineId);
	if (iter == lines.end())
		return;
	if (iter->second.line.size() <= pointId)
		return;

	SetPointValueDlg dialog(iter->second.line[pointId].x, minTime, maxTime, iter->second.line[pointId].y, this);
	if (dialog.exec())
	{
		SetPointValue(iter->first, pointId, Vector2(dialog.GetTime(), dialog.GetValue()), true);
		UpdateLimits();
		emit ValueChanged();
		update();
	}
}

SetPointValueDlg::SetPointValueDlg(float32 time, float32 minTime, float32 maxTime, float32 value, QWidget *parent):
	QDialog(parent)
{
	QVBoxLayout* mainBox = new QVBoxLayout;
	setLayout(mainBox);
	
	QHBoxLayout* valueBox = new QHBoxLayout;
	timeSpin = new QDoubleSpinBox(this);
	valueSpin = new QDoubleSpinBox(this);
	valueBox->addWidget(new QLabel("T:"));
	valueBox->addWidget(timeSpin);
	valueBox->addWidget(new QLabel("V:"));
	valueBox->addWidget(valueSpin);
	mainBox->addLayout(valueBox);
	
	QHBoxLayout* btnBox = new QHBoxLayout;
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnBox->addWidget(btnCancel);
	btnBox->addWidget(btnOk);
	mainBox->addLayout(btnBox);
	
	timeSpin->setMinimum(minTime);
	timeSpin->setMaximum(maxTime);
	timeSpin->setValue(time);
	valueSpin->setMinimum(-std::numeric_limits<float32>::infinity());
	valueSpin->setMaximum(std::numeric_limits<float32>::infinity());
	valueSpin->setValue(value);
	
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

float32 SetPointValueDlg::GetTime() const
{
	return timeSpin->value();
}

float32 SetPointValueDlg::GetValue() const
{
	return valueSpin->value();
}