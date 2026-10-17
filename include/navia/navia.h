/**
 * @file navia.h
 * @author Miku AuahDark
 * @brief NPad's Audio Video decoding library.
 * @version 0.1.0
 * @copyright Copyright (c) 2024 Miku AuahDark Licensed under zlib/libpng license.
 */

#ifndef _NAVIA_H_
#define _NAVIA_H_

#define NAVIA_VERSION_MAJOR 0
#define NAVIA_VERSION_MINOR 3
#define NAVIA_VERSION_PATCH 0
#define NAVIA_VERSION_FORMAT(a, b, c) ((a << 16) | (b << 8) | c)
#define NAVIA_VERSION NAVIA_VERSION_FORMAT(NAVIA_VERSION_MAJOR, NAVIA_VERSION_MINOR, NAVIA_VERSION_PATCH)

#ifdef NAVIA_SHARED
#	if defined(_NAVIA_IMPLEMENTATION_)
#		if defined(_MSC_VER)
#			define NAVIA_API __declspec(dllexport)
#		elif defined(__GNUC__) || defined(__clang__)
#			define NAVIA_API __attribute__((visibility("default")))
#		else
#			define NAVIA_API
#		endif
#	elif defined(_MSC_VER)
#		define NAVIA_API __declspec(dllimport)
#	else
#		define NAVIA_API extern
#	endif
#else
#	define NAVIA_API extern
#endif /* NAVIA_SHARED */

#include "attributes.h"
#include "audioformat.h"
#include "input.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get integer version of currently loaded Navia library.
 * @return Integer version of Navia, formatted same as NAVIA_VERSION.
 */
NAVIA_API uint32_t navia_version();

/**
 * @brief Get null-terminated string representation of currently loaded Navia library.
 * @return Pointer to null-terminated string (this always points to valid pointer).
 */
NAVIA_API const char *navia_version_string();

/**
 * @brief Get last error message from last invocation of Navia functions.
 * @return Pointer to the error message, or NULL if there are no errors.
 * @note Error messages are local to the caller thread.
 * @warning The error message pointer will be invalid when calling another Navia function!
 */
NAVIA_API const char *navia_error();

/**
 * @brief Populate pointer of navia_input to read input data from memory.
 * @param input Allocated, but uninitialized pointer to navia_input.
 * @param buf Memory buffer.
 * @param size Size of memory buffer, in bytes.
 * @note Ensure `buf` exist during the whole lifetime of the navia_input.
 */
NAVIA_API void navia_input_populate_from_memory(navia_input *input, void *buf, size_t size);

/**
 * @brief Populate pointer of navia_input to read input data from file using C's stdio file operations.
 * @param input Allocated, but uninitialized pointer to navia_input.
 * @param filename Path to the UTF-8 encoded file.
 * @return 1 if success, 0 otherwise.
 */
NAVIA_API navia_bool navia_input_populate_from_file(navia_input *input, const char *filename);

/**
 * @brief Get amount of available backends.
 * @return Amount of available backends. 0 means no backends are available.
 */
NAVIA_API size_t navia_backend_count();

/**
 * @brief Get an unique name identifier of a backend.
 * @param index **1-based index** of the backend. So, 1 is the first backend, 2 is the second backend, and so on.
 * @return Lowercase name of the backend, or NULL on failure.
 * @note A backend name is unlikely to change.
 */
NAVIA_API const char *navia_backend_name(size_t index);

/**
 * @brief Get the backend type.
 * @param index **1-based index** of the backend. So, 1 is the first backend, 2 is the second backend, and so on.
 * @return Valid backend type enum or NAVIA_BACKENDTYPE_UNKNOWN on failure.
 */
NAVIA_API navia_backendtype navia_backend_type(size_t index);

/**
 * @brief Get additional information of a certain backend.
 * @param index **1-based index** of the backend. So, 1 is the first backend, 2 is the second backend, and so on.
 * @return Additional information about the backend, or NULL if there are no additional information or on failure.
 * @note use navia_error() to check if an error occured or a backend has no additional information.
 */
NAVIA_API const char *navia_backend_info(size_t index);

/**
 * @brief Open new Navia instance.
 *
 * The `filename` parameter acts as a "hint" to backends to try best possible format or to deduce the media format
 * directly. While it's not required, some backends require it, so it's recommended to specify one if possible. When
 * `filename` is specified, it must be valid UTF-8 string.
 *
 * @param input Pointer to "input data". This function will take the ownership of the input.
 * @param filename Pseudo-filename that will be used to improve probing. This can be NULL, but it's recommended to
 *                 specify a pseudo-filename (the file doesn't have to exist in filesystem).
 * @param settings Additional settings to specify. This can be NULL.
 * @return Pointer to Navia instance, or NULL on failure.
 * @note When the function errors, the input ownership will be given back to the caller.
 */
NAVIA_NODISCARD NAVIA_API navia_t *navia_open(navia_input *input, const char *filename, const navia_settings *settings);

/**
 * @brief Close existing Navia instance.
 * @param navia Pointer to Navia instance.
 */
