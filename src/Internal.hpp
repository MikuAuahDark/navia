#ifndef _NAVIA_INTERNAL_HPP_
#define _NAVIA_INTERNAL_HPP_

#ifndef _NAVIA_IMPLEMENTATION_
#define _NAVIA_IMPLEMENTATION_
#endif

#define NAVIA_STRINGIZE(x) _NAVIA_STRINGIZE0_(x)
#define _NAVIA_STRINGIZE0_(x) #x

#ifndef NAVIA_CAT
#define NAVIA_CAT(x, y) x##y
#endif

#include <cstdint>

#include "../include/navia/audioformat.h"
#include "../include/navia/input.h"
#include "../include/navia/types.h"

namespace navia
{

using State = ::navia_t;
using StreamInfo = ::navia_streaminfo_t;
using Frame = ::navia_frame_t;

class Backend
{
public:
	virtual ~Backend();
	virtual const char *getName() const noexcept = 0;
	virtual navia_backendtype getType() const noexcept = 0;
	virtual const char *getInfo() = 0;
	virtual State *open(navia_input *input, const char *filename, const navia_settings *settings) = 0;
};

}

struct navia_t
{
	virtual ~navia_t();
	virtual navia::Backend *getBackend() const noexcept = 0;
	virtual size_t getStreamCount() const noexcept = 0;
	virtual const navia_streaminfo_t *getStreamInfo(size_t index) const noexcept = 0;
	virtual bool isStreamEnabled(size_t index) const noexcept = 0;
	virtual bool setStreamEnabled(size_t index, bool enabled) = 0;
	virtual double getDuration() noexcept = 0;
	virtual double getPosition() noexcept = 0;
	virtual double setPosition(double off) = 0;
	virtual bool prepare() = 0;
	virtual bool isPrepared() const noexcept = 0;
	virtual navia::Frame *read() = 0;
};

struct navia_streaminfo_t
{
	struct AudioStreamInfo
	{
		uint32_t nchannels;
		uint32_t sample_rate;
		navia_audioformat format;

		inline size_t size() const
		{
			return NAVIA_AUDIOFORMAT_BYTESIZE(format) * (size_t) nchannels;
		}
	};

	struct VideoStreamInfo
	{
		double fps;
		uint32_t width, height;
		navia_pixelformat format;

		inline size_t size() const
		{
			size_t dimensions = (size_t) width * (size_t) height;

			switch (format)
			{
				case NAVIA_PIXELFORMAT_UNKNOWN:
				default:
					return 0;
				case NAVIA_PIXELFORMAT_RGB8:
				case NAVIA_PIXELFORMAT_YUV444:
					return 3 * dimensions;
				case NAVIA_PIXELFORMAT_YUV420:
				case NAVIA_PIXELFORMAT_NV12:
					return dimensions + 2 * ((width + 1) / 2) * ((height + 1) / 2);
			}
		}

		// For YUV, only the Y plane is considered.
		inline size_t stride() const
		{
			switch (format)
			{
				case NAVIA_PIXELFORMAT_UNKNOWN:
				case NAVIA_PIXELFORMAT_YUV420:
				case NAVIA_PIXELFORMAT_YUV444:
				case NAVIA_PIXELFORMAT_NV12:
				default:
					return width;
				case NAVIA_PIXELFORMAT_RGB8:
					return ((size_t) width) * 3;
			}
		}
	};

	inline size_t plane_width(size_t index) const
	{
		switch (type)
		{
			case NAVIA_STREAMTYPE_UNKNOWN:
			default:
				return 0;
			case NAVIA_STREAMTYPE_VIDEO:
			{
				switch (video.format)
				{
					case NAVIA_PIXELFORMAT_UNKNOWN:
					default:
						return 0;
					case NAVIA_PIXELFORMAT_YUV420:
					{
						switch (index)
						{
							case 0:
								return video.width;
							case 1:
							case 2:
								return (video.width + 1) / 2;
							default:
								return 0;
						}

						return 0;
					}
					case NAVIA_PIXELFORMAT_YUV444:
						return (index >= 0 && index < 3) * (size_t) video.width;
					case NAVIA_PIXELFORMAT_NV12:
						switch (index)
						{
							case 0:
								return video.width;
							case 1:
								return ((video.width + 1) / 2) * 2;
							default:
								return 0;
						}

						return 0;
					case NAVIA_PIXELFORMAT_RGB8:
						return video.width * (size_t) 3;
				}

				return 0;
			}
			case NAVIA_STREAMTYPE_AUDIO:
				return (index == 0) * audio.size();
		}

		return 0;
	}

	inline size_t plane_height(size_t index) const
	{
		switch (type)
		{
			case NAVIA_STREAMTYPE_UNKNOWN:
			default:
				return 0;
			case NAVIA_STREAMTYPE_VIDEO:
			{
				switch (video.format)
				{
					case NAVIA_PIXELFORMAT_UNKNOWN:
					default:
						return 0;
					case NAVIA_PIXELFORMAT_YUV420:
					{
						switch (index)
						{
							case 0:
								return video.height;
							case 1:
							case 2:
								return (video.height + 1) / 2;
							default:
								return 0;
						}

						return 0;
					}
					case NAVIA_PIXELFORMAT_YUV444:
						return (index >= 0 && index < 3) * (size_t) video.height;
					case NAVIA_PIXELFORMAT_NV12:
						switch (index)
						{
							case 0:
								return video.height;
							case 1:
								return (video.height + 1) / 2;
							default:
								return 0;
						}

						return 0;
					case NAVIA_PIXELFORMAT_RGB8:
						return video.height;
				}

				return 0;
			}
			case NAVIA_STREAMTYPE_AUDIO:
				return (size_t) (index == 0);
		}

		return 0;
	}

	navia_streamtype type;
	union
	{
		AudioStreamInfo audio;
		VideoStreamInfo video;
	};
};

struct navia_frame_t
{
	virtual ~navia_frame_t();
	virtual size_t getStreamIndex() const noexcept = 0;
	virtual const navia_streaminfo_t *getStreamInfo() const noexcept = 0;
	virtual double tell() const noexcept = 0;
	virtual const uint8_t *const *acquire(ptrdiff_t **strides, size_t *nplanes) = 0;
	virtual void release() noexcept = 0;
	virtual navia_hwacceltype getHWAccelType() const noexcept = 0;
	virtual void *getHWAccelHandle() = 0;

	inline bool operator<(const navia_frame_t &other) const noexcept
	{
		return tell() < other.tell();
	}
};

#endif /* _NAVIA_INTERNAL_HPP_ */
