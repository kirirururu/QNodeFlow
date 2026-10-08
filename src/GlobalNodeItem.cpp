#include "GlobalNodeItem.h"

#include "NodeView.h"
#include "PortItem.h"
#include "Style.h"

namespace QNodeFlow {

namespace {

constexpr double PANEL_WIDTH = 150;

void drawHeader(QPainter* painter, const QString& title, qreal width)
{
	// Header background
	const auto headerRect =
	    QRectF(BODY_BORDER_WIDTH, BODY_BORDER_WIDTH, width - 2 * BODY_BORDER_WIDTH, HEADER_HEIGHT);
	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(color::HEADER));
	painter->drawRect(headerRect);

	// Title centered in the header
	QFont titleFont = painter->font();
	titleFont.setPointSize(12);
	titleFont.setBold(true);
	painter->setFont(titleFont);
	painter->setPen(QPen(color::NODE_TITLE));
	painter->drawText(headerRect, Qt::AlignCenter, title);
}

} // namespace

GlobalInputNodeItem::GlobalInputNodeItem(QVariant id, QString title, NodeView* view)
    : BasicNodeItem(std::move(id), std::move(title)), NodeWithOutputs(), _view(view)
{
	Q_PRE(_view);
	setFlag(ItemIsMovable, false);
	setFlag(ItemIgnoresTransformations);
	setZValue(1000);

	_width = PANEL_WIDTH;
	updatePosition();
}

void GlobalInputNodeItem::updatePosition()
{
	_height = _view->viewport()->height();

	QPointF scenePos = _view->mapToScene(QPoint(0, 0));
	setPos(scenePos.x(), _view->mapToScene(QPoint(0, 0)).y());
	update();
	for (auto* port : _outputs)
		emit port->positionChanged();
}

void GlobalInputNodeItem::relayout()
{
}

double GlobalInputNodeItem::getPortY(int index)
{
	return BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS + ROW_HEIGHT * index;
}

QRectF GlobalInputNodeItem::boundingRect() const
{
	return QRectF(QPointF(0, 0), QPointF(_width, _height));
}

void GlobalInputNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QRectF body = boundingRect();
	const QPen borderPen(color::BORDER, BODY_BORDER_WIDTH);
	painter->setPen(borderPen);
	painter->setBrush(QBrush(color::BACKGROUND));
	painter->drawRect(body);

	drawHeader(painter, title(), _width);
}

////////////////////////////////////////////////////////////

GlobalOutputNodeItem::GlobalOutputNodeItem(QVariant id, QString title, NodeView* view)
    : BasicNodeItem(std::move(id), std::move(title)), NodeWithInputs(), _view(view)
{
	Q_PRE(_view);
	setFlag(ItemIsMovable, false);
	setFlag(ItemIgnoresTransformations);
	setZValue(1000);

	_width = PANEL_WIDTH;
	updatePosition();
}

void GlobalOutputNodeItem::updatePosition()
{
	_height = _view->viewport()->height();

	setPos(_view->mapToScene(QPoint(_view->viewport()->width() - _width, 0)));
	update();
	for (auto* port : _inputs)
		emit port->positionChanged();
}

void GlobalOutputNodeItem::relayout()
{
}

double GlobalOutputNodeItem::getPortY(int index)
{
	return BODY_BORDER_WIDTH + HEADER_HEIGHT + PORTS_TOP_PADDING + PORT_RADIUS + ROW_HEIGHT * index;
}

QRectF GlobalOutputNodeItem::boundingRect() const
{
	return QRectF(QPointF(0, 0), QPointF(_width, _height));
}

void GlobalOutputNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QRectF body = boundingRect();
	const QPen borderPen(color::BORDER, BODY_BORDER_WIDTH);
	painter->setPen(borderPen);
	painter->setBrush(QBrush(color::BACKGROUND));
	painter->drawRect(body);

	drawHeader(painter, title(), _width);
}

} // namespace QNodeFlow
