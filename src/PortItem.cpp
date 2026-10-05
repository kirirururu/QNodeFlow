#include "PortItem.h"

#include "Style.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>

namespace QNodeFlow {

PortItem::PortItem(PortDirection direction, int index, const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent), _direction(direction), _index(index), _name(name)
{
	setAcceptHoverEvents(true);
}

QPointF PortItem::scenePos() const
{
	return mapToScene(QPointF(0.0, 0.0));
}

QRectF PortItem::boundingRect() const
{
	constexpr double half = PORT_RADIUS + PORT_LINE_WIDTH / 2;
	return QRectF(-half, -half, 2 * half, 2 * half).united(labelRect());
}

void PortItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Label first so the port circle sits on top of it.
	QFont labelFont = painter->font();
	labelFont.setPointSize(12);
	painter->setFont(labelFont);
	painter->setPen(QPen(color::PORT_LABEL));
	painter->drawText(labelRect(),
	                  _direction == PortDirection::Input ? Qt::AlignVCenter | Qt::AlignLeft
	                                                     : Qt::AlignVCenter | Qt::AlignRight,
	                  _name);

	const QColor border = _hovered ? color::PORT_BORDER_HOVER : color::PORT_BORDER;
	const QColor fill = _hovered ? color::PORT_FILL_HOVER : color::PORT_FILL;
	painter->setPen(QPen(border, PORT_LINE_WIDTH));
	painter->setBrush(QBrush(fill));
	painter->drawEllipse(QPointF(0.0, 0.0), PORT_RADIUS, PORT_RADIUS);
}

QPainterPath PortItem::shape() const
{
	// Hover area: only the port circle (plus a small tolerance)
	QPainterPath path;
	path.addEllipse(QPointF(0.0, 0.0), PORT_RADIUS + PORT_HIT_RADIUS, PORT_RADIUS + PORT_HIT_RADIUS);
	return path;
}

void PortItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = true;
	update();
	emit hoverChanged(true);
	QGraphicsItem::hoverEnterEvent(event);
}

void PortItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = false;
	update();
	emit hoverChanged(false);
	QGraphicsItem::hoverLeaveEvent(event);
}

QRectF PortItem::labelRect() const
{
	if (_direction == PortDirection::Input)
		// Label to the right of the port, spanning to the node's middle.
		return QRectF(PORT_RADIUS + PORT_LABEL_GAP, -ROW_HEIGHT / 2, BODY_WIDTH / 2, ROW_HEIGHT);

	// X coordinate of the port center in the parent node's coordinate system.
	const double centerX = BODY_WIDTH - (BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2);
	// Label to the left of the port, from the node's left edge.
	return QRectF(-centerX, -ROW_HEIGHT / 2, centerX - PORT_RADIUS - PORT_LABEL_GAP, ROW_HEIGHT);
}

} // namespace QNodeFlow
