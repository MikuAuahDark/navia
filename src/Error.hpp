#ifndef _NAVIA_ERROR_HPP_
#define _NAVIA_ERROR_HPP_

#include <exception>
#include <string>

namespace navia::error
{

const char *get();
void set(const std::string &err);
inline void set(const std::exception &e)
{
	set(e.what());
}

}

#endif /* _NAVIA_ERROR_HPP_ */