NAVIA_API void navia_close(navia_t *navia);

/**
 * @brief Get backend index that crated this Navia instance.
 * @param navia Pointer to Navia instance.
 * @return **1-based index** of the backend that created this Navia instance.
 */
NAVIA_API size_t navia_backend_index(const navia_t *navia);

/**
 * @brief Get amount of streams in the Navia instance.
 * @param navia Pointer to Navia instance.
 * @return Amount of streams.
 */
NAVIA_API size_t navia_nstreams(const navia_t *navia);

/**
 * @brief Get stream information
 * @param navia Pointer to Navia instance.
 * @param index Stream index.
 * @return Pointer to Navia stream information, or NULL on failure.
 */
NAVIA_API const navia_streaminfo_t *navia_stream_info(const navia_t *navia, size_t index);

/**
 * @brief Check if stream at specific index is enabled.
 *
 * Disabled stream won't generate data during navia_read().
 *
 * @param navia Pointer to Navia instance.
 * @param index Stream index.
 * @return 1 if enabled, 0 otherwise.
 * @sa navia_stream_enable
 */
NAVIA_API navia_bool navia_stream_is_enabled(const navia_t *navia, size_t index);

/**
 * @brief Set if specific stream should be enabled.
 *
 * Disabled stream won't generate data during navia_read(). This can serve as optimization.
 *
 * This function is only callable if the stream is not yet prepared.
 *
 * @param navia Pointer to Navia instance.
 * @param index Stream index.
 * @param enable 1 to enable, 0 to disable.
 * @return 1 if the change success, 0 otherwise.
 * @sa navia_stream_enabled
 */
NAVIA_API navia_bool navia_stream_enable(navia_t *navia, size_t index, navia_bool enable);

/**
 * @brief Get media position.
 * @param navia Pointer to Navia instance.
 * @return Current media position in seconds, or -1 if unknown.
 */
NAVIA_API double navia_tell(navia_t *navia);

/**
 * @brief Get media duration.
 * @param navia Pointer to Navia instance.
 * @return Media duration, or -1 on unknown.
 */
NAVIA_API double navia_duration(navia_t *navia);

/**
 * @brief Set media position
 * @param navia Pointer to Navia instance.
 * @param position Position in seconds, relative to the beginning of the media.
 * @return New (re-adjusted) position, or -1 on unknown or failure.
 * @note navia_error() will return NULL on success but new position can't be determined, non-NULL otherwise.
 */
NAVIA_API double navia_seek(navia_t *navia, double position);

/**
 * @brief Initialize decoders.
 *
 * You can call this once to finalize your stream enablement setup. This will actually initialize all the decoders.
 * Once all the decoder is initialized, changing stream enablement is no longer possible. This is also called by
 * navia_read() if it's not been initialized yet.
 *
 * Calling this function multiple times is no-op.
 *
 * @param navia Pointer to Navia instance.
 * @return Is preparation succeeded?
 */
NAVIA_API bool navia_prepare(navia_t *navia);

/**
 * @brief Check if decoders have been initialized.
 * @param navia Pointer to Navia instance.
 * @return Is instance have been prepared for decoding?
 */
NAVIA_API bool navia_is_prepared(const navia_t *navia);

/**
 * @brief Decode stream from Navia instance.
 *
 * If the decoders is not yet initialized, this also initialize the decoders.
 *
 * @param navia Pointer to Navia instance.
 * @return Pointer to the Navia decoded frame instance, or NULL on failure.
 * @note navia_error() will return NULL on EOS, non-NULL otherwise.
 * @sa navia_frame_free navia_prepare
 */
NAVIA_NODISCARD NAVIA_API navia_frame_t *navia_read(navia_t *navia);

/**
 * @brief Get stream type.
 * @param streaminfo Pointer to Navia stream information.
 * @return Stream type.
 */
NAVIA_API navia_streamtype navia_streaminfo_type(const navia_streaminfo_t *streaminfo);


/**
 * @brief Calculate the size in bytes of single audio sample frame.
 * @param streaminfo Pointer to Navia stream information.
 * @return Size of 1 sample frame, in bytes.
 */
