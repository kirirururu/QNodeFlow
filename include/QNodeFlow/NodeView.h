#pragma once

#include "NodeId.h"

#include <QGraphicsView>
#include <QList>

class QGraphicsScene;

namespace QNodeFlow {

class NodeItem;
class ConnectionItem;

class NodeIdWrapper
{
public:
	explicit NodeIdWrapper(const BasicNodeId& id) : id_(id.clone()) { }

	NodeIdWrapper(const NodeIdWrapper& other) : id_(other.id_->clone()) { }
	NodeIdWrapper& operator=(const NodeIdWrapper& other)
	{
		if (this != &other)
			id_ = other.id_->clone();
		return *this;
	}

	NodeIdWrapper(NodeIdWrapper&& other) noexcept = default;
	NodeIdWrapper& operator=(NodeIdWrapper&& other) noexcept = default;

	bool operator<(const NodeIdWrapper& other) const { return id_->lessThan(*other.id_); }

private:
	BasicNodeId::Ptr id_;
};

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
	explicit NodeView(QWidget* parent = nullptr);
	~NodeView() override;

	void addNode(const BasicNodeId& id, NodeItem* node);
	void addConnection(ConnectionItem* connection);
	void removeNode(const BasicNodeId& id);
	void removeConnection(ConnectionItem* connection);

private:
	void removeConnectionsForNode(NodeItem* node);
	void recalcSceneRect();

	QGraphicsScene* _scene;
	QMap<NodeIdWrapper, NodeItem*> _nodes;
	QList<ConnectionItem*> _connections;
};

} // namespace QNodeFlow
