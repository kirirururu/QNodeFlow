#include "NodeItem.h"

#include "PortItem.h"
#include "Style.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace QNodeFlow {

namespace {

QPainterPath createTopRoundedRect(const QRectF& rect, qreal radius)
{
	QPainterPath path;
	// start of the top edge
	path.moveTo(rect.left() + radius, rect.top());
	// top edge
	path.lineTo(rect.right() - radius, rect.top());
	// top-right corner
	path.arcTo(rect.right() - 2 * radius, rect.top(), 2 * radius, 2 * radius, 90, -90);
	// right edge
	path.lineTo(rect.right(), rect.bottom());
	// bottom edge
	path.lineTo(rect.left(), rect.bottom());
	// left edge
	path.lineTo(rect.left(), rect.top() + radius);
	// top-left corner
	path.arcTo(rect.left(), rect.top(), 2 * radius, 2 * radius, 180, -90);
	// finish
	path.closeSubpath();
	return path;
}

qreal getPortY(int index)
{
	return BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS + ROW_HEIGHT * index;
}

} // namespace

NodeItem::NodeItem(QVariant id, const QString& title) : _id(std::move(id)), _title(title)
{
	setFlag(ItemIsMovable, true);
	setFlag(ItemSendsGeometryChanges, true);
	setAcceptHoverEvents(true);
	relayout();
}

QVariant NodeItem::id() const
{
	return _id;
}

void NodeItem::setTitle(const QString& title)
{
	_title = title;
	update();
}

PortItem* NodeItem::addInputPort(const QString& name)
{
	const int index = _inputs.size();
	auto* port = new PortItem(PortDirection::Input, index, name, this);
	port->setPos(QPointF(BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2, getPortY(index)));
	connect(port, &PortItem::hoverChanged, this, &NodeItem::onPortHoverChanged);
	_inputs.append(port);
	relayout();
	return port;
}

PortItem* NodeItem::addOutputPort(const QString& name)
{
	const int index = _outputs.size();
	auto* port = new PortItem(PortDirection::Output, _outputs.size(), name, this);
	port->setPos(QPointF(_width - (BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2), getPortY(index)));
	connect(port, &PortItem::hoverChanged, this, &NodeItem::onPortHoverChanged);
	_outputs.append(port);
	relayout();
	return port;
}

QPointF NodeItem::inputScenePos(int index) const
{
	const auto* port = inputPort(index);
	return port ? port->scenePos() : QPointF();
}

QPointF NodeItem::outputScenePos(int index) const
{
	const auto* port = outputPort(index);
	return port ? port->scenePos() : QPointF();
}

PortItem* NodeItem::inputPort(int index) const
{
	if (index < 0 || index >= _inputs.size())
		return nullptr;
	return _inputs[index];
}

PortItem* NodeItem::outputPort(int index) const
{
	if (index < 0 || index >= _outputs.size())
		return nullptr;
	return _outputs[index];
}

int NodeItem::inputsCount() const
{
	return _inputs.size();
}

int NodeItem::outputsCount() const
{
	return _outputs.size();
}

QRectF NodeItem::boundingRect() const
{
	// Margin so the border pen is not clipped; ports are child items with
	// their own bounding rectangles.
	const double margin = BODY_BORDER_WIDTH / 2;
	return QRectF(QPointF(0, 0), QSizeF(_width, _height)).adjusted(-margin, -margin, margin, margin);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setRenderHint(QPainter::Antialiasing, true);
	paintBody(painter);
	paintHeader(painter);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = true;
	update();
	QGraphicsItem::hoverEnterEvent(event);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = false;
	update();
	QGraphicsItem::hoverLeaveEvent(event);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemPositionHasChanged)
		emit positionChanged();
	return QGraphicsItem::itemChange(change, value);
}

void NodeItem::onPortHoverChanged(bool hovered)
{
	_hoveredPortCount += hovered ? 1 : -1;
	update();
}

void NodeItem::relayout()
{
	const int rows = qMax(qMax(_inputs.size(), _outputs.size()), static_cast<int>(MIN_BODY_ROWS));
	_width = BODY_WIDTH;
	_height = HEADER_HEIGHT + ROW_HEIGHT * rows + BOTTOM_PADDING;
	prepareGeometryChange();
	update();
}

void NodeItem::paintBody(QPainter* painter) const
{
	const QRectF body(0, 0, _width, _height);
	const bool highlighted = _hovered || _hoveredPortCount > 0;
	const QColor borderColor = highlighted ? color::BORDER_HOVER : color::BORDER;
	const QPen borderPen(borderColor, BODY_BORDER_WIDTH);
	painter->setPen(borderPen);
	painter->setBrush(QBrush(color::BACKGROUND));
	painter->drawRoundedRect(body, BODY_RADIUS, BODY_RADIUS);
}

void NodeItem::paintHeader(QPainter* painter) const
{
	// Header background
	const auto headerRect =
	    QRectF(BODY_BORDER_WIDTH, BODY_BORDER_WIDTH, _width - 2 * BODY_BORDER_WIDTH, HEADER_HEIGHT);
	const auto header = createTopRoundedRect(headerRect, BODY_RADIUS);
	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(color::HEADER));
	painter->drawPath(header);

	// Title centered in the header
	QFont titleFont = painter->font();
	titleFont.setPointSize(12);
	titleFont.setBold(true);
	painter->setFont(titleFont);
	painter->setPen(QPen(color::NODE_TITLE));
	painter->drawText(headerRect, Qt::AlignCenter, _title);
}

} // namespace QNodeFlow
