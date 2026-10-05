#include "NodeItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace {

constexpr double HEADER_HEIGHT = 32.0;     // header height
constexpr double ROW_HEIGHT = 36.0;        // port row height
constexpr double BOTTOM_PADDING = 20.0;    // bottom padding
constexpr double MIN_BODY_ROWS = 2;        // minimum rows so the node is not a thin strip
constexpr double BODY_WIDTH = 320.0;       // body width
constexpr double BODY_BORDER_WIDTH = 3;    // body border thickness
constexpr double BODY_RADIUS = 10;         // body angle radius
constexpr double PORT_RADIUS = 8.0;        // port radius
constexpr double PORT_LINE_WIDTH = 2.0;    // circle outline thickness
constexpr double PORTS_TOP_PADDING = 10.0; // distance between the header and the top ports
constexpr double PORT_LABEL_GAP = 12.0;    // gap between a port and its label

namespace color {
constexpr QColor BACKGROUND(23, 25, 30);          // body background
constexpr QColor BORDER(60, 64, 72);              // body border
constexpr QColor BORDER_HOVER(120, 170, 210);     // body border on hover
constexpr QColor HEADER(16, 18, 22);              // header background
constexpr QColor NODE_TITLE(235, 238, 242);       // title color
constexpr QColor PORT_FILL(46, 125, 110);         // port fill
constexpr QColor PORT_BORDER(58, 190, 150);       // port outline
constexpr QColor PORT_FILL_HOVER(66, 165, 145);   // port fill on hover
constexpr QColor PORT_BORDER_HOVER(96, 230, 190); // port outline on hover
constexpr QColor PORT_LABEL(190, 194, 200);       // port label color
} // namespace color

// Radius within which the cursor is considered over a port.
constexpr double PORT_HIT_RADIUS = 3.0;

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

bool isPortHovered(const QPointF& center, const QPointF& localPos)
{
	const double dx = localPos.x() - center.x();
	const double dy = localPos.y() - center.y();
	const double r = PORT_RADIUS + PORT_HIT_RADIUS;
	return dx * dx + dy * dy <= r * r;
}

} // namespace

namespace QNodeFlow {

NodeItem::NodeItem(QVariant id, const QString& title, QGraphicsItem* parent)
    : QGraphicsObject(parent), _id(std::move(id)), _title(title)
{
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
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

void NodeItem::setInputs(const QVector<Port>& ports)
{
	_inputs = ports;
	relayout();
}

void NodeItem::setOutputs(const QVector<Port>& ports)
{
	_outputs = ports;
	relayout();
}

QPointF NodeItem::inputScenePos(int index) const
{
	return mapToScene(inputPortCenter(index));
}

QPointF NodeItem::outputScenePos(int index) const
{
	return mapToScene(outputPortCenter(index));
}

QRectF NodeItem::boundingRect() const
{
	// Account for ports protruding past the body edge and the outline thickness.
	const double margin = PORT_RADIUS - BODY_BORDER_WIDTH / 2 + 3.0;
	return QRectF(QPointF(0, 0), QSizeF(_width, _height)).adjusted(-margin, -margin, margin, margin);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setRenderHint(QPainter::Antialiasing, true);
	paintBody(painter);
	paintHeader(painter);
	paintPorts(painter);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		// ItemIsMovable handles dragging; here we only acknowledge the press.
		QGraphicsItem::mousePressEvent(event);
		return;
	}
	QGraphicsItem::mousePressEvent(event);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = true;
	updateHoveredPort(mapFromScene(event->scenePos()));
	update();
	QGraphicsItem::hoverEnterEvent(event);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	_hovered = false;
	_hoveredInput = -1;
	_hoveredOutput = -1;
	update();
	QGraphicsItem::hoverLeaveEvent(event);
}

void NodeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	updateHoveredPort(mapFromScene(event->scenePos()));
	QGraphicsItem::hoverMoveEvent(event);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemPositionHasChanged)
		emit positionChanged();
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

void NodeItem::updateHoveredPort(const QPointF& localPos)
{
	int in = -1;
	for (int i = 0; i < _inputs.size(); ++i)
	{
		if (isPortHovered(inputPortCenter(i), localPos))
		{
			in = i;
			break;
		}
	}

	int out = -1;
	for (int i = 0; i < _outputs.size(); ++i)
	{
		if (isPortHovered(outputPortCenter(i), localPos))
		{
			out = i;
			break;
		}
	}

	if (in != _hoveredInput || out != _hoveredOutput)
	{
		_hoveredInput = in;
		_hoveredOutput = out;
		update();
	}
}

QPointF NodeItem::inputPortCenter(int index) const
{
	if (index < 0 || index >= _inputs.size())
		return QPointF();
	return QPointF(BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2,
	               BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS +
	                   ROW_HEIGHT * index);
}

QPointF NodeItem::outputPortCenter(int index) const
{
	if (index < 0 || index >= _outputs.size())
		return QPointF();
	return QPointF(_width - (BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2),
	               BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS +
	                   ROW_HEIGHT * index);
}

void NodeItem::paintBody(QPainter* painter) const
{
	const QRectF body(0, 0, _width, _height);
	const QColor borderColor = _hovered ? color::BORDER_HOVER : color::BORDER;
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

void NodeItem::paintPorts(QPainter* painter) const
{
	// Port labels.
	QFont labelFont = painter->font();
	labelFont.setPointSize(12);
	painter->setFont(labelFont);
	painter->setPen(QPen(color::PORT_LABEL));

	// Inputs: port on the left edge, label to its right.
	for (int i = 0; i < _inputs.size(); ++i)
	{
		const QPointF c = inputPortCenter(i);
		painter->drawText(QRectF(c.x() + PORT_RADIUS + PORT_LABEL_GAP, c.y() - ROW_HEIGHT / 2,
		                         _width / 2, ROW_HEIGHT),
		                  Qt::AlignVCenter | Qt::AlignLeft, _inputs[i].name);
	}

	// Outputs: port on the right edge, label to its left.
	for (int i = 0; i < _outputs.size(); ++i)
	{
		const QPointF c = outputPortCenter(i);
		painter->drawText(
		    QRectF(0, c.y() - ROW_HEIGHT / 2, c.x() - PORT_RADIUS - PORT_LABEL_GAP, ROW_HEIGHT),
		    Qt::AlignVCenter | Qt::AlignRight, _outputs[i].name);
	}

	// Draw the port circle after the labels so it sits on top.
	auto drawPort = [&](const QPointF& center, bool hovered)
	{
		const QColor border = hovered ? color::PORT_BORDER_HOVER : color::PORT_BORDER;
		const QColor fill = hovered ? color::PORT_FILL_HOVER : color::PORT_FILL;
		painter->setPen(QPen(border, PORT_LINE_WIDTH));
		painter->setBrush(QBrush(fill));
		painter->drawEllipse(center, PORT_RADIUS, PORT_RADIUS);
	};
	for (int i = 0; i < _inputs.size(); ++i)
		drawPort(inputPortCenter(i), i == _hoveredInput);
	for (int i = 0; i < _outputs.size(); ++i)
		drawPort(outputPortCenter(i), i == _hoveredOutput);
}

} // namespace QNodeFlow
