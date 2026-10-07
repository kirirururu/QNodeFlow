#include "NodeView.h"

#include "ConnectionItem.h"
#include "NodeItem.h"
#include "PortItem.h"
#include "TemporaryConnectionItem.h"

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>

namespace QNodeFlow {

NodeView::NodeView(QMetaType keyMetaType, QWidget* parent)
    : QGraphicsView(parent), _idMetaType(keyMetaType), _scene(new QGraphicsScene(this))
{
	Q_ASSERT(_idMetaType.id() != QMetaType::UnknownType);
	setScene(_scene);
	setRenderHint(QPainter::Antialiasing, true);
	setDragMode(QGraphicsView::NoDrag); // the node itself is dragged, not the scene
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setBackgroundBrush(QColor(42, 44, 50));
	recalcSceneRect();
}

NodeView::~NodeView()
{
	// Connections first (they reference nodes), then the nodes themselves.
	for (const auto& connection : _connections)
		connection->deleteLater();
	for (const auto& [id, node] : _nodes)
		node->deleteLater();

	if (_tempConnection)
		delete _tempConnection;
}

void NodeView::addNode(const QVariant& id, NodeItem* node)
{
	checkIdType(id);
	if (node == nullptr)
		throw std::invalid_argument("node is nullptr");
	if (_nodes.find(id) != _nodes.end())
		throw std::invalid_argument("duplicate node id");
	_nodes.insert({id, node});
	_scene->addItem(node);
	recalcSceneRect();
}

NodeItem* NodeView::findNode(const QVariant& id) const
{
	checkIdType(id);
	if (const auto iter = _nodes.find(id); iter != _nodes.end())
		return iter->second;
	return nullptr;
}

void NodeView::removeNode(const QVariant& id)
{
	checkIdType(id);
	const auto nodeIter = _nodes.find(id);
	if (nodeIter == _nodes.end())
		return;
	auto* node = nodeIter->second;
	removeConnectionsForNode(node);
	_scene->removeItem(node);
	node->deleteLater();
	_nodes.erase(nodeIter);
	recalcSceneRect();
}

void NodeView::addConnection(const QVariant& sourceId,
                             int sourcePort,
                             const QVariant& destinationId,
                             int destinationPort)
{
	const auto source = findNode(sourceId);
	if (!source)
		throw std::runtime_error("source node not found");
	const auto destination = findNode(destinationId);
	if (!destination)
		throw std::runtime_error("destination node not found");
	addConnection(source, sourcePort, destination, destinationPort);
}

void NodeView::addConnection(NodeItem* source, int sourcePort, NodeItem* destination, int destinationPort)
{
	if (source == nullptr || destination == nullptr)
		throw std::invalid_argument("source or destination node is nullptr");

	const auto srcPort = source->outputPort(sourcePort);
	const auto dstPort = destination->inputPort(destinationPort);
	if (!srcPort || !dstPort)
		throw std::invalid_argument("invalid source or destination port index");

	const auto connection = new ConnectionItem(srcPort, dstPort);
	_scene->addItem(connection);
	_connections.append(connection);
	recalcSceneRect();
}

void NodeView::removeConnection(const QVariant& sourceId,
                                int sourcePort,
                                const QVariant& destinationId,
                                int destinationPort)
{
	const auto source = findNode(sourceId);
	if (!source)
		throw std::runtime_error("source node not found");
	const auto destination = findNode(destinationId);
	if (!destination)
		throw std::runtime_error("destination node not found");

	removeConnection(source, sourcePort, destination, destinationPort);
}

void NodeView::removeConnection(NodeItem* source,
                                int sourcePort,
                                NodeItem* destination,
                                int destinationPort)
{
	const auto srcPort = source->outputPort(sourcePort);
	const auto dstPort = destination->inputPort(destinationPort);
	if (!srcPort || !dstPort)
		throw std::invalid_argument("invalid source or destination port index");

	ConnectionItem target{srcPort, dstPort};
	auto iter =
	    std::find_if(_connections.begin(), _connections.end(),
	                 [&target](const ConnectionItem* connection) { return *connection == target; });
	if (iter != _connections.end())
	{
		ConnectionItem* connection = *iter;
		_scene->removeItem(connection);
		_connections.erase(iter);
		connection->deleteLater();
	}
	recalcSceneRect();
}

void NodeView::removeConnectionsForNode(const NodeItem* node)
{
	for (auto it = _connections.begin(); it != _connections.end();)
	{
		ConnectionItem* connection = *it;
		if (connection->from()->parentItem() == node || connection->to()->parentItem() == node)
		{
			_scene->removeItem(connection);
			it = _connections.erase(it);
			delete connection;
		}
		else
		{
			++it;
		}
	}
}

bool NodeView::viewportEvent(QEvent* event)
{
	// The viewport implicitly captures the mouse while a button is held, so it
	// receives all move/release events of an in-progress drag.
	switch (event->type())
	{
	case QEvent::MouseButtonPress:
	{
		const auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton)
			startConnectionDrag(mapToScene(mouseEvent->pos()));
		break;
	}
	case QEvent::MouseMove:
	{
		if (_dragSource)
			updateConnectionDrag(mapToScene(static_cast<QMouseEvent*>(event)->pos()));
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		const auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton && _dragSource)
			finishConnectionDrag();
		break;
	}
	default:
		break;
	}
	return QGraphicsView::viewportEvent(event);
}

