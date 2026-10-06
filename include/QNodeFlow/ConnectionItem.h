#pragma once

#include <QGraphicsObject>

class QPainterPath;

namespace QNodeFlow {

class PortItem;

/**
 * A connection line between two node ports
 */
class ConnectionItem : public QGraphicsObject
{
	Q_OBJECT

public:
	ConnectionItem(PortItem* from, PortItem* to);

	PortItem* from() const { return _from; }
	PortItem* to() const { return _to; }

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

	friend bool operator==(const ConnectionItem& lhs, const ConnectionItem& rhs);
	friend bool operator!=(const ConnectionItem& lhs, const ConnectionItem& rhs);

private slots:
	void refresh();

private:
	QPainterPath buildPath() const;

	PortItem* _from;
	PortItem* _to;
};

} // namespace QNodeFlow
