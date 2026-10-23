#ifdef _NAVIA_FFMPEG_VERSION

#ifndef _NAVIA_BACKEND_FFMPEG_INTERNAL_
#define _NAVIA_BACKEND_FFMPEG_INTERNAL_

#include <memory>
#include <string>
#include <vector>

#include "Internal.hpp"
#include "FFmpegBackend.hpp"
#include "FFmpegCommon.hpp"
#include "DynLib.hpp"

namespace navia::_NAVIA_FFMPEG_NAMESPACE
{

using UniqueAVFormatContext = std::unique_ptr<AVFormatContext, decltype(&avformat_free_context)>;
using UniqueAVIOContext = std::unique_ptr<AVIOContext, ffmpeg_common::DoublePointerDeleter<AVIOContext>>;
using UniqueAVCodecContext = std::unique_ptr<AVCodecContext, ffmpeg_common::DoublePointerDeleter<AVCodecContext>>;
using UniqueAVPacket = std::unique_ptr<AVPacket, ffmpeg_common::DoublePointerDeleter<AVPacket>>;
using UniqueAVFrame = std::unique_ptr<AVFrame, ffmpeg_common::DoublePointerDeleter<AVFrame>>;

class FFmpegBackend;

class FFmpegFrame: public Frame
{
public:
	FFmpegFrame(
		FFmpegBackend *backend,
		navia_streaminfo_t *sinfo,
		AVFrame *frame,
		const AVCodecContext *cctx,
		double pts,
		size_t si
	);
	~FFmpegFrame() override;
	size_t getStreamIndex() const noexcept override;
	const navia_streaminfo_t *getStreamInfo() const noexcept override;
	double tell() const noexcept override;
	const uint8_t *const *acquire(ptrdiff_t **strides, size_t *nplanes) override;
	void release() noexcept override;
	navia_hwacceltype getHWAccelType() const noexcept override;
	void *getHWAccelHandle() override;

private:
	AcquireData acquireData;
	AVFrame *frame;
	const AVCodecContext *codecContext;
	FFmpegBackend *f;
	navia_streaminfo_t *streamInfo;
	double pts;
	size_t index;
};

class FFmpegState: public State
{
public:
	FFmpegState(FFmpegBackend *backend, UniqueAVFormatContext &formatContext, UniqueAVIOContext &ioContext, const navia_settings &settings);
	~FFmpegState() override;
	Backend *getBackend() const noexcept override;
	size_t getStreamCount() const noexcept override;
	const navia_streaminfo_t *getStreamInfo(size_t index) const noexcept override;
	bool isStreamEnabled(size_t index) const noexcept override;
	bool setStreamEnabled(size_t index, bool enabled) override;
	double getDuration() noexcept override;
	double getPosition() noexcept override;
	double setPosition(double off) override;
	bool prepare() override;
	bool isPrepared() const noexcept override;
	navia_frame_t *read() override;

private:
	navia_frame_t *decode(AVFrame *frame, size_t index);
	bool canDecode(size_t index);
	std::vector<AVHWDeviceType> getHWAccels();
	static AVPixelFormat pickPixelFormat(AVCodecContext *s, const AVPixelFormat *fmt) noexcept;
	AVBufferRef *createHWCtx(AVHWDeviceType devtype, const navia_settings &settings) noexcept;

	FFmpegBackend *f;
	UniqueAVFormatContext formatContext;
	UniqueAVIOContext ioContext;
	UniqueAVPacket tempPacket;
	UniqueAVFrame tempFrame;
	double position;
	bool eof;
	bool prepared;

	std::vector<navia_streaminfo_t> streamInfo;
	std::vector<AVCodecContext*> decoders;
	std::vector<SwrContext*> resamplers;
	std::vector<SwsContext*> rescalers;
	std::vector<bool> streamEofs;
};

class FFmpegBackend: public Backend
{
public:
	FFmpegBackend();
	~FFmpegBackend() override;
	const char *getName() const noexcept override;
	navia_backendtype getType() const noexcept override;
	const char *getInfo() override;
	State *open(navia_input *input, const char *filename, const navia_settings *settings) override;

private:
	friend class FFmpegState;
	friend class FFmpegFrame;

	DynLib avutil, avcodec, avformat, swscale, swresample;
	std::string info;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define _NAVIA_PROXY_FUNCTION_POINTER(lib, n) decltype(n) *func_##n;
#include "FFmpegPointers.h"
#undef _NAVIA_PROXY_FUNCTION_POINTER

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
};

}

#endif /* _NAVIA_BACKEND_FFMPEG_INTERNAL_ */
#endif /* _NAVIA_FFMPEG_VERSION */
