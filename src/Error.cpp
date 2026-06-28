#include "Error.hpp"

static thread_local std::string err;

const char *navia::error::get()
{
	return err.empty() ? nullptr : err.c_str();
}

void navia::error::set(const std::string &str)
{
	err = str;
}
