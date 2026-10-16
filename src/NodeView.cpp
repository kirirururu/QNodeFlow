#include "NodeView.h"

#include "ConnectionItem.h"
#include "GlobalNodeItem.h"
#include "NodeItem.h"
#include "PortItem.h"
#include "SelectionItem.h"
#include "TemporaryConnectionItem.h"

#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

namespace {

constexpr auto SCENE_BACKGROUND = QColor(42, 44, 50);
constexpr auto VIEW_BACKGROUND = Qt::black;

constexpr double MIN_SCALE_FACTOR = 0.5;
constexpr double MAX_SCALE_FACTOR = 1.0;

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
	// The view must keep keyboard focus for DEL to reach keyPressEvent.
	setFocusPolicy(Qt::StrongFocus);
	setCanvasSize(500, 500);

	_selection = new SelectionItem();
	_selection->setVisible(false);
	_scene->addItem(_selection);
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

NodeItem* NodeView::addNode(QVariant id, QString title)
{
	checkIdType(id);
	if (_nodes.find(id) != _nodes.end())
		throw std::invalid_argument("duplicate node id");
	auto* node = new NodeItem(std::move(id), std::move(title));
	_nodes.insert({node->id(), node});
	_scene->addItem(node);
	emit nodeAdded(node->id());
	return node;
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
	emit nodeRemoved(node->id());
	node->deleteLater();
	_nodes.erase(nodeIter);
}

GlobalInputNodeItem* NodeView::addGlobalInputNode(QVariant id, QString title)
{
	checkIdType(id);
	if (_globalInputNode)
		throw std::runtime_error("global input node already added");
	_globalInputNode = new GlobalInputNodeItem(std::move(id), std::move(title), this);
	connect(horizontalScrollBar(), &QScrollBar::valueChanged, _globalInputNode,
	        &GlobalInputNodeItem::updatePosition);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, _globalInputNode,
	        &GlobalInputNodeItem::updatePosition);
	_scene->addItem(_globalInputNode);
	resizeScene();
	return _globalInputNode;
}

GlobalInputNodeItem* NodeView::getGlobalInputNode() const
{
	return _globalInputNode;
}

void NodeView::removeGlobalInputNode()
{
	if (!_globalInputNode)
		throw std::runtime_error("global input node was not added");
	removeConnectionsForNode(_globalInputNode);
	connect(_globalInputNode, &GlobalInputNodeItem::destroyed, this, &NodeView::resizeScene);
	_globalInputNode->deleteLater();
	_globalInputNode = nullptr;
}

GlobalOutputNodeItem* NodeView::addGlobalOutputNode(QVariant id, QString title)
{
	checkIdType(id);
	if (_globalOutputNode)
		throw std::runtime_error("global output node already added");
	_globalOutputNode = new GlobalOutputNodeItem(std::move(id), std::move(title), this);
	connect(horizontalScrollBar(), &QScrollBar::valueChanged, _globalOutputNode,
	        &GlobalOutputNodeItem::updatePosition);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, _globalOutputNode,
	        &GlobalOutputNodeItem::updatePosition);
	_scene->addItem(_globalOutputNode);
	resizeScene();
	return _globalOutputNode;
}

GlobalOutputNodeItem* NodeView::getGlobalOutputNode() const
{
	return _globalOutputNode;
}

void NodeView::removeGlobalOutputNode()
{
	if (!_globalOutputNode)
		throw std::runtime_error("global input node was not added");
	removeConnectionsForNode(_globalOutputNode);
	connect(_globalInputNode, &GlobalInputNodeItem::destroyed, this, &NodeView::resizeScene);
	_globalOutputNode->deleteLater();
	_globalOutputNode = nullptr;
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

void NodeView::addConnection(NodeWithOutputs* source,
                             int sourcePort,
                             NodeWithInputs* destination,
                             int destinationPort)
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
		removeConnectionItem(*iter);
}

QSizeF NodeView::canvasSize() const
{
	return _canvasSize;
}

void NodeView::setCanvasSize(quint32 width, quint32 height)
{
	_canvasSize = QSizeF(width, height);
	resizeScene();
}

void NodeView::removeConnectionsForNode(const BasicNodeItem* node)
{
	auto it = _connections.begin();
	while (it != _connections.end())
	{
		ConnectionItem* connection = *it;
		if (connection->from()->node() == node || connection->to()->node() == node)
		{
			it = _connections.erase(it);
			emit connectionRemoved(connection->from()->node()->id(), connection->from()->index(),
			                       connection->to()->node()->id(), connection->to()->index());
			connection->deleteLater();
		}
		else
			++it;
	}
}

void NodeView::removeConnectionItem(ConnectionItem* connection)
{
	_connections.removeOne(connection);
	emit connectionRemoved(connection->from()->node()->id(), connection->from()->index(),
	                       connection->to()->node()->id(), connection->to()->index());
	connection->deleteLater();
}

