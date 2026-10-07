#pragma once

#include <QGraphicsItem>

namespace QNodeFlow {

class PortItem;

/**
 * A lightweight line shown while a connection is being dragged from an output port.
 */
class TemporaryConnectionItem : public QGraphicsItem
{
public:
	explicit TemporaryConnectionItem(PortItem* from);

	void setTo(const QPointF& to);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
	PortItem* _from;
	QPointF _target;
	bool _hasTarget = false;
};

} // namespace QNodeFlow
