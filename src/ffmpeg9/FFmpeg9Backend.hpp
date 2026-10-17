#ifndef _NAVIA_BACKEND_FFMPEG_9_
#define _NAVIA_BACKEND_FFMPEG_9_

#include "../Config.hpp"

#ifdef NAVIA_BACKEND_FFMPEG_9

#ifdef _NAVIA_FFMPEG_VERSION
#undef _NAVIA_FFMPEG_VERSION
#endif
#define _NAVIA_FFMPEG_VERSION 9

#include "../ffmpeg_common/FFmpegBackend.hpp"
#undef _NAVIA_BACKEND_FFMPEG_

#endif /* NAVIA_BACKEND_FFMPEG_9 */
#endif /* _NAVIA_BACKEND_FFMPEG_9_ */
