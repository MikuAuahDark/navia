#ifndef _NAVIA_BACKEND_FFMPEG_5_
#define _NAVIA_BACKEND_FFMPEG_5_

#include "../Config.hpp"

#ifdef NAVIA_BACKEND_FFMPEG_5

#ifdef _NAVIA_FFMPEG_VERSION
#undef _NAVIA_FFMPEG_VERSION
#endif
#define _NAVIA_FFMPEG_VERSION 5

#include "../ffmpeg_common/FFmpegBackend.hpp"
#undef _NAVIA_BACKEND_FFMPEG_

#endif /* NAVIA_BACKEND_FFMPEG_5 */
#endif /* _NAVIA_BACKEND_FFMPEG_5_ */
