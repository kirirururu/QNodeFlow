#include <QNodeFlow/ConnectionItem.h>
#include <QNodeFlow/NodeItem.h>
#include <QNodeFlow/NodeView.h>

#include <QApplication>
#include <QDebug>

using namespace QNodeFlow;

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	NodeView view;
	view.setWindowTitle(QStringLiteral("QNodeFlow"));

	NodeItem* node1 = new NodeItem(QStringLiteral("Node name"));
	node1->setInputs({
	    NodeItem::Port{QStringLiteral("input 1")},
	    NodeItem::Port{QStringLiteral("input 2")},
	    NodeItem::Port{QStringLiteral("input 3")},
	    NodeItem::Port{QStringLiteral("input 4")},
	});
	node1->setOutputs({
	    NodeItem::Port{QStringLiteral("output 1")},
	    NodeItem::Port{QStringLiteral("output 2")},
	});
	node1->setPos(60.0, 60.0);
	view.addNode(NodeId<QString>("node1"), node1);

	// Second test node — to check a scene with multiple nodes.
	NodeItem* node2 = new NodeItem(QStringLiteral("Node 2"));
	node2->setInputs({
	    NodeItem::Port{QStringLiteral("in")},
	});
	node2->setOutputs({
	    NodeItem::Port{QStringLiteral("out")},
	});
	node2->setPos(460.0, 260.0);
	view.addNode(NodeId<QString>("node2"), node2);

	// Connection: output 0 of the first node -> input 0 of the second node.
	ConnectionItem* connection =
	    new ConnectionItem(ConnectionItem::PortRef{node1, false, 0}, // output of node1
	                       ConnectionItem::PortRef{node2, true, 0}); // input of node2
	view.addConnection(connection);

	view.showMaximized();

	return QApplication::exec();
}
