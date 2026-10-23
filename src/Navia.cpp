#include <exception>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>
#include <vector>

#include "Internal.hpp"
#include "Common.hpp"
#include "Config.hpp"

#ifdef NAVIA_BACKEND_ANDROIDNDK
#include "androidndk/AndroidNDKBackend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_4
#include "ffmpeg4/FFmpeg4Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_5
#include "ffmpeg5/FFmpeg5Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_6
#include "ffmpeg6/FFmpeg6Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_7
#include "ffmpeg7/FFmpeg7Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_8
#include "ffmpeg8/FFmpeg8Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_FFMPEG_9
#include "ffmpeg9/FFmpeg9Backend.hpp"
#endif

#ifdef NAVIA_BACKEND_GSTREAMER
#include "gstreamer/GStreamerBackend.hpp"
#endif

#ifdef NAVIA_BACKEND_MEDIAFOUNDATION
#include "mediafoundation/MediaFoundationBackend.hpp"
#endif

#include "Error.hpp"
#include "InputFile.hpp"
#include "InputMemory.hpp"

#include "../include/navia/navia.h"

static class BackendContainer
{
public:
	BackendContainer(std::initializer_list<navia::Backend*(*)()> backendlist)
	: initialized(false)
	, activeBackend()
	, factory(backendlist)
	, mutex()
	{}

	~BackendContainer()
	{
		for (navia::Backend *b: activeBackend)
			delete b;
	}

	void init()
	{
		std::lock_guard lg(mutex);

		if (!initialized)
		{
			for (auto func: factory)
			{
				if (func)
				{
					navia::Backend *b = func();

					if (b != nullptr)
						activeBackend.push_back(b);
				}
			}

			for (size_t i = 0; i < activeBackend.size(); i++)
				defaultOrder.push_back(i + 1);
			defaultOrder.push_back(0);

			defaultSettings = {
				NAVIA_SETTINGS_VERSION,
				nullptr,
				std::max<uint32_t>(std::thread::hardware_concurrency(), 1),
				navia::getEnvvarBool("NAVIA_DISABLE_HWACCEL") ? NAVIA_HWACCELMODE_DISABLED : NAVIA_HWACCELMODE_DEFAULT
			};
			std::optional<int> threadCount = navia::getEnvvarInt("NAVIA_THREAD_COUNT");
			if (threadCount.has_value())
				defaultSettings.max_threads = (uint32_t) std::max(threadCount.value(), 1);

			initialized = true;
		}

	}

	inline void ensureInit()
	{
		if (!initialized)
			init();
	}

	navia::State *open(navia_input *input, const char *filename, const navia_settings *settings)
	{
		ensureInit();

		if (settings == nullptr)
			settings = &defaultSettings;

		navia_settings newSettings = *settings;
		newSettings.max_threads = std::max<uint32_t>(newSettings.max_threads, 1);

		std::vector<std::string> errors;
		const size_t *order = newSettings.backend_order ? newSettings.backend_order : defaultOrder.data();

		for (size_t backendIndex = *order; *order; backendIndex = *++order)
		{
			if (backendIndex > 0 && backendIndex <= activeBackend.size())
			{
				navia::Backend *b = activeBackend[backendIndex - 1];

				try
				{
					return b->open(input, filename, &newSettings);
				}
				catch (const std::exception &e)
				{
					errors.push_back(e.what());
				}
			}
		}

		if (errors.empty())
			navia::error::set("No backend available");
		else
		{
			std::stringstream ss;
			ss << "Error opening input:";
			for (const std::string &s: errors)
				ss << "\n" << s;

			navia::error::set(ss.str());
		}
		return nullptr;
	}

	size_t count()
	{
		ensureInit();
		return activeBackend.size();
	}

	navia::Backend *getBackend(size_t i)
	{
		ensureInit();

		if (i > 0 && i <= activeBackend.size())
			return activeBackend[i - 1];

		navia::error::set("Index out of range");
		return nullptr;
	}

