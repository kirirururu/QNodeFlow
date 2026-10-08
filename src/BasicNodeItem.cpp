#include "BasicNodeItem.h"

#include "PortItem.h"
#include "Style.h"

namespace QNodeFlow {

BasicNodeItem::BasicNodeItem(QVariant id, QString title)
    : _id(std::move(id)), _title(std::move(title))
{
}

const QVariant& BasicNodeItem::id() const
{
	return _id;
}

const QString& BasicNodeItem::title() const
{
	return _title;
}

void BasicNodeItem::setTitle(QString title)
{
	_title = std::move(title);
	update();
}

void BasicNodeItem::onPortHoverChanged(bool hovered)
{
	_hoveredPortCount += hovered ? 1 : -1;
	update();
}

////////////////////////////////////////////////////////////

PortItem* NodeWithInputs::addInputPort(const QString& name)
{
	const int index = _inputs.size();
	auto* port = new PortItem(PortDirection::Input, index, name, this);
	port->setPos(QPointF(BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2, getPortY(index)));
	QObject::connect(port, &PortItem::hoverChanged,
	                 [this](bool hovered) { onPortHoverChanged(hovered); });
	_inputs.append(port);
	relayout();
	return port;
}

PortItem* NodeWithInputs::inputPort(int index) const
{
	if (index < 0 || index >= _inputs.size())
		return nullptr;
	return _inputs[index];
}

QPointF NodeWithInputs::inputScenePos(int index) const
{
	const auto* port = inputPort(index);
	return port ? port->scenePos() : QPointF();
}

int NodeWithInputs::inputsCount() const
{
	return _inputs.size();
}

////////////////////////////////////////////////////////////

PortItem* NodeWithOutputs::addOutputPort(const QString& name)
{
	const int index = _outputs.size();
	auto* port = new PortItem(PortDirection::Output, _outputs.size(), name, this);
	port->setPos(QPointF(_width - (BODY_BORDER_WIDTH / 2 - PORT_LINE_WIDTH / 2), getPortY(index)));
	QObject::connect(port, &PortItem::hoverChanged,
	                 [this](bool hovered) { onPortHoverChanged(hovered); });
	_outputs.append(port);
	relayout();
	return port;
}

PortItem* NodeWithOutputs::outputPort(int index) const
{
	if (index < 0 || index >= _outputs.size())
		return nullptr;
	return _outputs[index];
}

QPointF NodeWithOutputs::outputScenePos(int index) const
{
	const auto* port = outputPort(index);
	return port ? port->scenePos() : QPointF();
}

int NodeWithOutputs::outputsCount() const
{
	return _outputs.size();
}

} // namespace QNodeFlow