NAVIA_API size_t navia_audio_size(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get audio sample rate.
 * @param streaminfo Pointer to Navia stream information.
 * @return Audio sample rate.
 * @note This call only return meaningful value if the stream is an audio.
 */
NAVIA_API uint32_t navia_audio_sample_rate(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get number of audio channels.
 * @param streaminfo Pointer to Navia stream information.
 * @return Number of audio channels.
 * @note This call only return meaningful value if the stream is an audio.
 */
NAVIA_API uint32_t navia_audio_nchannels(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get the bitwise audio format.
 * @param streaminfo Pointer to Navia stream information.
 * @return Bitwise audio format.
 * @note The bitwise audio format is same as [SDL's AudioFormat](https://wiki.libsdl.org/SDL3/SDL_AudioFormat)
 * @note This call only return meaningful value if the stream is an audio.
 */
NAVIA_API navia_audioformat navia_audio_format(const navia_streaminfo_t *streaminfo);

/**
 * @brief Calculate the size of uncompressed video frame without any padding.
 * @param streaminfo Pointer to Navia stream information.
 * @return Size of 1 video frame, in bytes.
 * @note This call only return meaningful value if the stream is a video.
 */
NAVIA_API size_t navia_video_size(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get amount of planes for this video format.
 * @param streaminfo Pointer to Navia stream information.
 * @return Amount of planes for this video format.
 */
NAVIA_API size_t navia_video_plane_count(navia_pixelformat pixfmt);

/**
 * @brief Get the dimension of video plane without any padding, in bytes.
 *
 * @param streaminfo Pointer to Navia stream information.
 * @param index Plane indices.
 * @param width Pointer to store the plane width in bytes.
 * @param height Pointer to store the plane height in bytes.
 * @note This call only return meaningful value if the stream is a video.
 */
NAVIA_API void navia_video_plane_dimensions(const navia_streaminfo_t *streaminfo, size_t index, size_t *width, size_t *height);

/**
 * @brief Get video dimensions.
 * @param streaminfo Pointer to Navia stream information.
 * @param width Pointer to store the video width.
 * @param height Pointer to store the video height.
 * @note This call only return meaningful value if the stream is a video.
 */
NAVIA_API void navia_video_dimensions(const navia_streaminfo_t *streaminfo, uint32_t *width, uint32_t *height);

/**
 * @brief Get video decode pixel format.
 * @param streaminfo Pointer to Navia stream information.
 * @return Video pixel format.
 * @note This call only return meaningful value if the stream is a video.
 */
NAVIA_API navia_pixelformat navia_video_pixel_format(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get video frames per second.
 * @param streaminfo Pointer to Navia stream information.
 * @return Video FPS, or 0 if unknown.
 * @note This function may be an approximate.
 * @note This call only return meaningful value if the stream is a video.
 */
NAVIA_API double navia_video_fps(const navia_streaminfo_t *streaminfo);

/**
 * @brief Get stream index correspond to the Navia frame instance.
 * @param frame Pointer to the Navia frame instance.
 * @return Stream index of the Navia frame.
 */
NAVIA_API size_t navia_frame_streamindex(const navia_frame_t *frame);

/**
 * @brief Get stream type correspond to the Navia frame instance.
 * @param frame Pointer to the Navia frame instance.
 * @return Stream info of the Navia frame.
 */
NAVIA_API const navia_streaminfo_t *navia_frame_streaminfo(const navia_frame_t *frame);

/**
 * @brief Get presentation timestamp of this frame.
 * @param frame Pointer to the Navia frame instance.
 * @return Presentation time of the decoded data in seconds, or -1 if unknown.
 */
NAVIA_API double navia_frame_tell(const navia_frame_t *frame);

/**
 * @brief Get hardware acceleration type of this frame.
 * @param frame Pointer to the Navia frame instance.
 * @return Hardware acceleration type of the current Navia frame.
 */
NAVIA_API navia_hwacceltype navia_frame_hwacceltype(const navia_frame_t *frame);

/**
 * @brief Make the current frame available for reading.
 *
 * For hardware-accelerated video decoding, this copies the image contents from GPU to CPU.
 * @param frame Pointer to the Navia frame instance.
 * @param strides Pointer to store the list of strides/pitch of each plane in bytes. Negative stride means plane is bottom-up. For audio, this always positive.
 * @param nplanes Pointer to store the amount of planes. For audio, this always writes 1. This can be NULL.
 * @return Array of pointer to each plane, or NULL on failure. Subsequent calls to this function return same pointer.
 * @note `navia_frame_release` must only be called once regardless on how many times `navia_frame_acquire` is called.
 * @sa navia_streaminfo_type_plane_count
 * @sa navia_frame_release
 */
NAVIA_NODISCARD NAVIA_API const uint8_t *const *navia_frame_acquire(navia_frame_t *frame, ptrdiff_t **strides, size_t *nplanes);

/**
 * @brief Get the hardware-acceleration-specific handle.
 * @param frame Pointer to the Navia frame instance.
 * @return Hardware-acceleration-specific handle casted to `void*` or NULL on failure.
 * @note Navia owns the handle, so `navia_frame_free` will free it.
 * @warning The image contents of the handle when keeping `navia_frame_t` is not guaranteed to be same after decoding
 *          another frame! Copy the image contents to (other) device or to CPU (with `navia_frame_acquire`) if necessary.
 * @sa navia_hwacceltype
 */
NAVIA_NODISCARD NAVIA_API void *navia_frame_hwhandle(navia_frame_t *frame);

/**
 * @brief Release the previously-acquired frame, making the acquired pointer invalid.
 * @param frame Pointer to the Navia frame instance.
 * @sa navia_frame_acquire
 */
NAVIA_API void navia_frame_release(navia_frame_t *frame);

/**
 * @brief Free the Navia frame instance.
 *
 * If the frame was acquired, it will be released.
 * @param frame Pointer to the Navia frame instance.
 */
NAVIA_API void navia_frame_free(navia_frame_t *frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _NAVIA_H_ */
