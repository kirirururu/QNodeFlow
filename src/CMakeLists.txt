qt_add_library(QNodeFlow)

set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PRIVATE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/QNodeFlow)

target_sources(QNodeFlow PRIVATE
	BasicNodeItem.cpp
	ConnectionItem.cpp
	GlobalNodeItem.cpp
	NodeItem.cpp
	NodeView.cpp
	PortItem.cpp
	TemporaryConnectionItem.cpp
)
target_sources(QNodeFlow PRIVATE
	${PRIVATE_INCLUDE_DIR}/BasicNodeItem.h
	${PRIVATE_INCLUDE_DIR}/ConnectionItem.h
	${PRIVATE_INCLUDE_DIR}/GlobalNodeItem.h
	${PRIVATE_INCLUDE_DIR}/NodeItem.h
	${PRIVATE_INCLUDE_DIR}/NodeView.h
	${PRIVATE_INCLUDE_DIR}/PortItem.h
	${PRIVATE_INCLUDE_DIR}/TemporaryConnectionItem.h
)

target_include_directories(QNodeFlow INTERFACE ${PUBLIC_INCLUDE_DIR} PRIVATE ${PRIVATE_INCLUDE_DIR})

target_link_libraries(QNodeFlow
	Qt::Core
	Qt::Gui
	Qt::Widgets
)

if (UNIX)
	target_compile_options(QNodeFlow PRIVATE -Wall -Wextra -Werror)
endif ()
