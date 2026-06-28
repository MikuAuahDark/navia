#ifndef _NAVIA_BACKEND_GSTREAMER_H_
#define _NAVIA_BACKEND_GSTREAMER_H_

#include "Config.hpp"

#ifdef NAVIA_BACKEND_GSTREAMER

#include "Internal.hpp"

namespace navia::gstreamer
{

Backend *create();

}

#endif /* NAVIA_BACKEND_GSTREAMER */
#endif /* _NAVIA_BACKEND_GSTREAMER_H_ */
