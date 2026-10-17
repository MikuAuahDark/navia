#ifndef _NAVIA_FFMPEG_COMMON_HPP_
#define _NAVIA_FFMPEG_COMMON_HPP_

#if defined(_WIN32)
#	define _NAVIA_FFMPEG_LIB_NAME(libname, ver) libname "-" NAVIA_STRINGIZE(ver) ".dll"
#elif defined(__ANDROID__)
#	define _NAVIA_FFMPEG_LIB_NAME(libname, ver) "lib" libname ".so"
#else
#	define _NAVIA_FFMPEG_LIB_NAME(libname, ver) "lib" libname ".so." NAVIA_STRINGIZE(ver)
#endif

namespace navia::ffmpeg_common
{

template<typename T>
struct DoublePointerDeleter
{
	inline void operator()(T *ptr)
	{
		if (ptr)
		{
			T *temp = ptr;
			deleter(&temp);
		}
	}

	void(*deleter)(T**);
};

inline double derationalize(const AVRational &r, double dv0 = 0.0)
{
	return navia::derationalize(r.num, r.den, dv0);
}

template<typename T>
double derationalize(T multipler, const AVRational &r, double dv0 = 0.0)
{
	return navia::derationalize(multipler * r.num, (T) r.den, dv0);
}

}

#endif /* _NAVIA_FFMPEG_COMMON_HPP_ */
