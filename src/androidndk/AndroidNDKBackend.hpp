#ifndef _NAVIA_BACKEND_ANDROIDNDK_HPP_
#define _NAVIA_BACKEND_ANDROIDNDK_HPP_

#include "../Config.hpp"

#ifdef NAVIA_BACKEND_ANDROIDNDK

#include "../Internal.hpp"
#include "../Backend.hpp"

namespace navia::androidndk
{

Backend *create();

}

#endif /* NAVIA_BACKEND_ANDROIDNDK */

#endif /* _NAVIA_BACKEND_ANDROIDNDK_HPP_ */
