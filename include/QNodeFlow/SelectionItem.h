#pragma once

#include <QGraphicsItem>
#include <QRectF>

namespace QNodeFlow {

/**
 * Rectangle of the rubber-band node selection, drawn on top of the other scene items.
 *
 * The item lives in scene coordinates and is non-interactive: it is created by NodeView,
 * positioned while the user drags on the empty background, and hidden when the drag ends.
 */
class SelectionItem : public QGraphicsItem
{
public:
	SelectionItem();

	void setRect(QRectF rect);
	const QRectF& rect() const;

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

private:
	QRectF _rect;
};

} // namespace QNodeFlow
