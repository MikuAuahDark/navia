/**
 * This is an example code on how to use Navia to convert first audio stream
 * of media file to WAV. This example code is written with C++17 in mind.
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

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <system_error>

#include "navia/navia.h"

/* A struct containing "fmt " chunk data for WAV. */
struct WAVFormat
{
	uint16_t formatTag;
	uint16_t channels;
	uint32_t samplesPerSec;
	uint32_t avgBytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
};
/* Ensure the size is 16 bytes */
static_assert(sizeof(WAVFormat) == 16, "WAV format size is not 16");

/* A convenience class for writing WAV audio file.
 * We declare it as `struct` to make all the members public
 * for convenience.
 */
struct WAVWriter
{
	WAVWriter(const char *output, const navia_streaminfo_t *sinfo)
	: f(fopen(output, "wb"))
	, written_total(0)
	{
		if (!f)
			throw std::system_error(errno, std::generic_category());

		write_header(sinfo);
	}

	~WAVWriter()
	{
		fclose(f);
	}

	void write(const void *buf, size_t size)
	{
		size_t w = fwrite(buf, 1, size, f);
		if (w != size)
			throw std::system_error(errno, std::generic_category());

		written_total += w;
	}

	void write_header(const navia_streaminfo_t *sinfo)
	{
		// Write header
		write("RIFF\0\0\0\0WAVEfmt \x10\0\0\0", 20);

		// Pull the format integer value.
		navia_audioformat format = navia_audio_format(sinfo);
		WAVFormat wavformat = {
			// We either use IEEE-float (format 3) or PCM (format 1).
			uint16_t(NAVIA_AUDIOFORMAT_ISFLOAT(format) ? 3 : 1),
			// Get number of channels
			(uint16_t) navia_audio_nchannels(sinfo),
			// Get sample rate
			navia_audio_sample_rate(sinfo),
			// Bytes per second (changed later)
			0,
			// Byte size alignment per sample
			(uint16_t) NAVIA_AUDIOFORMAT_BYTESIZE(format),
			// Bits per sample
			(uint16_t) NAVIA_AUDIOFORMAT_BITSIZE(format)
		};
		wavformat.avgBytesPerSec = wavformat.samplesPerSec * wavformat.channels * wavformat.blockAlign;
		// Write "fmt " chunk
		write(&wavformat, sizeof(WAVFormat));

		// Write "data" chunk and its dummy size.
		write("data\0\0\0\0", 8);
	}

	void finalize()
	{
		uint32_t riffsize = uint32_t(written_total - 8); // RIFF header + size
		uint32_t datasize = uint32_t(written_total - 44); // Everything up to "data" chunk and its size.

		fseek(f, 4, SEEK_SET);
		fwrite(&riffsize, 1, 4, f);
		fseek(f, 40, SEEK_SET);
		fwrite(&datasize, 1, 4, f);
	}

	FILE *f;
	size_t written_total;
};

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		fprintf(stderr, "Usage: %s <input> <output.wav>\nLoads first found audio stream and convert it to WAV.\n", argv[0]);
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

	/* Holds the stream info of the target audio stream. */
	const navia_streaminfo_t *audioStreamInfo = nullptr;
	/* Now we iterate list of audio/video streams in the file. */
	for (size_t i = 0; i < navia_nstreams(navia.get()); i++)
	{
		/* Now we retrieve information about each stream. */
		const navia_streaminfo_t *sinfo = navia_stream_info(navia.get(), i);

		/* If we previously have one audio stream activated,
		 * deactivate the rest of the streams. Otherwise audio
		 * stream from other stream indices (in case of multiple
		 * audio streams) will also enter the WAV, wrecking havoc.
		 */
		if (audioStreamInfo)
		{
			if (!navia_stream_enable(navia.get(), i, false))
			{
				/* This means it fails to disable the stream. */
				fprintf(stderr, "Cannot disable stream %zu: %s\n", i, navia_error());
				/* Because we're using RAII, we don't have to call `navia_close()`
				 * ourselves. std::unique_ptr destructor will do it.
				 */
				return 1;
			}
		}

		if (navia_streaminfo_type(sinfo) == NAVIA_STREAMTYPE_AUDIO)
		{
			/* We got audio stream. */
			audioStreamInfo = sinfo;
			/* Enable this stream. Although Navia defaults to enabling
			 * all streams, it won't hurt enabling it again.
			 */
			if (!navia_stream_enable(navia.get(), i, true))
			{
				/* This means it fails to enable the stream. */
				fprintf(stderr, "Cannot enable stream %zu: %s\n", i, navia_error());
				/* Again, RAII will call `navia_close()`. */
				return 1;
			}
		}
		else
		{
			/* This is a video stream. Disable it. */
			if (!navia_stream_enable(navia.get(), i, false))
			{
				fprintf(stderr, "Cannot disable stream %zu: %s\n", i, navia_error());
				return 1;
			}
		}
	}

	if (!audioStreamInfo)
	{
		/* No audio stream found. */
		fprintf(stderr, "File has no audio stream.\n");
		return 1;
	}

	/* Initialize our WAV writer. */
	WAVWriter wavwriter(argv[2], audioStreamInfo);

	/* Now the most exciting part, reading the frames. */
	while (navia_frame_t *frame = navia_read(navia.get()))
	{
		/* Just for RAII purpose. */
		std::unique_ptr<navia_frame_t, decltype(&navia_frame_free)> frame_uq {frame, navia_frame_free};

		/* Now we "acquire" the frame data.
		 * * For video, this will give you 1 decoded video picture
		 *   with multiple planes depending on the video pixel format.
		 * * For audio, this will give you interleaved audio data
		 *   span across `*strides` bytes.
		 *
		 * Since we're dealing with audio data, we can assume there's only 1 plane.
		 */
		ptrdiff_t *strides;
		if (const uint8_t *const *audiodata = navia_frame_acquire(frame, &strides, nullptr))
			/* Write the audio data. */
			wavwriter.write(audiodata[0], *strides);
		else
		{
			/* This means it fails to acquire frame. */
			fprintf(stderr, "Cannot acquire frame: %s\n", navia_error());
			/* RAII will close the frame and the Navia instance. */
			return 1;
		}

		/* Depending on your case, you may or may not need to call `navia_frame_release()`.
		 * In this example code, we don't have to. RAII will call `navia_frame_free()`.
		 * `navia_frame_free()` will also release the acquired frame pointer.
		 *
		 * Do note that once frame is released, the `audiodata` pointer will no longer valid.
		 * Also do note that "release" is not same as "free". "Released" frame can be
		 * re-acquired (the `navia_frame_t*` is still valid), whereas "free"d frame is
		 * no longer points to valid `navia_frame_t*`!
		 */
	}
	/* `navia_read` will return null pointer either if:
	 * * There are no more frames (which is expected behavior)
	 * * There's an error reading the frame (which is unexpected).
	 *
	 * To distinguish between them, we call `navia_error()`. If it returns
	 * null pointer then it means there are no more frames.
	 * */
	if (const char *err = navia_error())
	{
		/* In this case, there's an error reading the frame. */
		fprintf(stderr, "Cannot read frame: %s\n", err);
		return 1;
	}

	wavwriter.finalize();
	/* All is done. RAII will take care of the rest. */
	return 0;
}
