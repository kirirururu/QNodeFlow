#pragma once

#include <QGraphicsObject>
#include <QPointF>
#include <QString>

class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;

namespace QNodeFlow {

class NodeItem;

enum class PortDirection
{
	Input,
	Output
};

/**
 * A single node port (circle + label), a child item of NodeItem.
 *
 * The item's local origin is the port center, so scenePos() is the port center.
 *
 * Output ports are the start of connection drawing: pressing the left mouse button on
 * one and dragging to an input port creates a connection. The press is accepted here
 * (so the movable node does not start dragging); the drag itself is tracked by the
 * owning NodeView.
 */
class PortItem : public QGraphicsObject
{
	Q_OBJECT

public:
	PortItem(PortDirection direction, int index, const QString& name, NodeItem* parent = nullptr);

	PortDirection direction() const;
	int index() const;
	QString name() const;
	NodeItem* node() const;

	// Port center in scene coordinates.
	QPointF scenePos() const;

	// Highlights the port as a valid drop target during connection drawing.
	void setTargeted(bool targeted);

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

	QPainterPath shape() const override;

signals:
	void hoverChanged(bool hovered);

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	QRectF labelRect() const;

	PortDirection _direction;
	int _index;
	QString _name;

	// hovered property for different modes
	bool _hovered = false;  // normal mode
	bool _targeted = false; // connection drawing mode
};

} // namespace QNodeFlow
