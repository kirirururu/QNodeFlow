#include "TemporaryConnectionItem.h"

#include "ConnectionItem.h"

#include <QPainter>

namespace QNodeFlow {

namespace {
namespace color {
constexpr QColor TEMP_LINE_COLOR(120, 160, 200); // color of the line being drawn
}
} // namespace

TemporaryConnectionItem::TemporaryConnectionItem(PortItem* from) : _from(from)
{
	setZValue(1.0); // draw on top of everything
	_path.reset(new QPainterPath());
}

void TemporaryConnectionItem::setTo(const QPointF& to)
{
	_target = to;
	_hasTarget = true;
	prepareGeometryChange();
	*_path = ConnectionItem::buildPath(_from, to);
	update();
}

QRectF TemporaryConnectionItem::boundingRect() const
{
	if (!_hasTarget)
		return QRectF();
	constexpr double margin = 4.0;
	return _path->boundingRect().adjusted(-margin, -margin, margin, margin);
}

void TemporaryConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (!_hasTarget)
		return;
	painter->setRenderHint(QPainter::Antialiasing, true);
	QPen pen(color::TEMP_LINE_COLOR, 2.0, Qt::DashLine);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setCapStyle(Qt::RoundCap);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(*_path);
}

} // namespace QNodeFlow
