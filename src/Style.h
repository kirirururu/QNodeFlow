#pragma once

#include <QColor>

namespace QNodeFlow {

// Node body
constexpr double BODY_WIDTH = 250.0;    // body width
constexpr double BODY_BORDER_WIDTH = 3; // body border thickness
constexpr double BODY_RADIUS = 10;      // body corner radius
constexpr double HEADER_HEIGHT = 32.0;  // header height
constexpr double ROW_HEIGHT = 36.0;     // port row height
constexpr double BOTTOM_PADDING = 20.0; // bottom padding
constexpr double MIN_BODY_ROWS = 2;     // minimum rows so the node is not a thin strip

// Ports
constexpr double PORT_RADIUS = 8.0;        // port radius
constexpr double PORT_LINE_WIDTH = 2.0;    // circle outline thickness
constexpr double PORTS_TOP_PADDING = 10.0; // gap between the header and the top ports
constexpr double PORT_LABEL_GAP = 12.0;    // gap between a port and its label
constexpr double PORT_HIT_RADIUS = 3.0;    // extra radius for hover hit-testing

namespace color {
constexpr QColor BACKGROUND(23, 25, 30);          // body background
constexpr QColor BORDER(60, 64, 72);              // body border
constexpr QColor BORDER_HOVER(120, 170, 210);     // body border on hover
constexpr QColor HEADER(16, 18, 22);              // header background
constexpr QColor NODE_TITLE(235, 238, 242);       // title color
constexpr QColor PORT_FILL(46, 125, 110);         // port fill
constexpr QColor PORT_BORDER(58, 190, 150);       // port outline
constexpr QColor PORT_FILL_HOVER(66, 165, 145);   // port fill on hover
constexpr QColor PORT_BORDER_HOVER(96, 230, 190); // port outline on hover
constexpr QColor PORT_LABEL(190, 194, 200);       // port label color
} // namespace color

} // namespace QNodeFlow
