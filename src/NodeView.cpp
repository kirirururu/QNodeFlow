#include "NodeView.h"

#include "ConnectionItem.h"
#include "NodeItem.h"

#include <QGraphicsScene>
#include <QPainter>

namespace QNodeFlow {

NodeView::NodeView(QWidget* parent) : QGraphicsView(parent), _scene(new QGraphicsScene(this))
{
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
	qDeleteAll(_connections);
	qDeleteAll(_nodes);
}

void NodeView::addNode(const BasicNodeId& id, NodeItem* node)
{
	if (node == nullptr)
		throw std::invalid_argument("node is nullptr");
	const NodeIdWrapper idw(id);
	if (_nodes.contains(idw))
		throw std::invalid_argument("duplicate node id");
	_nodes.insert(idw, node);
	_scene->addItem(node);
	recalcSceneRect();
}

void NodeView::addConnection(ConnectionItem* connection)
{
	if (connection == nullptr)
		return;
	_scene->addItem(connection);
	_connections.append(connection);
	recalcSceneRect();
}

void NodeView::removeNode(const BasicNodeId& id)
{
	const NodeIdWrapper idw(id);
	auto nodeIter = _nodes.find(idw);
	if (nodeIter == _nodes.end())
		return;

	removeConnectionsForNode(*nodeIter);
	_scene->removeItem(*nodeIter);
	_nodes.remove(idw);
	delete *nodeIter;
	recalcSceneRect();
}

void NodeView::removeConnection(ConnectionItem* connection)
{
	if (connection == nullptr)
		return;
	_scene->removeItem(connection);
	_connections.removeOne(connection);
	delete connection;
	recalcSceneRect();
}

void NodeView::removeConnectionsForNode(NodeItem* node)
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

void NodeView::recalcSceneRect()
{
	const qreal margin = 120.0; // margin to allow dragging nodes past the edges

	QRectF rect;
	for (QGraphicsItem* item : _scene->items())
	{
		QRectF r = item->sceneBoundingRect();
		rect = rect.isNull() ? r : rect.united(r);
	}
	if (rect.isNull())
		rect = QRectF(0.0, 0.0, 800.0, 520.0);

	_scene->setSceneRect(rect.adjusted(-margin, -margin, margin, margin));
}

} // namespace QNodeFlow
