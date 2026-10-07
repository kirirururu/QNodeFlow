#include "ConnectionItem.h"

#include "NodeItem.h"
#include "PortItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace {

constexpr QColor CONNECTION_COLOR(120, 160, 200); // connection line color
constexpr double LINE_WIDTH = 2.0;                // line thickness
constexpr double CORNER_RADIUS = 8.0;             // corner rounding radius
constexpr double PORT_STUB = 30.0;                // fixed length of the segment near a port
constexpr double MIN_FORWARD_GAP = 2 * PORT_STUB; // min x2 - x1 for the forward variant
constexpr double PARALLEL_LINES_GAP = 20;         // gap between two parallel connections

// Returns the unit vector in direction d ((0, 0) for a zero d).
QPointF unitVector(const QPointF& d)
{
	const double len = qHypot(d.x(), d.y());
	if (len <= 0.0)
		return QPointF(0.0, 0.0);
	return QPointF(d.x() / len, d.y() / len);
}

// Builds a QPainterPath from a set of points: straight segments + rounded corners (quadTo).
QPainterPath roundedPolyline(const QVector<QPointF>& points, double radius)
{
	QPainterPath path;
	if (points.size() < 2)
		return path;

	path.moveTo(points.first());
	for (int i = 1; i < points.size() - 1; ++i)
	{
		const QPointF a = points[i - 1];
		const QPointF v = points[i];
		const QPointF b = points[i + 1];

		const QPointF dirIn = unitVector(v - a);
		const QPointF dirOut = unitVector(b - v);

		// Clamp the radius so the corners do not overlap on short segments.
		const double maxR = qMin((v - a).manhattanLength(), (b - v).manhattanLength()) / 2.0;
		const double r = qMin(radius, qMax(0.0, maxR));

		const QPointF p1 = v - dirIn * r;
		const QPointF p2 = v + dirOut * r;

		path.lineTo(p1);
		path.quadTo(v, p2);
	}
	path.lineTo(points.last());
	return path;
}

} // namespace

namespace QNodeFlow {

ConnectionItem::ConnectionItem(PortItem* from, PortItem* to) : _from(from), _to(to)
{
	setZValue(-1.0); // draw under the nodes so the line ends are covered by the ports

	connect(_from->node(), &NodeItem::positionChanged, this, &ConnectionItem::refresh);
	connect(_to->node(), &NodeItem::positionChanged, this, &ConnectionItem::refresh);

	refresh();
}

void ConnectionItem::refresh()
{
	prepareGeometryChange();
	update();
}

QPainterPath ConnectionItem::buildPath(const PortItem* from, const QPointF& to)
{
	const auto a = from->scenePos();
	const auto b = to;

	// Add a horizontal shift to avoid overlapping
	double xShift = from->index() * PARALLEL_LINES_GAP;

	QVector<QPointF> pts;
	if (b.x() - a.x() > MIN_FORWARD_GAP + xShift * 2)
	{
		// Forward case: the receiver is far enough to the right of the source. The
		// middle (vertical) segment sits at the center between the nodes, the side
		// segments span half the distance (they change with distance). All corners 90°.
		if (b.y() > a.y())
			xShift *= -1;
		const double midX = (a.x() + b.x()) / 2.0 + xShift;
		pts = {a, QPointF(midX, a.y()), QPointF(midX, b.y()), b};
	}
	else
	{
		// Reverse case: the source is to the right of the receiver. The middle
		// horizontal segment runs at the vertical midpoint between the ports so the
		// line passes between the nodes and stays visible.
		// An output port (right) leads the line to the right, an input (left) to the left.
		// The segments near the ports point outward and have a fixed length.
		const auto p1 = QPointF(a.x() + PORT_STUB + xShift, a.y());
		const auto p2 = QPointF(b.x() - PORT_STUB - xShift, b.y());

		const double midY = (a.y() + b.y()) / 2.0;
		pts = {a, p1, QPointF(p1.x(), midY), QPointF(p2.x(), midY), p2, b};
	}
	return roundedPolyline(pts, CORNER_RADIUS);
}

QRectF ConnectionItem::boundingRect() const
{
	constexpr double margin = LINE_WIDTH + 2.0;
	return buildPath(_from, _to->scenePos()).boundingRect().adjusted(-margin, -margin, margin, margin);
}

void ConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->setRenderHint(QPainter::Antialiasing, true);
	QPen pen(CONNECTION_COLOR, LINE_WIDTH);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setCapStyle(Qt::RoundCap);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(buildPath(_from, _to->scenePos()));
}

bool operator==(const ConnectionItem& lhs, const ConnectionItem& rhs)
{
	return lhs._from == rhs._from && lhs._to == rhs._to;
}

bool operator!=(const ConnectionItem& lhs, const ConnectionItem& rhs)
{
	return !(lhs == rhs);
}

} // namespace QNodeFlow