void NodeView::startConnectionDrag(const QPointF& scenePos)
{
	Q_PRE(!_tempConnection);
	// The press is forwarded to the scene below; the node must be made
	// non-movable first so it cannot start dragging over the inner half of the port.
	auto* source = findPortAtPosition(scenePos);
	if (!source || source->direction() != PortDirection::Output)
		return;

	_dragSource = source;
	_dragSource->node()->setFlag(QGraphicsItem::ItemIsMovable, false);
	_dragSource->setTargeted(true);

	_dragTarget = nullptr;

	_tempConnection = new TemporaryConnectionItem(_dragSource);
	_scene->addItem(_tempConnection);
}

void NodeView::updateConnectionDrag(const QPointF& scenePos)
{
	Q_PRE(_tempConnection);
	_tempConnection->setTo(scenePos);

	// Highlight the input port currently under the cursor, if any.
	auto* targetPort = findPortAtPosition(scenePos);
	if (targetPort && targetPort->direction() != PortDirection::Input)
		targetPort = nullptr;
	if (targetPort != _dragTarget)
	{
		if (_dragTarget)
			_dragTarget->setTargeted(false);
		_dragTarget = targetPort;
		if (targetPort)
			targetPort->setTargeted(true);
	}
}

void NodeView::finishConnectionDrag()
{
	if (_dragTarget)
	{
		_dragTarget->setTargeted(false);
		addConnection(_dragSource->node(), _dragSource->index(), _dragTarget->node(),
		              _dragTarget->index());
		_dragTarget = nullptr;
	}

	_dragSource->setTargeted(false);
	_dragSource->node()->setFlag(QGraphicsItem::ItemIsMovable, true);
	_dragSource = nullptr;

	Q_PRE(_tempConnection);
	delete _tempConnection;
	_tempConnection = nullptr;
}

PortItem* NodeView::findPortAtPosition(const QPointF& scenePos) const
{
	// Find the first port covering the point
	for (QGraphicsItem* item : _scene->items(scenePos))
		if (auto* port = dynamic_cast<PortItem*>(item))
			return port;
	return nullptr;
}

void NodeView::recalcSceneRect() const
{
	constexpr qreal margin = 120.0; // margin to allow dragging nodes past the edges

	QRectF rect;
	for (const QGraphicsItem* item : _scene->items())
	{
		QRectF r = item->sceneBoundingRect();
		rect = rect.isNull() ? r : rect.united(r);
	}
	if (rect.isNull())
		rect = QRectF(0.0, 0.0, 800.0, 520.0);

	_scene->setSceneRect(rect.adjusted(-margin, -margin, margin, margin));
}

void NodeView::checkIdType(const QVariant& id) const
{
	Q_PRE_X(id.metaType() == _idMetaType, QString("Invalid node id type (expected %1, got %2)")
	                                          .arg(_idMetaType.name(), id.metaType().name())
	                                          .toStdString()
	                                          .c_str());
}

} // namespace QNodeFlow
