#ifndef _NAVIA_ATTRIBUTES_H_
#define _NAVIA_ATTRIBUTES_H_

#if (defined(__cplusplus) && __cplusplus >= 201703L) || \
    (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || \
	(defined(__STDC_VERSION__ ) && __STDC_VERSION__ >= 202311L)
#	define NAVIA_NODISCARD [[nodiscard]]
#	define NAVIA_DEPRECATED [[deprecated]]
#elif defined(__GNUC__) || defined(__clang__)
#	define NAVIA_NODISCARD __attribute__((warn_unused_result))
#	define NAVIA_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#	include <sal.h>
#	define NAVIA_NODISCARD _Check_return_
#	define NAVIA_DEPRECATED __declspec(deprecated)
#else
#	define NAVIA_NODISCARD
#	define NAVIA_DEPRECATED
#endif

#endif /* _NAVIA_ATTRIBUTES_H_ */