	size_t getBackendIndex(navia::Backend *backend)
	{
		for (size_t i = 0; i < activeBackend.size(); i++)
		{
			if (activeBackend[i] == backend)
				return i + 1;
		}

		return 0;
	}

private:
	bool initialized;
	std::vector<navia::Backend*> activeBackend;
	std::vector<navia::Backend*(*)()> factory;
	std::vector<size_t> defaultOrder;
	std::mutex mutex;
	navia_settings defaultSettings;
} backendContainer({
#ifdef NAVIA_BACKEND_FFMPEG_9
	&navia::ffmpeg9::create,
#endif
#ifdef NAVIA_BACKEND_FFMPEG_8
	&navia::ffmpeg8::create,
#endif
#ifdef NAVIA_BACKEND_FFMPEG_7
	&navia::ffmpeg7::create,
#endif
#ifdef NAVIA_BACKEND_FFMPEG_6
	&navia::ffmpeg6::create,
#endif
#ifdef NAVIA_BACKEND_FFMPEG_5
	&navia::ffmpeg5::create,
#endif
#ifdef NAVIA_BACKEND_FFMPEG_4
	&navia::ffmpeg4::create,
#endif
#ifdef NAVIA_BACKEND_ANDROIDNDK
	&navia::androidndk::create,
#endif
#ifdef NAVIA_BACKEND_GSTREAMER
	&navia::gstreamer::create,
#endif
#ifdef NAVIA_BACKEND_MEDIAFOUNDATION
	&navia::mediafoundation::create,
#endif
	nullptr
});

namespace navia
{

static std::optional<navia_settings> convertSettings(const uint64_t *settingsbase)
{
	switch (*settingsbase)
	{
		case 0:
			return *((const navia_settings*) settingsbase);
		default:
			return std::nullopt;
	}
}

}

template<typename T, typename C, typename... Args, typename... UArgs>
T wrapcall(C *c, T(C::*m)(Args...), T defval, UArgs... args)
{
	try
	{
		navia::error::set("");
		T result = (c->*m)(args...);
		return result;
	}
	catch (const std::exception &e)
	{
		navia::error::set(e.what());
		return defval;
	}
}

extern "C" uint32_t navia_version()
{
	return NAVIA_VERSION;
}

extern "C" const char *navia_version_string()
{
	return NAVIA_STRINGIZE(NAVIA_VERSION_MAJOR) "." NAVIA_STRINGIZE(NAVIA_VERSION_MINOR) "." NAVIA_STRINGIZE(NAVIA_VERSION_PATCH);
}

extern "C" const char *navia_error()
{
	return navia::error::get();
}

extern "C" void navia_input_populate_from_memory(navia_input *input, void *buf, size_t size)
{
	navia::error::set("");
	navia::input::memory::populate(input, buf, size);
}

extern "C" navia_bool navia_input_populate_from_file(navia_input *input, const char *filename)
{
	navia::error::set("");
	return (navia_bool) navia::input::file::populate(input, filename);
}

extern "C" size_t navia_backend_count()
{
	navia::error::set("");
	return backendContainer.count();
}

extern "C" const char *navia_backend_name(size_t index)
{
	navia::error::set("");
	navia::Backend *backend = backendContainer.getBackend(index);
	return backend ? backend->getName() : nullptr;
}

extern "C" navia_backendtype navia_backend_type(size_t index)
{
	navia::error::set("");
	navia::Backend *backend = backendContainer.getBackend(index);
	return backend ? backend->getType() : NAVIA_BACKENDTYPE_UNKNOWN;
}

extern "C" const char *navia_backend_info(size_t index)
{
	navia::error::set("");
	navia::Backend *backend = backendContainer.getBackend(index);
	return backend ? wrapcall<const char*>(backend, &navia::Backend::getInfo, nullptr) : nullptr;
}

extern "C" navia_t *navia_open(navia_input *input, const char *filename, const navia_settings *settings)
{
	return wrapcall<navia_t*>(&backendContainer, &BackendContainer::open, nullptr, input, filename, settings);
}

extern "C" void navia_close(navia_t *state)
{
	navia::error::set("");
	delete state;
}

extern "C" size_t navia_backend_index(const navia_t *state)
{
	navia::error::set("");
	navia::Backend *b = state->getBackend();
	return backendContainer.getBackendIndex(b);
}

extern "C" size_t navia_nstreams(const navia_t *state)
{
	navia::error::set("");
	return state->getStreamCount();
}

extern "C" const navia_streaminfo_t *navia_stream_info(const navia_t *state, size_t index)
{
	navia::error::set("");
	return state->getStreamInfo(index);
}

extern "C" navia_bool navia_stream_is_enabled(const navia_t *state, size_t index)
{
	navia::error::set("");
	return (navia_bool) state->isStreamEnabled(index);
}

extern "C" navia_bool navia_stream_enable(navia_t *state, size_t index, navia_bool enable)
{
	return (navia_bool) wrapcall(state, &navia::State::setStreamEnabled, false, index, enable);
}

extern "C" double navia_tell(navia_t *state)
{
	navia::error::set("");
	return state->getPosition();
}

extern "C" double navia_duration(navia_t *state)
{
	navia::error::set("");
	return state->getDuration();
}

extern "C" double navia_seek(navia_t *state, double position)
{
	return wrapcall(state, &navia::State::setPosition, -1., position);
}

