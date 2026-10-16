#pragma once

#include "BasicNodeItem.h"

#include <QGraphicsView>
#include <QList>
#include <QPoint>

#include <map>

class QGraphicsScene;
class QKeyEvent;

namespace QNodeFlow {

class NodeWithInputs;
class NodeWithOutputs;
class NodeItem;
class GlobalInputNodeItem;
class GlobalOutputNodeItem;
class ConnectionItem;
class TemporaryConnectionItem;
class SelectionItem;

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
 * connections that reference it.
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

	QSizeF canvasSize() const;
	void setCanvasSize(quint32 width, quint32 height);

	template <typename IdType>
	static NodeView* create(QWidget* parent = nullptr)
	{
		return new NodeView(QMetaType::fromType<IdType>(), parent);
	}

signals:
	void nodeAdded(const QVariant& id);
	void nodeRemoved(const QVariant& id);

	void connectionAdded(const QVariant& sourceId,
	                     int sourcePort,
	                     const QVariant& destinationId,
	                     int destinationPort);
	void connectionRemoved(const QVariant& sourceId,
	                       int sourcePort,
	                       const QVariant& destinationId,
	                       int destinationPort);

protected:
	void drawBackground(QPainter* painter, const QRectF& rect) override;

	// Tracks connection drawing: press on an output port, move, release on an input port.
	bool viewportEvent(QEvent* event) override;

	// Zooms the scene with the mouse wheel, keeping the point under the cursor fixed.
	void wheelEvent(QWheelEvent* event) override;

	// Deletes the selected nodes and connections (DEL).
	void keyPressEvent(QKeyEvent* event) override;

private:
	void removeConnectionsForNode(const BasicNodeItem* node);
	void removeConnectionItem(ConnectionItem* connection);
	void removeSelectedItems();
	void resizeScene();
	void checkIdType(const QVariant& id) const;

	void startConnectionDrag(const QPointF& scenePos);
	void updateConnectionDrag(const QPointF& scenePos);
	void finishConnectionDrag();
	PortItem* findPortAtPosition(const QPointF& scenePos) const;

	void startRectSelection(const QPointF& scenePos, bool additive);
	void updateRectSelection(const QPointF& scenePos);
	void finishRectSelection();

	QMetaType _idMetaType;
	QGraphicsScene* _scene;
	std::map<QVariant, NodeItem*, detail::NodeIdComparator> _nodes;
	GlobalInputNodeItem* _globalInputNode = nullptr;
	GlobalOutputNodeItem* _globalOutputNode = nullptr;
	QList<ConnectionItem*> _connections;

	// Active connection being drawn, if any
	TemporaryConnectionItem* _tempConnection = nullptr;
	PortItem* _dragSource = nullptr;
	PortItem* _dragTarget = nullptr;

	// Right-button view panning state
	bool _panning = false;
	QPoint _panLastPos;

	// Rectangular selection
	SelectionItem* _selection = nullptr;
	bool _rectSelecting = false;
	bool _rectSelectionAdditive = false;
	QPointF _rectSelectionOrigin;

	QSizeF _canvasSize;
	bool _updatingSceneRect = false;
};

} // namespace QNodeFlow
