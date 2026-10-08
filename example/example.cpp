#include <QNodeFlow/GlobalNodeItem.h>
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

	auto* node1 = view->addNode("node1", "Node 1");
	node1->addInputPort("input 1");
	node1->addInputPort("input 2");
	node1->addInputPort("input 3");
	node1->addInputPort("input 4");

	node1->addOutputPort("output 1");
	node1->addOutputPort("output 2");
	node1->addOutputPort("output 3");
	node1->addOutputPort("output 4");
	node1->setPos(60.0, 160.0);

	auto* node2 = view->addNode("node2", "Node 2");
	node2->addInputPort("input 1");
	node2->addInputPort("input 2");
	node2->addOutputPort("output 1");
	node2->addOutputPort("output 2");
	node2->setPos(600.0, 40.0);

	auto* node3 = view->addNode("node3", "Node 3");
	node3->addInputPort("input");
	node3->addOutputPort("output");
	node3->setPos(700.0, 400.0);

	auto* globalInput = view->addGlobalInputNode("input", "Inputs");
	globalInput->addOutputPort("source 1");
	globalInput->addOutputPort("source 2");

	auto* globalOutput = view->addGlobalOutputNode("output", "Outputs");
	globalOutput->addInputPort("sink 1");
	globalOutput->addInputPort("sink 2");

	using namespace std::placeholders;
	QObject::connect(view, &NodeView::connectionAdded,
	                 std::bind(&printConnectionChange, true, _1, _2, _3, _4));
	QObject::connect(view, &NodeView::connectionRemoved,
	                 std::bind(&printConnectionChange, false, _1, _2, _3, _4));

	view->addConnection(node1, 0, node2, 0);
	view->addConnection(node1, 1, node2, 1);

	view->addConnection(node2, 0, node3, 0);
	view->removeConnection(node2, 0, node3, 0);

	window.setCentralWidget(view);
	window.showMaximized();

	return QApplication::exec();
}
