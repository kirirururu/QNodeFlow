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

} // namespace

NodeItem::NodeItem(QVariant id, QString title)
    : BasicNodeItem(std::move(id), std::move(title)), NodeWithInputs(), NodeWithOutputs()
{
	setFlag(ItemIsMovable, true);
	setFlag(ItemSendsGeometryChanges, true);
	setAcceptHoverEvents(true);
	NodeItem::relayout();
}

QRectF NodeItem::boundingRect() const
{
	// Margin so the border pen is not clipped; ports are child items with
	// their own bounding rectangles.
	// const double margin = BODY_BORDER_WIDTH / 2;
	return QRectF(QPointF(0, 0), QSizeF(_width, _height));
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
	{
		emit positionChanged();
		for (auto* port : _inputs)
			emit port->positionChanged();
		for (auto* port : _outputs)
			emit port->positionChanged();
	}
	return QGraphicsItem::itemChange(change, value);
}

void NodeItem::relayout()
{
	const int rows = qMax(qMax(_inputs.size(), _outputs.size()), static_cast<int>(MIN_BODY_ROWS));
	_width = BODY_WIDTH;
	_height = HEADER_HEIGHT + ROW_HEIGHT * rows + BOTTOM_PADDING;
	prepareGeometryChange();
	update();
}

double NodeItem::getPortY(int index)
{
	return BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS + ROW_HEIGHT * index;
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
	painter->drawText(headerRect, Qt::AlignCenter, title());
}

} // namespace QNodeFlow
