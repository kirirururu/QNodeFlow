#pragma once

#include <QGraphicsObject>

class QPainterPath;

namespace QNodeFlow {

class NodeItem;

/**
 * A connection line between two node ports
 */
class ConnectionItem : public QGraphicsObject
{
	Q_OBJECT

public:
	struct PortRef
	{
		NodeItem* node = nullptr;
		bool isInput = false; // false = output, true = input
		int index = 0;

		// Current position of the port in scene coordinates.
		QPointF scenePos() const;
	};

	ConnectionItem(const PortRef& from, const PortRef& to, QGraphicsItem* parent = nullptr);

	// Connection ends: "from" is the source (output), "to" is the receiver (input).
	const PortRef& from() const { return _from; }
	const PortRef& to() const { return _to; }

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

private slots:
	void refresh();

private:
	QPainterPath buildPath() const;

	PortRef _from;
	PortRef _to;
};

} // namespace QNodeFlow
