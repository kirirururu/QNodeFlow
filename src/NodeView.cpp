#include "NodeView.h"

#include "ConnectionItem.h"
#include "NodeItem.h"
#include "PortItem.h"
#include "TemporaryConnectionItem.h"

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

namespace {

constexpr double SCENE_MARGIN = 120;
constexpr double MIN_SCALE_FACTOR = 0.5;
constexpr double MAX_SCALE_FACTOR = 3.0;

} // namespace

namespace QNodeFlow {

NodeView::NodeView(QMetaType keyMetaType, QWidget* parent)
    : QGraphicsView(parent), _idMetaType(keyMetaType), _scene(new QGraphicsScene(this))
{
	Q_ASSERT(_idMetaType.id() != QMetaType::UnknownType);
	setScene(_scene);
	setRenderHint(QPainter::Antialiasing, true);
	setDragMode(NoDrag); // the node itself is dragged, not the scene
	setTransformationAnchor(AnchorUnderMouse);
	setBackgroundBrush(QColor(42, 44, 50));
	extendSceneIfNeeded();
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
	connect(node, &NodeItem::positionChanged, this, &NodeView::extendSceneIfNeeded);
	extendSceneIfNeeded();
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
	extendSceneIfNeeded();
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
}

void NodeView::resizeSceneToContent()
{
	const QRectF target = _scene->itemsBoundingRect().adjusted(-SCENE_MARGIN, -SCENE_MARGIN,
	                                                           SCENE_MARGIN, SCENE_MARGIN);
	resizeScene(target);
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
		else if (mouseEvent->button() == Qt::RightButton)
		{
			_panning = true;
			_panLastPos = mouseEvent->pos();
		}
		break;
	}
	case QEvent::MouseMove:
	{
		const auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (_panning)
		{
			const QPoint delta = mouseEvent->pos() - _panLastPos;
			_panLastPos = mouseEvent->pos();
			horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
			verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
		}
		if (_dragSource)
			updateConnectionDrag(mapToScene(mouseEvent->pos()));
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		const auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton && _dragSource)
			finishConnectionDrag();
		else if (mouseEvent->button() == Qt::RightButton)
			_panning = false;
		break;
	}
	default:
		break;
	}
	return QGraphicsView::viewportEvent(event);
}

void NodeView::wheelEvent(QWheelEvent* event)
{
	const auto delta = event->angleDelta().y();
	if (delta == 0)
	{
		QGraphicsView::wheelEvent(event);
		return;
	}

	constexpr qreal zoom = 1.15;
	const qreal current = transform().m11();
	const qreal clamped =
	    qBound(MIN_SCALE_FACTOR, current * (delta > 0 ? zoom : 1.0 / zoom), MAX_SCALE_FACTOR);
	const qreal factor = clamped / current;

	// Keep the scene point under the cursor fixed after scaling.
	const QPoint cursor = event->position().toPoint();
	const auto sceneBefore = mapToScene(cursor);
	scale(factor, factor);
	const auto sceneAfter = mapToScene(cursor);
	horizontalScrollBar()->setValue(horizontalScrollBar()->value() +
	                                (sceneBefore.x() - sceneAfter.x()) * transform().m11());
	verticalScrollBar()->setValue(verticalScrollBar()->value() +
	                              (sceneBefore.y() - sceneAfter.y()) * transform().m22());

	event->accept();
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

void NodeView::extendSceneIfNeeded()
{
	const QRectF target = _scene->itemsBoundingRect().adjusted(-SCENE_MARGIN, -SCENE_MARGIN,
	                                                           SCENE_MARGIN, SCENE_MARGIN);
	const QRectF current = _scene->sceneRect();
	if (!current.contains(target))
		resizeScene(current.united(target));
}

void NodeView::resizeScene(const QRectF& rect)
{
	// setSceneRect()/centerOn() below can re-trigger itemChange -> positionChanged
	// on the dragged node; skip the re-entrant call to avoid infinite recursion.
	if (_updatingSceneRect)
		return;
	_updatingSceneRect = true;

	const QPointF center = mapToScene(viewport()->rect().center());
	_scene->setSceneRect(rect);
	centerOn(center);

	_updatingSceneRect = false;
}

void NodeView::checkIdType(const QVariant& id) const
{
	Q_PRE_X(id.metaType() == _idMetaType, QString("Invalid node id type (expected %1, got %2)")
	                                          .arg(_idMetaType.name(), id.metaType().name())
	                                          .toStdString()
	                                          .c_str());
}

} // namespace QNodeFlow
