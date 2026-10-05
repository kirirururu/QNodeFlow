#include "NodeView.h"

#include "ConnectionItem.h"
#include "NodeItem.h"

#include <QGraphicsScene>
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

	// TODO: check that port indexes are valid

	const auto connection = new ConnectionItem(
	    ConnectionItem::PortRef{.node = source, .isInput = false, .index = sourcePort},
	    ConnectionItem::PortRef{.node = destination, .isInput = true, .index = destinationPort});
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
	const auto connection = new ConnectionItem(
	    ConnectionItem::PortRef{.node = source, .isInput = false, .index = sourcePort},
	    ConnectionItem::PortRef{.node = destination, .isInput = true, .index = destinationPort});
	_scene->removeItem(connection);
	_connections.removeOne(connection);
	delete connection;
	recalcSceneRect();
}

void NodeView::removeConnectionsForNode(const NodeItem* node)
{
	for (auto it = _connections.begin(); it != _connections.end();)
	{
		ConnectionItem* connection = *it;
		if (connection->from().node == node || connection->to().node == node)
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
