#pragma once

#include "BasicNodeItem.h"

#include <QGraphicsView>
#include <QList>
#include <QPoint>

#include <map>

class QGraphicsScene;

namespace QNodeFlow {

class NodeWithInputs;
class NodeWithOutputs;
class NodeItem;
class GlobalInputNodeItem;
class GlobalOutputNodeItem;
class ConnectionItem;
class TemporaryConnectionItem;

class PortItem;

namespace detail {

struct NodeIdComparator
{
	bool operator()(const QVariant& lhs, const QVariant& rhs) const
	{
		return QVariant::compare(lhs, rhs) == QPartialOrdering::Less;
	}
};

} // namespace detail

/**
 * QGraphicsView displaying a graph of nodes (NodeItem) and connections (ConnectionItem).
 *
 * NodeView owns the scene: nodes and connections are added via addNode()/addConnection()
 * and removed via removeNode()/removeConnection(). NodeView owns the added items and
 * frees them on removal or in the destructor. Removing a node automatically removes the
 * connections that reference it. The scene rectangle is computed automatically from the
 * content, with a margin for dragging.
 */
class NodeView : public QGraphicsView
{
	Q_OBJECT

public:
	explicit NodeView(QMetaType keyMetaType, QWidget* parent = nullptr);
	~NodeView() override;

	NodeItem* addNode(QVariant id, QString title);
	NodeItem* findNode(const QVariant& id) const;
	void removeNode(const QVariant& id);

	GlobalInputNodeItem* addGlobalInputNode(QVariant id, QString title);
	GlobalInputNodeItem* getGlobalInputNode() const;
	void removeGlobalInputNode();

	GlobalOutputNodeItem* addGlobalOutputNode(QVariant id, QString title);
	GlobalOutputNodeItem* getGlobalOutputNode() const;
	void removeGlobalOutputNode();

	void addConnection(const QVariant& sourceId,
	                   int sourcePort,
	                   const QVariant& destinationId,
	                   int destinationPort);
	void addConnection(NodeWithOutputs* source,
	                   int sourcePort,
	                   NodeWithInputs* destination,
	                   int destinationPort);

	void removeConnection(const QVariant& sourceId,
	                      int sourcePort,
	                      const QVariant& destinationId,
	                      int destinationPort);
	void removeConnection(NodeItem* source, int sourcePort, NodeItem* destination, int destinationPort);

	void resizeSceneToContent();

	template <typename IdType>
	static NodeView* create(QWidget* parent = nullptr)
	{
		return new NodeView(QMetaType::fromType<IdType>(), parent);
	}

signals:
	void connectionAdded(const QVariant& sourceId,
	                     int sourcePort,
	                     const QVariant& destinationId,
	                     int destinationPort);
	void connectionRemoved(const QVariant& sourceId,
	                       int sourcePort,
	                       const QVariant& destinationId,
	                       int destinationPort);

protected:
	// Tracks connection drawing: press on an output port, move, release on an input port.
	bool viewportEvent(QEvent* event) override;

	// Zooms the scene with the mouse wheel, keeping the point under the cursor fixed.
	void wheelEvent(QWheelEvent* event) override;

	// Keeps the global nodes pinned to the viewport on resize.
	void resizeEvent(QResizeEvent* event) override;

private:
	void removeConnectionsForNode(const BasicNodeItem* node);
	QRectF contentBoundingRect() const;
	void extendSceneIfNeeded();
	void resizeScene(const QRectF& rect);
	void checkIdType(const QVariant& id) const;

	// Connection-drawing state (drag from an output port to an input port).
	void startConnectionDrag(const QPointF& scenePos);
	void updateConnectionDrag(const QPointF& scenePos);
	void finishConnectionDrag();
	PortItem* findPortAtPosition(const QPointF& scenePos) const;

	QMetaType _idMetaType;
	QGraphicsScene* _scene;
	std::map<QVariant, NodeItem*, detail::NodeIdComparator> _nodes;
	GlobalInputNodeItem* _globalInputNode = nullptr;
	GlobalOutputNodeItem* _globalOutputNode = nullptr;
	QList<ConnectionItem*> _connections;

	// Active connection being drawn, if any.
	TemporaryConnectionItem* _tempConnection = nullptr;
	PortItem* _dragSource = nullptr;
	PortItem* _dragTarget = nullptr;

	// Right-button view panning state.
	bool _panning = false;
	QPoint _panLastPos;

	// Guards updateSceneRect against re-entrancy: setSceneRect()/centerOn() can
	// re-trigger itemChange -> positionChanged while a node is being dragged.
	bool _updatingSceneRect = false;
};

} // namespace QNodeFlow
