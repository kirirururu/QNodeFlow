#pragma once

#include "BasicNodeItem.h"

#include <QGraphicsObject>

namespace QNodeFlow {

class NodeView;

class GlobalInputNodeItem final : public QObject, public NodeWithOutputs
{
	Q_OBJECT
public:
	explicit GlobalInputNodeItem(QVariant id, QString title, NodeView* view);

	void updatePosition();

protected:
	void relayout() override;
	double getPortY(int index) override;

public:
	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
	NodeView* _view;
};

class GlobalOutputNodeItem final : public QObject, public NodeWithInputs
{
	Q_OBJECT
public:
	explicit GlobalOutputNodeItem(QVariant id, QString title, NodeView* view);

	void updatePosition();

protected:
	void relayout() override;
	double getPortY(int index) override;

public:
	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
	NodeView* _view;
};

} // namespace QNodeFlow
