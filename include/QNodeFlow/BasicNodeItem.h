#pragma once

#include <QGraphicsObject>
#include <QString>
#include <QVariant>
#include <QVector>

namespace QNodeFlow {

class PortItem;

class BasicNodeItem : public QGraphicsItem
{
public:
	explicit BasicNodeItem(QVariant id, QString title);

	const QVariant& id() const;

	const QString& title() const;
	void setTitle(QString title);

protected:
	void onPortHoverChanged(bool hovered);

	virtual void relayout() = 0;
	virtual double getPortY(int index) = 0;

	// Current geometric size of the node.
	double _width = 0.0;
	double _height = 0.0;

	int _hoveredPortCount = 0;

private:
	QVariant _id;
	QString _title;
};

class NodeWithInputs : virtual public BasicNodeItem
{
public:
	PortItem* addInputPort(const QString& name);
	PortItem* inputPort(int index) const;
	QPointF inputScenePos(int index) const;
	int inputsCount() const;

protected:
	QVector<PortItem*> _inputs;
};

class NodeWithOutputs : virtual public BasicNodeItem
{
public:
	PortItem* addOutputPort(const QString& name);
	PortItem* outputPort(int index) const;
	QPointF outputScenePos(int index) const;
	int outputsCount() const;

protected:
	QVector<PortItem*> _outputs;
};

} // namespace QNodeFlow
