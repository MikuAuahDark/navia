#ifndef _NAVIA_DYNLIB_HPP_
#define _NAVIA_DYNLIB_HPP_

#include <string>

namespace navia
{

class DynLib
{
public:
	DynLib();
	DynLib(const std::string &name);
	DynLib(DynLib&& other) noexcept;
	DynLib(DynLib&) = delete;
	~DynLib();
	DynLib& operator=(DynLib&& other) noexcept;

	template<typename T> bool get(const std::string &name, T* dest)
	{
		bool success = false;
		void *result = _get(name, success);
		*dest = (T) result;
		return success;
	}

private:
	void *_get(const std::string &name, bool &success);
	void close();

	void *mod;
};

}

#endif /* _NAVIA_DYNLIB_HPP_ */
