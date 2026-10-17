/**
 * This is an example code on how to use Navia to inspect all audio and video
 * streams of media file. This example code is written with C++17 in mind.
 *
 * Licensed under MIT No Attribution
 *
 * Copyright (c) 2025 Miku AuahDark
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <cstdint>
#include <cstdio>
#include <memory>

#include "navia/navia.h"

const char *stream_type_name(navia_streamtype type)
{
	switch (type)
	{
		case NAVIA_STREAMTYPE_AUDIO:
			return "audio";
		case NAVIA_STREAMTYPE_VIDEO:
			return "video";
		case NAVIA_STREAMTYPE_UNKNOWN:
		default:
			return "unknown";
	}
}

const char *pixel_format_name(navia_pixelformat format)
{
	switch (format)
	{
		case NAVIA_PIXELFORMAT_RGB8:
			return "RGB8";
		case NAVIA_PIXELFORMAT_YUV420:
			return "YUV420";
		case NAVIA_PIXELFORMAT_YUV444:
			return "YUV444";
		case NAVIA_PIXELFORMAT_NV12:
			return "NV12";
		case NAVIA_PIXELFORMAT_UNKNOWN:
		default:
			return "unknown";
	}
}

void print_duration(double duration)
{
	if (duration < 0.0)
		printf("Duration: unknown\n");
	else
		printf("Duration: %.14g seconds\n", duration);
}

void print_audio_stream(const navia_streaminfo_t *sinfo)
{
	navia_audioformat format = navia_audio_format(sinfo);

	printf("  Sample rate: %u Hz\n", (unsigned) navia_audio_sample_rate(sinfo));
	printf("  Channels: %u\n", (unsigned) navia_audio_nchannels(sinfo));
	printf("  Sample frame size: %zu bytes\n", navia_audio_size(sinfo));
	printf("  Sample format: %u-bit %s %s, %s\n",
		(unsigned) NAVIA_AUDIOFORMAT_BITSIZE(format),
		NAVIA_AUDIOFORMAT_ISSIGNED(format) ? "signed" : "unsigned",
		NAVIA_AUDIOFORMAT_ISFLOAT(format) ? "float" : "integer",
		NAVIA_AUDIOFORMAT_ISBIGENDIAN(format) ? "big-endian" : "little-endian"
	);
}

void print_video_stream(const navia_streaminfo_t *sinfo)
{
	uint32_t width, height;
	navia_video_dimensions(sinfo, &width, &height);

	navia_pixelformat format = navia_video_pixel_format(sinfo);
	double fps = navia_video_fps(sinfo);

	printf("  Dimensions: %ux%u\n", (unsigned) width, (unsigned) height);
	if (fps <= 0.0)
		printf("  FPS: unknown\n");
	else
		printf("  FPS: %.3f\n", fps);
	printf("  Pixel format: %s\n", pixel_format_name(format));
	printf("  Frame size: %zu bytes\n", navia_video_size(sinfo));

	size_t nplanes = navia_video_plane_count(format);
	printf("  Planes: %zu\n", nplanes);
	for (size_t i = 0; i < nplanes; i++)
	{
		size_t planeWidth, planeHeight;
		navia_video_plane_dimensions(sinfo, i, &planeWidth, &planeHeight);
		printf("    Plane %zu: %zux%zu bytes\n", i, planeWidth, planeHeight);
	}
}

void print_stream(size_t index, const navia_streaminfo_t *sinfo)
{
	navia_streamtype type = navia_streaminfo_type(sinfo);

	printf("Stream #%zu: %s\n", index, stream_type_name(type));
	switch (type)
	{
		case NAVIA_STREAMTYPE_AUDIO:
			print_audio_stream(sinfo);
			break;
		case NAVIA_STREAMTYPE_VIDEO:
			print_video_stream(sinfo);
			break;
		case NAVIA_STREAMTYPE_UNKNOWN:
		default:
			printf("  No audio/video information available.\n");
			break;
	}
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		fprintf(stderr, "Usage: %s <input>\nInspect all audio and video streams of media file.\n", argv[0]);
		return 1;
	}

	/* We need to populate navia_input before we can open stream.
	 * navia_input is abstraction over input stream. This means
	 * the input data can come either from file, memory, or
	 * even streamed inside a Zip file.
	 *
	 * Either you can fill all the function yourself, or you can
	 * use convenience function such as:
	 * * `navia_input_populate_from_file` - Load from disk.
	 * * `navia_input_populate_from_memory` - Load from memory.
	 *
	 * Also you'd notice that we allocate `navia_input` on the stack.
	 * Yes, that's totally fine and intended way to use Navia.
	 */
	navia_input input;
	/* For this one, we just use `navia_input_populate_from_file` for convenience. */
	if (!navia_input_populate_from_file(&input, argv[1]))
	{
		/* On error, we retrieve the error message with `navia_error()`.
		 * Don't worry about data race. `navia_error()` error message
		 * is thread-local.
		 */
		fprintf(stderr, "Cannot populate input: %s\n", navia_error());
		return 1;
	}

	/* For this one, we use std::unique_ptr with `navia_close` as deleter to ease
	 * object management (RAII ftw).
	 *
	 * We call `navia_open` with the following arguments, in order:
	 * * `input` - Already-populated input from previous function.
	 * * `argv[1]` - Pseudo-filename of the input stream.
	 * * `nullptr` - Additional settings (e.g. disable hardware acceleration).
	 *               We pass `nullptr` which means "use defaults".
	 *
	 * When `navia_open` succeeded, it will take the ownership of the `input`.
	 * Assume the `input` has undefined contents in that case. This means
	 * it's totally fine to reuse the `input` variable to populate another input
	 * stream.
	 */
	std::unique_ptr<navia_t, decltype(&navia_close)> navia {navia_open(&input, argv[1], nullptr), navia_close};
	if (!navia)
	{
		fprintf(stderr, "Cannot open: %s\n", navia_error());
		/* However, if `navia_open` fails, the input ownership won't be taken.
		 * So we have to close it manually!
		 *
		 * When including `navia.h` in C++ translation unit, it provides convenience
		 * function `input.closef()` to close the input stream. However, if you're
		 * using C, you need to use `input.close(&input.userdata)` instead.
		 * Just keep that in mind.
		 */
		input.closef();
		return 1;
	}

	size_t backendIndex = navia_backend_index(navia.get());
	const char *backendName = navia_backend_name(backendIndex);

	printf("File: %s\n", argv[1]);
	printf("Navia version: %s\n", navia_version_string());
	printf("Backend: %s (#%zu)\n", backendName ? backendName : "unknown", backendIndex);
	print_duration(navia_duration(navia.get()));

	size_t streams = navia_nstreams(navia.get());
	printf("Streams: %zu\n", streams);
	for (size_t i = 0; i < streams; i++)
	{
		/* Now we retrieve information about each stream. */
		const navia_streaminfo_t *sinfo = navia_stream_info(navia.get(), i);
		if (!sinfo)
		{
			fprintf(stderr, "Cannot retrieve stream %zu information: %s\n", i, navia_error());
			return 1;
		}

		/* Print a blank line between stream entries. */
		printf("\n");
		print_stream(i, sinfo);
	}

	/* All is done. RAII will take care of the rest. */
	return 0;
}
