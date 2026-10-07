#pragma once

#include <QGraphicsView>
#include <QList>
#include <QPointF>

#include <map>

class QGraphicsScene;

namespace QNodeFlow {

class NodeItem;
class ConnectionItem;
class PortItem;
class TemporaryConnectionItem;

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

	void addNode(const QVariant& id, NodeItem* node);
	NodeItem* findNode(const QVariant& id) const;
	void removeNode(const QVariant& id);

	void addConnection(const QVariant& sourceId,
	                   int sourcePort,
	                   const QVariant& destinationId,
	                   int destinationPort);
	void addConnection(NodeItem* source, int sourcePort, NodeItem* destination, int destinationPort);

	void removeConnection(const QVariant& sourceId,
	                      int sourcePort,
	                      const QVariant& destinationId,
	                      int destinationPort);
	void removeConnection(NodeItem* source, int sourcePort, NodeItem* destination, int destinationPort);

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

private:
	void removeConnectionsForNode(const NodeItem* node);
	void recalcSceneRect() const;
	void checkIdType(const QVariant& id) const;

	// Connection-drawing state (drag from an output port to an input port).
	void startConnectionDrag(const QPointF& scenePos);
	void updateConnectionDrag(const QPointF& scenePos);
	void finishConnectionDrag();
	PortItem* findPortAtPosition(const QPointF& scenePos) const;

	QMetaType _idMetaType;
	QGraphicsScene* _scene;
	std::map<QVariant, NodeItem*, detail::NodeIdComparator> _nodes;
	QList<ConnectionItem*> _connections;

	// Active connection being drawn, if any.
	TemporaryConnectionItem* _tempConnection = nullptr;
	PortItem* _dragSource = nullptr;
	PortItem* _dragTarget = nullptr;
};

} // namespace QNodeFlow
