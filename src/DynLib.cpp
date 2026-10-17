#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "DynLib.hpp"

namespace navia
{

DynLib::DynLib()
: mod(nullptr)
{}

DynLib::DynLib(const std::string &name)
{
#ifdef _WIN32
	mod = LoadLibraryA(name.c_str());
#else
	mod = dlopen(name.c_str(), RTLD_LAZY);
#endif

	if (mod == nullptr)
		throw std::runtime_error("cannot load " + name);
}

DynLib::DynLib(DynLib&& other) noexcept
: mod(other.mod)
{
	other.mod = nullptr;
}

DynLib &DynLib::operator=(DynLib&& other) noexcept
{
	close();
	mod = other.mod;
	other.mod = nullptr;
	return *this;
}

DynLib::~DynLib()
{
	close();
}

void *DynLib::_get(const std::string &name, bool &success)
{
	void *result = nullptr;

#ifdef _WIN32
	SetLastError(0);
	result = (void*) GetProcAddress((HMODULE) mod, name.c_str());
	success = GetLastError() == 0;
#else
	dlerror();
	result = dlsym(mod, name.c_str());
	success = dlerror() == nullptr;
#endif

	// Note: We don't check if `result != nullptr` because it doesn't
	// necessarily signify failure (e.g. the value is actually 0).
	return result;
}

void DynLib::close()
{
	if (mod != nullptr)
	{
#ifdef _WIN32
	FreeLibrary((HMODULE) mod);
#else
	dlclose(mod);
#endif
	}
}

}
