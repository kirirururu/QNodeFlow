#include "SelectionItem.h"

#include "Style.h"

#include <QPainter>

namespace QNodeFlow {

SelectionItem::SelectionItem()
{
	setZValue(1000.0); // draw on top of everything, above the temporary connection

	// The item must not catch the mouse: while it is visible the pressed button is
	// already held, and a zero-size rect does not contain any point.
	setAcceptedMouseButtons(Qt::NoButton);
}

void SelectionItem::setRect(QRectF rect)
{
	if (_rect == rect)
		return;
	prepareGeometryChange();
	_rect = std::move(rect);
	update();
}

const QRectF& SelectionItem::rect() const
{
	return _rect;
}

QRectF SelectionItem::boundingRect() const
{
	return _rect;
}

void SelectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (_rect.isEmpty())
		return;
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->fillRect(_rect, color::SELECTION_FILL);
	painter->setPen(QPen(color::SELECTION_BORDER, 1.0));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(_rect);
}

} // namespace QNodeFlow
