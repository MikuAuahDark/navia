#ifndef _NAVIA_TYPES_H_
#define _NAVIA_TYPES_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque structure that contains a loaded instance of media file.
 * @sa navia_open
 */
typedef struct navia_t navia_t;

/**
 * @brief Opaque structure that contains stream information.
 * @sa navia_stream_info
 * @sa navia_frame_streaminfo
 */
typedef struct navia_streaminfo_t navia_streaminfo_t;

/**
 * @brief Opaque structure that contains decoded stream data.
 * @sa navia_read
 */
typedef struct navia_frame_t navia_frame_t;

#ifdef __cplusplus
typedef bool navia_bool;
#else
#include <stdbool.h>
typedef _Bool navia_bool;
#endif

/**
 * @brief Possible pixel format of decoded video frames.
 * @sa navia_streaminfo_t
 */
typedef enum navia_pixelformat
{
	/* Unknown pixel format (may denote errors) */
	NAVIA_PIXELFORMAT_UNKNOWN = -1,
	/* RGB, 24 bits per pixel, packed. */
	NAVIA_PIXELFORMAT_RGB8,
	/* YUV 4:2:0 subsampling, planar. */
	NAVIA_PIXELFORMAT_YUV420,
	/* YUV 4:4:4 subsampling, planar. */
	NAVIA_PIXELFORMAT_YUV444,
	/* YUV 4:2:0 subsampling, Y is planar, UV is packed. */
	NAVIA_PIXELFORMAT_NV12
} navia_pixelformat;

/**
 * @brief Possible stream type.
 * @sa navia_streaminfo_t
 */
typedef enum navia_streamtype
{
	/* Unknown stream type */
	NAVIA_STREAMTYPE_UNKNOWN = -1,
	/* Audio stream */
	NAVIA_STREAMTYPE_AUDIO,
	/* Video stream */
	NAVIA_STREAMTYPE_VIDEO
} navia_streamtype;

typedef enum navia_backendtype
{
	/* Unknown backend type. */
	NAVIA_BACKENDTYPE_UNKNOWN = -1,
	/* This backend leverages OS-specific API to work (almost guaranteed to exist in particular OS). */
	NAVIA_BACKENDTYPE_OS_API,
	/* This backend leverages 3rd-party library to work (depends if the user installed the library). */
	NAVIA_BACKENDTYPE_3RD_PARTY
} navia_backendtype;

typedef enum navia_hwacceltype
{
	/* Not hardware accelerated or unknown hardware acceleration. */
	NAVIA_HWACCELTYPE_NONE,
	/* Hardware-accelerated by Vulkan. The HW-accelerated handle is `VkImageView*`. */
	NAVIA_HWACCELTYPE_VULKAN,
	/* (Windows) Hardware-accelerated by D3D11. The HW-accelerated handle is `ID3D11Texture2D*`. */
	NAVIA_HWACCELTYPE_D3D11,
	/* (Linux) Hardware-accelerated by VA-API. The HW-accelerated handle is `VASurfaceID*`. */
	NAVIA_HWACCELTYPE_VAAPI,
} navia_hwacceltype;

#define NAVIA_SETTINGS_VERSION 0

typedef struct navia_settings
{
	/* navia_settings struct version. Must be initialized to NAVIA_SETTINGS_VERSION */
	uint64_t version;
	/* 0-terminated **1-based** backend index to try in order. Example: if `{2, 1, 0}` is specified, then it
	 * will try to load using 2nd backend first, then trying the 1st backend. This can be NULL to use default
	 * order (which is `{1, 2, 3, ..., navia_backend_count(), 0}`). */
	const size_t *backend_order;
	/* Hint to backend on how many threads to use. If this is 0, it defaults to value of NAVIA_THREAD_COUNT environment
	 * variable, or amount of threads in the current user system.
	 * **Note**: NAVIA won't check if the specified `max_threads` exceeded the amount of the system CPU thread count,
	 * but it ensure the value is at least 1.
	 */
	uint32_t max_threads;
	/* If true, this hints backends to prefer CPU decoding. */
	navia_bool disable_hwaccel;
} navia_settings;

#endif /* _NAVIA_TYPES_H_ */
