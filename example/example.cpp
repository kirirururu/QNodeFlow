#include <QNodeFlow/NodeItem.h>
#include <QNodeFlow/NodeView.h>

#include <QApplication>
#include <QMainWindow>

using namespace QNodeFlow;

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QMainWindow window;
	window.setWindowTitle("QNodeFlow Example");

	NodeView* view = NodeView::create<QString>();

	NodeItem* node1 = new NodeItem("node1", "Node 1");
	node1->addInputPort("input 1");
	node1->addInputPort("input 2");
	node1->addInputPort("input 3");
	node1->addInputPort("input 4");

	node1->addOutputPort("output 1");
	node1->addOutputPort("output 2");
	node1->addOutputPort("output 3");
	node1->addOutputPort("output 4");
	node1->setPos(60.0, 160.0);
	view->addNode("node1", node1);

	NodeItem* node2 = new NodeItem("node2", "Node 2");
	node2->addInputPort("input 1");
	node2->addInputPort("input 2");
	node2->addOutputPort("output 1");
	node2->addOutputPort("output 2");
	node2->setPos(600.0, 40.0);
	view->addNode("node2", node2);

	NodeItem* node3 = new NodeItem("node3", "Node 3");
	node3->addInputPort("input");
	node3->addOutputPort("output");
	node3->setPos(700.0, 400.0);
	view->addNode("node3", node3);

	view->addConnection(node1, 0, node2, 0);
	view->addConnection(node1, 1, node2, 1);

	view->addConnection(node2, 0, node3, 0);
	view->removeConnection(node2, 0, node3, 0);

	window.setCentralWidget(view);
	window.showMaximized();

	return QApplication::exec();
}