void NodeView::removeSelectedItems()
{
	// Copy first: removing a node drops its connections as a side effect, which
	// would invalidate the iteration.
	QList<ConnectionItem*> selectedConnections;
	for (ConnectionItem* connection : _connections)
		if (connection->isSelected())
			selectedConnections.append(connection);

	QList<QVariant> selectedNodeIds;
	for (const auto& [id, node] : _nodes)
		if (node->isSelected())
			selectedNodeIds.append(id);

	for (ConnectionItem* connection : selectedConnections)
		removeConnectionItem(connection);

	for (const QVariant& id : selectedNodeIds)
		removeNode(id);
}

void NodeView::drawBackground(QPainter* painter, const QRectF& rect)
{
	painter->fillRect(rect, VIEW_BACKGROUND);
	painter->fillRect(scene()->sceneRect() & rect, SCENE_BACKGROUND);
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
		{
			startConnectionDrag(mapToScene(mouseEvent->pos()));
			// The press missed an output port: on the empty background it starts the
			// rubber band; clicks on items are left to the scene (selection, dragging).
			if (!_dragSource && _scene->items(mapToScene(mouseEvent->pos())).isEmpty())
				startRectSelection(mapToScene(mouseEvent->pos()),
				                   mouseEvent->modifiers() & Qt::ControlModifier);
		}
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
		if (_rectSelecting)
			updateRectSelection(mapToScene(mouseEvent->pos()));
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
		if (mouseEvent->button() == Qt::LeftButton)
		{
			if (_rectSelecting)
				finishRectSelection();
			else if (_dragSource)
				finishConnectionDrag();
		}
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

void NodeView::keyPressEvent(QKeyEvent* event)
{
	// Deleting while a connection is being drawn would leave _dragSource dangling.
	if (event->key() == Qt::Key_Delete && !_tempConnection)
	{
		removeSelectedItems();
		return;
	}
	QGraphicsView::keyPressEvent(event);
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
	// Same for selectability: a selectable node under the press would swallow the
	// selection of the other nodes.
	if (dynamic_cast<NodeItem*>(_dragSource->node()))
		_dragSource->node()->setFlag(QGraphicsItem::ItemIsSelectable, false);
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
		auto* source = dynamic_cast<NodeWithOutputs*>(_dragSource->node());
		auto* destination = dynamic_cast<NodeWithInputs*>(_dragTarget->node());
		Q_ASSERT(source && destination);
		addConnection(source, _dragSource->index(), destination, _dragTarget->index());
		emit connectionAdded(source->id(), _dragSource->index(), destination->id(),
		                     _dragTarget->index());
		_dragTarget = nullptr;
	}

	_dragSource->setTargeted(false);
	if (dynamic_cast<NodeItem*>(_dragSource->node()))
	{
		_dragSource->node()->setFlag(QGraphicsItem::ItemIsMovable, true);
		_dragSource->node()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	}
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

void NodeView::startRectSelection(const QPointF& scenePos, bool additive)
{
	Q_PRE(_selection);
	_rectSelecting = true;
	_rectSelectionAdditive = additive;
	_rectSelectionOrigin = scenePos;
	_selection->setRect(QRectF(_rectSelectionOrigin, _rectSelectionOrigin));
	_selection->setVisible(true);
}

void NodeView::updateRectSelection(const QPointF& scenePos)
{
	Q_PRE(_rectSelecting && _selection);
	_selection->setRect(QRectF(_rectSelectionOrigin, scenePos).normalized());
}

void NodeView::finishRectSelection()
{
	Q_PRE(_rectSelecting && _selection);
	const QRectF rect = _selection->rect();
	_rectSelecting = false;
	_selection->setVisible(false);

	// Without Ctrl the previous selection is dropped; a plain click (zero-size rectangle)
	// therefore just clears it.
	if (!_rectSelectionAdditive)
		_scene->clearSelection();
	if (rect.width() > 0 || rect.height() > 0)
	{
		for (const auto& [id, node] : _nodes)
			if (node->sceneBoundingRect().intersects(rect))
				node->setSelected(true);
	}
}

void NodeView::resizeScene()
{
	// setSceneRect()/centerOn() below can re-trigger itemChange -> positionChanged
	// on the dragged node; skip the re-entrant call to avoid infinite recursion.
	if (_updatingSceneRect)
		return;
	_updatingSceneRect = true;

	const QPointF center = mapToScene(viewport()->rect().center());
	QRectF rect(QPointF(0, 0), _canvasSize);
	if (_globalInputNode)
		rect.adjust(-_globalInputNode->boundingRect().width(), 0, 0, 0);
	if (_globalOutputNode)
		rect.adjust(0, 0, _globalOutputNode->boundingRect().width(), 0);
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
