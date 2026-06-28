#ifndef _NAVIA_BACKEND_ANDROIDNDK_INTERNAL_HPP_
#define _NAVIA_BACKEND_ANDROIDNDK_INTERNAL_HPP_

#include "AndroidNDKBackend.hpp"

#ifdef NAVIA_BACKEND_ANDROIDNDK

#include <memory>
#include <vector>

#include <media/NdkMediaExtractor.h>

#include "../DynLib.hpp"

namespace navia::androidndk
{

using UniqueMediaExtractor = std::unique_ptr<AMediaExtractor, decltype(&AMediaExtractor_delete)>;
using UniqueMediaFormat = std::unique_ptr<AMediaFormat, decltype(&AMediaFormat_delete)>;
using UniqueMediaCodec = std::unique_ptr<AMediaCodec, decltype(&AMediaCodec_delete)>;

class AndroidNDKBackend;

class MediaSourceWrapper
{
public:
	MediaSourceWrapper(const MediaSourceWrapper &other) = delete;
	MediaSourceWrapper(AndroidNDKBackend *backend, navia_input *input);
	MediaSourceWrapper(MediaSourceWrapper &&other);
	~MediaSourceWrapper();
	AMediaDataSource *get() const noexcept;

private:
	static ssize_t readAt(void *userdata, off64_t offset, void * buffer, size_t size);
	static ssize_t getSize(void *userdata);
	static void close(void *userdata);
	static ssize_t getAvailableSize(void *userdata, off64_t offset);

	AndroidNDKBackend *f;
	AMediaDataSource *datasource;
	navia_input *input;
};

class AndroidNDKState: public State
{
public:
	AndroidNDKState(AndroidNDKBackend *backend, UniqueMediaExtractor &ext, MediaSourceWrapper &ds);
	~AndroidNDKState() override;
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
	AndroidNDKBackend *f;
	int64_t durationUs;
	int64_t positionUs;
	bool needAdvance, prepared;

	std::vector<bool> activeStream, hasEOS;
	std::vector<navia_streaminfo_t> streamInfo;
	std::vector<UniqueMediaCodec> decoders;

	UniqueMediaExtractor extractor;
	MediaSourceWrapper dataSource;
};

class AndroidNDKBackend: public Backend
{
public:
	AndroidNDKBackend();
	~AndroidNDKBackend() override;
	const char *getName() const noexcept override;
	navia_backendtype getType() const noexcept override;
	const char *getInfo() override;
	State *open(navia_input *input, const char *filename, const navia_settings *settings) override;

private:
	friend class MediaSourceWrapper;
	friend class AndroidNDKState;
	DynLib mediandk;

#define _NAVIA_PROXY_FUNCTION_POINTER(n) decltype(n) *ptr_##n;
#include "AndroidNDKPointers.h"
#undef _NAVIA_PROXY_FUNCTION_POINTER
};

}

#endif /* NAVIA_BACKEND_ANDROIDNDK */

#endif /* _NAVIA_BACKEND_ANDROIDNDK_INTERNAL_HPP_ */