extern "C" bool navia_prepare(navia_t *state)
{
	return wrapcall(state, &navia::State::prepare, false);
}

extern "C" bool navia_is_prepared(const navia_t *state)
{
	navia::error::set("");
	return state->isPrepared();
}

extern "C" navia_frame_t *navia_read(navia_t *state)
{
	if (!navia_prepare(state))
		return nullptr;

	return wrapcall<navia_frame_t*>(state, &navia::State::read, nullptr);
}

extern "C" navia_streamtype navia_streaminfo_type(const navia_streaminfo_t *sinfo)
{
	navia::error::set("");
	return sinfo->type;
}

extern "C" size_t navia_video_plane_count(navia_pixelformat fmt)
{
	navia::error::set("");
	return navia::planeCount(fmt);
}

extern "C" size_t navia_audio_size(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_AUDIO)
	{
		navia::error::set("Not an audio stream");
		return 0;
	}

	navia::error::set("");
	return sinfo->audio.size();
}

extern "C" uint32_t navia_audio_sample_rate(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_AUDIO)
	{
		navia::error::set("Not an audio stream");
		return 0;
	}

	navia::error::set("");
	return sinfo->audio.sample_rate;
}

extern "C" uint32_t navia_audio_nchannels(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_AUDIO)
	{
		navia::error::set("Not an audio stream");
		return 0;
	}

	navia::error::set("");
	return sinfo->audio.nchannels;
}

extern "C" navia_audioformat navia_audio_format(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_AUDIO)
	{
		navia::error::set("Not an audio stream");
		return 0;
	}

	navia::error::set("");
	return sinfo->audio.format;
}

extern "C" size_t navia_video_size(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_VIDEO)
	{
		navia::error::set("Not a video stream");
		return 0;
	}

	navia::error::set("");
	size_t result = 0;
	for (size_t i = 0; i < navia::planeCount(sinfo->video.format); i++)
		result += sinfo->plane_width(i) * sinfo->plane_height(i);

	return result;
}

extern "C" void navia_video_plane_dimensions(const navia_streaminfo_t *sinfo, size_t index, size_t *width, size_t *height)
{
	if (sinfo->type != NAVIA_STREAMTYPE_VIDEO)
	{
		navia::error::set("Not a video stream");
		return;
	}

	navia::error::set("");
	*width = sinfo->plane_width(index);
	*height = sinfo->plane_height(index);
}

extern "C" void navia_video_dimensions(const navia_streaminfo_t *sinfo, uint32_t *width, uint32_t *height)
{
	if (sinfo->type != NAVIA_STREAMTYPE_VIDEO)
	{
		navia::error::set("Not a video stream");
		*width = *height = 0;
		return;
	}

	navia::error::set("");
	*width = sinfo->video.width;
	*height = sinfo->video.height;
}

extern "C" navia_pixelformat navia_video_pixel_format(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_VIDEO)
	{
		navia::error::set("Not a video stream");
		return NAVIA_PIXELFORMAT_UNKNOWN;
	}

	navia::error::set("");
	return sinfo->video.format;
}

extern "C" double navia_video_fps(const navia_streaminfo_t *sinfo)
{
	if (sinfo->type != NAVIA_STREAMTYPE_VIDEO)
	{
		navia::error::set("Not a video stream");
		return 0.0;
	}

	navia::error::set("");
	return sinfo->video.fps;
}

extern "C" size_t navia_frame_streamindex(const navia_frame_t *frame)
{
	navia::error::set("");
	return frame->getStreamIndex();
}

extern "C" const navia_streaminfo_t *navia_frame_streaminfo(const navia_frame_t *frame)
{
	navia::error::set("");
	return frame->getStreamInfo();
}

extern "C" double navia_frame_tell(const navia_frame_t *frame)
{
	navia::error::set("");
	return frame->tell();
}

extern "C" navia_hwacceltype navia_frame_hwacceltype(const navia_frame_t *frame)
{
	navia::error::set("");
	return frame->getHWAccelType();
}

extern "C" const uint8_t *const *navia_frame_acquire(navia_frame_t *frame, ptrdiff_t **strides, size_t *nplanes)
{
	return wrapcall<const uint8_t *const *>(frame, &navia::Frame::acquire, nullptr, strides, nplanes);
}

extern "C" void *navia_frame_hwhandle(navia_frame_t *frame)
{
	return wrapcall<void*>(frame, &navia::Frame::getHWAccelHandle, nullptr);
}

extern "C" void navia_frame_release(navia_frame_t *frame)
{
	navia::error::set("");
	return frame->release();
}

extern "C" void navia_frame_free(navia_frame_t *frame)
{
	frame->release();
	delete frame;
}
