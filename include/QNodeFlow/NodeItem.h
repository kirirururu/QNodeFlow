#pragma once

#include <QGraphicsObject>
#include <QString>
#include <QVariant>
#include <QVector>

class QGraphicsSceneHoverEvent;

namespace QNodeFlow {

class PortItem;

/**
 * A displayable node with an arbitrary number of input and output ports.
 *
 * Each port is a separate PortItem, a child of the node: it moves with the node
 * and is drawn on top of the node body.
 */
class NodeItem : public QGraphicsObject
{
	Q_OBJECT

public:
	struct Port
	{
		QString name;
	};

	NodeItem(QVariant id, const QString& title);

	QVariant id() const;

	void setTitle(const QString& title);

	PortItem* addInputPort(const QString& name);
	PortItem* addOutputPort(const QString& name);

	// Center position of a port in scene coordinates (for future connection wiring).
	QPointF inputScenePos(int index) const;
	QPointF outputScenePos(int index) const;

	PortItem* inputPort(int index) const;
	PortItem* outputPort(int index) const;

	int inputsCount() const;
	int outputsCount() const;

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

signals:
	void positionChanged();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

	// Notify subscribers about a node position change (to redraw connections).
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private slots:
	void onPortHoverChanged(bool hovered);

private:
	void relayout();

	void paintBody(QPainter* painter) const;
	void paintHeader(QPainter* painter) const;

	QVariant _id;
	QString _title;
	QVector<PortItem*> _inputs;
	QVector<PortItem*> _outputs;

	// Current geometric size of the node.
	double _width = 0.0;
	double _height = 0.0;

	// Cursor hover state.
	bool _hovered = false;     // cursor over the node body
	int _hoveredPortCount = 0; // ports currently under the cursor, TODO: replace with bool
};

} // namespace QNodeFlow
