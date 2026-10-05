#pragma once

#include <QGraphicsObject>
#include <QString>
#include <QVector>

class QGraphicsSceneHoverEvent;

namespace QNodeFlow {

/**
 * A displayable node with an arbitrary number of input and output ports
 */
class NodeItem : public QGraphicsObject
{
	Q_OBJECT

public:
	struct Port
	{
		QString name;
	};

	explicit NodeItem(const QString& title, QGraphicsItem* parent = nullptr);

	void setTitle(const QString& title);
	void setInputs(const QVector<Port>& ports);
	void setOutputs(const QVector<Port>& ports);

	// Center position of a port in scene coordinates (for future connection wiring).
	QPointF inputScenePos(int index) const;
	QPointF outputScenePos(int index) const;

	QRectF boundingRect() const override;
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;

signals:
	void positionChanged();

protected:
	// Support dragging with the left mouse button.
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	// Cursor hover events: highlight the node border and the hovered port.
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

	// Notify subscribers about a node position change (to redraw connections).
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	void relayout();
	void updateHoveredPort(const QPointF& localPos);

	QPointF inputPortCenter(int index) const;
	QPointF outputPortCenter(int index) const;

	void paintBody(QPainter* painter) const;
	void paintHeader(QPainter* painter) const;
	void paintPorts(QPainter* painter) const;

	QString _title;
	QVector<Port> _inputs;
	QVector<Port> _outputs;

	// Current geometric size of the node.
	double _width = 0.0;
	double _height = 0.0;

	// Cursor hover state.
	bool _hovered = false;   // cursor over the node
	int _hoveredInput = -1;  // index of hovered input, -1 = none
	int _hoveredOutput = -1; // index of hovered output, -1 = none
};

} // namespace QNodeFlow
