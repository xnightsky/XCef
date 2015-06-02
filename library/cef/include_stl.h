#pragma once
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <conio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <stdint.h>
#include <process.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <assert.h>
#include <type_traits>
#include <sstream>
#include <mutex>



namespace xstd
{
	inline std::string vformat(std::string fmt, va_list va)
	{
		std::vector<char>	buf;
		int					ncount = 0;

		ncount = _vcprintf_s(fmt.c_str(), va);
		buf.resize(ncount + 1);
		vsnprintf_s(&buf[0], ncount, _TRUNCATE, fmt.c_str(), va);

		return std::string(&buf[0]);
	}

	template<class S>
	inline S format(S fmt, ...)
	{
		static_assert(std::is_same<S, std::string>::value, "S is not std::string.");
		return S()
	}
	template<>
	inline std::string format(std::string fmt, ...)
	{
		std::string		result;

		va_list		va;
		va_start(va, fmt);
		result = vformat(fmt, va);
		va_end(va);

		return result;
	}
	inline std::string format(const char* fmt, ...){
		std::string		result;

		va_list		va;
		va_start(va, fmt);
		result = vformat(std::string(fmt), va);
		va_end(va);

		return result;
	}


	// http://stackoverflow.com/questions/5290089/how-to-convert-a-number-to-string-and-vice-versa-in-c
	// make_string
	// eg:
	//	string str = make_string() << 6 << 8 << "hello";
	class make_string {
	public:
		template <typename T>
		make_string& operator<<(T const & val) {
			buffer_ << val;
			return *this;
		}
		operator std::string() const {
			return buffer_.str();
		}
	private:
		std::ostringstream buffer_;
	};

	// parse_string
	// eg:
	//	int x = parse_string<int>("78");
	template <typename RETURN_TYPE, typename STRING_TYPE>
	inline RETURN_TYPE parse_string(const STRING_TYPE& str) {
		std::stringstream buf;
		buf << str;
		RETURN_TYPE val;
		buf >> val;
		return val;
	}
}


