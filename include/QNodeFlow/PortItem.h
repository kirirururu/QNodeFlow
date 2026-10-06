#pragma once

#include <QGraphicsObject>
#include <QString>

class QGraphicsSceneHoverEvent;

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

	bool _hovered = false;
};

} // namespace QNodeFlow
