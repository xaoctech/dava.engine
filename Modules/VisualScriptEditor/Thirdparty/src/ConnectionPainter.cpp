#include "ConnectionPainter.hpp"

//#define DEBUG_DRAWING 1

#include "ConnectionGeometry.hpp"
#include "ConnectionState.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "Connection.hpp"

#include "NodeData.hpp"

#include "StyleCollection.hpp"

using QtNodes::ConnectionPainter;
using QtNodes::ConnectionGeometry;
using QtNodes::Connection;

ConnectionPainter::
ConnectionPainter()
{
}

QPainterPath
ConnectionPainter::
cubicPath(ConnectionGeometry const& geom)
{
    QPointF const& source = geom.source();
    QPointF const& sink = geom.sink();

    auto c1c2 = geom.pointsC1C2();

    // cubic spline
    QPainterPath cubic(source);

    cubic.cubicTo(c1c2.first, c1c2.second, sink);

    return cubic;
}

QPainterPath
ConnectionPainter::
getPainterStroke(ConnectionGeometry const& geom)
{
    auto cubic = cubicPath(geom);

    QPointF const& source = geom.source();
    QPainterPath result(source);

    unsigned segments = 20;

    for (auto i = 0ul; i < segments; ++i)
    {
        double ratio = double(i + 1) / segments;
        result.lineTo(cubic.pointAtPercent(ratio));
    }

    QPainterPathStroker stroker;
    stroker.setWidth(10.0);

    return stroker.createStroke(result);
}


#include <limits>

#include <QDebug>

void
ConnectionPainter::
paint(QPainter* painter,
      Connection const& connection)
{
    static int const rightCastPointStart = 270 * 16;
    static int const castPointLength = 180 * 16;

    auto const& connectionStyle =
    StyleCollection::connectionStyle();

    QColor normalColor = connectionStyle.normalColor();
    QColor hoverColor = connectionStyle.hoveredColor();
    QColor selectedColor = connectionStyle.selectedColor();

    auto const dataTypeOut = connection.dataType(PortType::Out);
    auto const dataTypeIn = connection.dataType(PortType::In);

    if (connectionStyle.useDataDefinedColors() && !dataTypeOut.id.isEmpty())
    {
        normalColor = connectionStyle.normalColor(dataTypeOut.id, dataTypeOut.hasData);
        hoverColor = normalColor.lighter(200);
        selectedColor = normalColor.darker(200);
    }

    ConnectionGeometry const& geom =
    connection.connectionGeometry();

    ConnectionState const& state =
    connection.connectionState();

    bool const sameTypes = state.requiresPort() || (dataTypeOut.id == dataTypeIn.id);

    double const lineWidth = connectionStyle.lineWidth();
    double const pointDiameter = connectionStyle.pointDiameter();

#ifdef DEBUG_DRAWING

    {
        QPointF const& source = geom.source();
        QPointF const& sink = geom.sink();

        auto points = geom.pointsC1C2();

        painter->setPen(Qt::red);
        painter->setBrush(Qt::red);

        painter->drawLine(QLineF(source, points.first));
        painter->drawLine(QLineF(points.first, points.second));
        painter->drawLine(QLineF(points.second, sink));
        painter->drawEllipse(points.first, 3, 3);
        painter->drawEllipse(points.second, 3, 3);

        painter->setBrush(Qt::NoBrush);

        painter->drawPath(cubicPath(geom));
    }

    {
        painter->setPen(Qt::yellow);

        painter->drawRect(geom.boundingRect());
    }
#endif

    auto cubic = cubicPath(geom);
    auto center = cubic.pointAtPercent(0.5);
    auto castPointRect = QRectF(center.x() - 10, center.y() - 10, 20, 20);

    bool const hovered = geom.hovered();

    auto const& graphicsObject =
    connection.getConnectionGraphicsObject();

    bool const selected = graphicsObject.isSelected();

    if (hovered || selected)
    {
        QPen p;

        p.setWidth(2 * lineWidth);
        p.setColor(selected ?
                   connectionStyle.selectedHaloColor() :
                   hoverColor);

        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);

        // cubic spline
        painter->drawPath(cubic);
        if (!sameTypes)
        {
            painter->drawEllipse(castPointRect);
        }
    }

    // draw normal line
    {
        QPen p;

        p.setWidth(lineWidth);

        if (selected)
            p.setColor(selectedColor);
        else
            p.setColor(normalColor);

        if (state.requiresPort())
        {
            p.setWidth(connectionStyle.constructionLineWidth());
            p.setColor(connectionStyle.constructionColor());
            p.setStyle(Qt::DashLine);
        }

        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);

        // cubic spline
        painter->drawPath(cubic);

        if (!sameTypes)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(p.color());

            // Draw out port type
            painter->drawEllipse(castPointRect);

            if (connectionStyle.useDataDefinedColors() && !dataTypeIn.id.isEmpty())
            {
                normalColor = connectionStyle.normalColor(dataTypeIn.id, dataTypeIn.hasData);
                selectedColor = normalColor.darker(200);
                if (selected)
                    painter->setBrush(selectedColor);
                else
                    painter->setBrush(normalColor);

                // Draw in port type
                painter->drawPie(castPointRect, rightCastPointStart, castPointLength);
            }
        }
    }

    QPointF const& source = geom.source();
    QPointF const& sink = geom.sink();

    painter->setPen(connectionStyle.constructionColor());
    painter->setBrush(connectionStyle.constructionColor());
    double const pointRadius = pointDiameter / 2.0;

    painter->drawEllipse(source, pointRadius, pointRadius);
    painter->drawEllipse(sink, pointRadius, pointRadius);
}
