#pragma once

#include "BasicNodeItem.h"

class QGraphicsSceneHoverEvent;

namespace QNodeFlow {

/**
 * A displayable node with an arbitrary number of input and output ports.
 *
 * Each port is a separate PortItem, a child of the node: it moves with the node
 * and is drawn on top of the node body.
 */
class NodeItem final : public QObject, public NodeWithInputs, public NodeWithOutputs
{
	Q_OBJECT

public:
	NodeItem(QVariant id, QString title);

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

	void relayout() override;
	double getPortY(int index) override;

	void paintBody(QPainter* painter) const;
	void paintHeader(QPainter* painter) const;


	// Cursor hover state.
	bool _hovered = false; // cursor over the node body
};

} // namespace QNodeFlow
