#ifndef _NAVIA_BACKEND_GSTREAMER_INTERNAL_H_
#define _NAVIA_BACKEND_GSTREAMER_INTERNAL_H_

#include "Config.hpp"

#ifdef NAVIA_BACKEND_GSTREAMER

#include <gst/gst.h>

#include <memory>
#include <string>
#include <vector>
#include <queue>

#include "Internal.hpp"
#include "Common.hpp"
#include "DynLib.hpp"

namespace navia::gstreamer
{

template<typename T>
using UniqueGst = std::unique_ptr<T, void(*)(T*)>;
template<typename T>
using UniqueGstObject = std::unique_ptr<T, decltype(&gst_object_unref)>;
using UniqueGstElement = UniqueGstObject<GstElement>;

class GStreamerBackend;

class GStreamerAudioFrame: public Frame
{
public:
	GStreamerAudioFrame(
		GStreamerBackend *backend,
		GstBuffer *buffer,
		navia_streaminfo_t *sinfo,
		size_t si,
		double pts
	);
	~GStreamerAudioFrame() override;
	size_t getStreamIndex() const noexcept override;
	const navia_streaminfo_t *getStreamInfo() const noexcept override;
	double tell() const noexcept override;
	const uint8_t *const *acquire(ptrdiff_t **strides, size_t *nplanes) override;
	void release() noexcept override;
	navia_hwacceltype getHWAccelType() const noexcept override;
	void *getHWAccelHandle() override;

private:
	AcquireData acquireData;
	GstMapInfo mapInfo;
	GstMemory *memory;
	double pts;
	GStreamerBackend *f;
	GstBuffer *buffer;
	navia_streaminfo_t *streamInfo;
	size_t streamIndex;
};

class GStreamerVideoFrame: public Frame
{
public:
	GStreamerVideoFrame(
		GStreamerBackend *backend,
		UniqueGst<GstVideoInfo> &&videoInfo,
		GstBuffer *buffer,
		navia_streaminfo_t *sinfo,
		size_t si,
		double pts
	);
	~GStreamerVideoFrame() override;
	size_t getStreamIndex() const noexcept override;
	const navia_streaminfo_t *getStreamInfo() const noexcept override;
	double tell() const noexcept override;
	const uint8_t *const *acquire(ptrdiff_t **strides, size_t *nplanes) override;
	void release() noexcept override;
	navia_hwacceltype getHWAccelType() const noexcept override;
	void *getHWAccelHandle() override;

private:
	AcquireData acquireData;
	GstVideoFrame videoFrame;
	UniqueGst<GstVideoInfo> videoInfo;
	double pts;
	GStreamerBackend *f;
	GstBuffer *buffer;
	navia_streaminfo_t *streamInfo;
	size_t streamIndex;
};

class GStreamerState: public State
{
public:
	GStreamerState(GStreamerBackend *backend, navia_input *input);
	~GStreamerState() override;
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
	struct AppSinkWrapper
	{
		AppSinkWrapper() = delete;
		AppSinkWrapper(const AppSinkWrapper &other) = default;
		AppSinkWrapper(AppSinkWrapper &&other) = default;
		AppSinkWrapper(GStreamerState *state, size_t streamIndex);
		~AppSinkWrapper();

		inline bool ok()
		{
			return queue && convert && sink && streamInfo.type != NAVIA_STREAMTYPE_UNKNOWN;
		}

		navia_streaminfo_t streamInfo;
		size_t streamIndex;
		GStreamerState *self;
		GstElement *queue, *convert, *sink;
		gulong probeID;
		bool eos, enabled;
	};

	struct FrameComparator
	{
		bool operator()(const Frame *lhs, const Frame *rhs) const noexcept;
	};

	GStreamerBackend *f;
	navia_input input;
	UniqueGstObject<GstBus> bus;
	UniqueGstElement pipeline;
	GstElement *source, *decoder;
	std::vector<std::unique_ptr<AppSinkWrapper>> streams;
	std::priority_queue<Frame*, std::deque<Frame*>, FrameComparator> queuedFrames;
	bool padProbed, eos, prepared;

	GstCaps *newVideoCapsForNAVIA();
	GstCaps *newAudioCapsForNAVIA();
	void clearQueuedFrames();
	void pollBus(bool noexception = false);
	Frame *dispatchDecode(GstBuffer *buffer, size_t streamIndex);
	static void padAdded(GstElement *element, GstPad *newPad, GStreamerState *self);
	static void noMorePads(GstElement *element, GStreamerState *self);
	static void needData(GstElement *element, guint length, GStreamerState *self);
	static gboolean seekData(GstElement *element, guint64 pos, GStreamerState *self);
};

class GStreamerBackend: public Backend
{
public:
	GStreamerBackend();
	~GStreamerBackend() override;
	const char *getName() const noexcept override;
	navia_backendtype getType() const noexcept override;
	const char *getInfo() override;
	State *open(navia_input *input, const char *filename, const navia_settings *settings) override;

private:
	DynLib glib, gobject, gstreamer, gstvideo;
	std::string version;

public:
#define _NAVIA_PROXY_FUNCTION_POINTER(lib, n) decltype(n) *ptr_##n;
#include "GStreamerPointers.h"
#undef _NAVIA_PROXY_FUNCTION_POINTER
};

}

#endif /* NAVIA_BACKEND_GSTREAMER */
#endif /* _NAVIA_BACKEND_GSTREAMER_INTERNAL_H_ */
