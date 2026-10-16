#pragma once

#include <QGraphicsObject>

class QPainterPath;

namespace QNodeFlow {

class PortItem;

/**
 * A connection line between two node ports
 */
class ConnectionItem final : public QGraphicsObject
{
	Q_OBJECT

public:
	ConnectionItem(PortItem* from, PortItem* to);

	PortItem* from() const { return _from; }
	PortItem* to() const { return _to; }

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

	// Build the connection path between source port and a scene-space point
	static QPainterPath buildPath(const PortItem* from, const QPointF& to);

	friend bool operator==(const ConnectionItem& lhs, const ConnectionItem& rhs);
	friend bool operator!=(const ConnectionItem& lhs, const ConnectionItem& rhs);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	void refresh();

	PortItem* _from;
	PortItem* _to;
};

} // namespace QNodeFlow
