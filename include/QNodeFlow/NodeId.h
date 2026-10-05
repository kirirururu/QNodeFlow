#pragma once

#include <memory>
#include <stdexcept>

namespace QNodeFlow {

/**
 * An abstract class used in NodeView API to uniquely identify nodes.
 * End users should use NodeId<T> instead (see below)!
 */
class BasicNodeId
{
public:
	using Ptr = std::unique_ptr<BasicNodeId>;
	virtual ~BasicNodeId() = default;
	virtual bool lessThan(const BasicNodeId& other) const = 0;
	virtual Ptr clone() const = 0;
};

/**
 * Template adapter class containing a single value of some type T.
 * Create an instance of NodeId from your "real" node identifier and use it in NodeView API.
 * @tparam T a real value type (int, QString, etc.) with comparison ("less") operator defined
 */
template <typename T>
class NodeId : public BasicNodeId
{
public:
	explicit NodeId(const T& value) : _value(value) { }

	bool lessThan(const BasicNodeId& other) const override
	{
		if (const auto* simpleOther = cast(other))
			return _value < simpleOther->_value;
		throw std::logic_error("Id types are different");
	}

	Ptr clone() const override { return std::make_unique<NodeId>(_value); }

	operator T&() { return _value; }
	operator const T&() const { return _value; }

private:
	static const NodeId* cast(const BasicNodeId& id) { return dynamic_cast<const NodeId*>(&id); }

	T _value;
};

} // namespace QNodeFlow
