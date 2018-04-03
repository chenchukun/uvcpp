#ifndef LIBUVCPP_ANY_H
#define LIBUVCPP_ANY_H
#include <typeinfo>
#include <cstddef>
#include "utility.h"
#include <algorithm>

NAMESPACE_START

class Any
{
public:

	Any():value(NULL){}
	~Any()
	{
		delete value;
	}

	template<typename ValueType>
	Any(ValueType v):value(NULL)
	{
		value = new AnyNode<ValueType>(v);
	}

	Any(const Any &operand):value(NULL)
	{
		value = (operand.value==NULL)?NULL: (operand.value)->clone();
	}

	template<typename ValueType>
	Any& operator=(const ValueType &right)
	{
		value = new AnyNode<ValueType>(right);
		return *this;
	}

	Any& operator=(const Any &right)
	{
		Any(right).swap(*this);
		return *this;
	}

	Any& swap(Any &right)
	{
		std::swap(this->value, right.value);
		return *this;
	}

	template<typename ValueType>
	int castTo(ValueType &operand) const
	{
		if(value->type() != typeid(ValueType))
			return -1;
		AnyNode<ValueType> *p = dynamic_cast<AnyNode<ValueType>*>(value);
		if (NULL == p) {
			return -1;
		}
		operand = p->value;
		return 0;
	}

	bool isEmpty() const 
	{
		return NULL == value;
	}


private:

	class AnyBase
	{
	public:
		virtual ~AnyBase(){}
		virtual const std::type_info& type() = 0;
		virtual AnyBase* clone() = 0;
	};

	template<typename ValueType>
	class AnyNode : public AnyBase
	{
	public:
		AnyNode(const ValueType &v):value(v){}	
		const std::type_info& type()
		{
			return typeid(value);
		}

		AnyBase* clone()
		{
			return new AnyNode(value);
		}

		ValueType value;
	};

	AnyBase *value;
};

NAMESPACE_END
#endif
