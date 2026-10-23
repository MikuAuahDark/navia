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
	/* Hardware-accelerated by Vulkan. The HW-accelerated handle is (TO BE ADDED). */
	NAVIA_HWACCELTYPE_VULKAN,
	/* (Windows) Hardware-accelerated by D3D11. The HW-accelerated handle is `ID3D11Texture2D*`. */
	NAVIA_HWACCELTYPE_D3D11,
	/* (Linux) Hardware-accelerated by VA-API. The HW-accelerated handle is `VASurfaceID*`. */
	NAVIA_HWACCELTYPE_VAAPI,
} navia_hwacceltype;

#define NAVIA_SETTINGS_VERSION 0

typedef enum navia_hwaccelmode
{
	/* Do not use hardware acceleration to decode video. */
	NAVIA_HWACCELMODE_DISABLED,
	/* Try to use hardware acceleration, optionally with the existing device if necessary, but allow making new device
	 * of different GPU API if fails. This allows CPU fallback as well.
	 */
	NAVIA_HWACCELMODE_DEFAULT,
	/* Use the existing device. Do not allow using other GPU API. Note that this still allows CPU fallback. */
	NAVIA_HWACCELMODE_EXISTING_ONLY,
	NAVIA_HWACCELMODE_MAX_ENUM,
} navia_hwaccelmode;

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
	 * **Note**: Navia won't check if the specified `max_threads` exceeded the amount of the system CPU thread count,
	 * but it ensure the value is at least 1.
	 */
	uint32_t max_threads;

	/* Video hardware-acceleration mode. */
	navia_hwaccelmode hwaccel_mode;
	/* Optional pointer to `ID3D11Device`, cast as `void*` to use for HW-accelerated decoding. The D3D11 device must
	 * be created with `D3D11_CREATE_DEVICE_VIDEO_SUPPORT` flag and multithread-protected.
	 *
	 * Ignored if `hwaccel_mode` is `NAVIA_HWACCELMODE_DISABLED`.
	 *
	 * Note that if the passed device is only allocated for Navia purpose, you need to do texture interop yourself.
	 */
	void *d3d11_device;
} navia_settings;

#endif /* _NAVIA_TYPES_H_ */
