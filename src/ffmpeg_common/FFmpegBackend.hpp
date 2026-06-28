#ifdef _NAVIA_FFMPEG_VERSION

#ifndef _NAVIA_BACKEND_FFMPEG_
#define _NAVIA_BACKEND_FFMPEG_

#include "Config.hpp"

#include "Internal.hpp"

#include "FFmpegSetup.h"

namespace navia::_NAVIA_FFMPEG_NAMESPACE
{

Backend *create();

}

#endif /* _NAVIA_BACKEND_FFMPEG_ */
#endif /* _NAVIA_FFMPEG_VERSION */
