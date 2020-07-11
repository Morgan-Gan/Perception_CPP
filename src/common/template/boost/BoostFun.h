#pragma once
#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>

namespace common_template
{
	template<class TFirst>
	void string_format(boost::format& fmt, TFirst&& first)
	{
		fmt % first;
	}

	template<class TFirst, class... TOther>
	void string_format(boost::format& fmt, TFirst&& first, TOther&&... other)
	{
		fmt % first;
		string_format(fmt, other...);
	}

	template<class TFirst, class... TOther>
	std::string string_format(const char* format, TFirst&& first, TOther&&... other)
	{
		boost::format fmt(format);
		string_format(fmt, first, other...);
		return fmt.str();
	}
}