/* Process this file with configure to produce config.h. -*- mode: c -*- */
#ifndef CLICK_CONFIG_H
#define CLICK_CONFIG_H

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
# undef __CHAR_UNSIGNED__
#endif

/* Define to byte order of target machine. */
#undef CLICK_BYTE_ORDER
#define CLICK_BIG_ENDIAN	4321
#define CLICK_LITTLE_ENDIAN	1234
#define CLICK_NO_ENDIAN		0

#include <click/config-default.h>

/* Define inline, if necessary. C only. */
#ifndef __cplusplus
#undef inline
#endif

/* Define constexpr to const under C or old C++. */
#if !defined(__cplusplus) || !HAVE_CXX_CONSTEXPR
# define constexpr const
#endif

/* Define CLICK_DEBUG_SCHEDULING to 0 if disabled. */
#ifndef CLICK_DEBUG_SCHEDULING
# define CLICK_DEBUG_SCHEDULING 0
#endif

/* Define macro for creating Click version codes (a la Linux version codes). */
#define CLICK_MAKE_VERSION_CODE(major, minor, patch) \
		(((major) << 16) | ((minor) << 8) | (patch))

/* Define macro for aligning variables. */
#if __GNUC__
# define CLICK_ALIGNED(x) __attribute__((aligned(x)))
#else
# define CLICK_ALIGNED(x) /* nothing */
#endif

/* Define macro for size of a cache line. */
#define CLICK_CACHE_LINE_SIZE 64

/* Define macro for the difference between 'x' and the next higher multiple
   of CLICK_CACHE_LINE_SIZE (between 0 and CLICK_CACHE_LINE_SIZE - 1). */
#define CLICK_CACHE_LINE_PAD_BYTES(x) \
	((((x) + CLICK_CACHE_LINE_SIZE - 1) / CLICK_CACHE_LINE_SIZE) * CLICK_CACHE_LINE_SIZE - (x))

/* Define macro for deprecated functions. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0)
# define CLICK_DEPRECATED /* nothing */
#else
# define CLICK_DEPRECATED __attribute__((deprecated))
#endif

/* Define macro for deprecated enumerations. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
# define CLICK_DEPRECATED_ENUM /* nothing */
#else
# define CLICK_DEPRECATED_ENUM __attribute__((deprecated))
#endif

/* Define macros for marking types as may-alias. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
# define CLICK_MAY_ALIAS /* nothing */
#else
# define CLICK_MAY_ALIAS __attribute__((__may_alias__))
#endif

/* Define macro for marking functions noninlinable. */
#ifdef CLICK_LINUXMODULE
# define CLICK_NOINLINE noinline
#elif __GNUC__
# define CLICK_NOINLINE __attribute__((noinline))
#else
# define CLICK_NOINLINE /* nothing */
#endif

/* Define macros for declaring packed structures. */
#ifdef __GNUC__
# define CLICK_PACKED_STRUCTURE(open, close) open close __attribute__((packed))
# define CLICK_SIZE_PACKED_STRUCTURE(open, close) open close __attribute__((packed)) /* deprecated */
# define CLICK_SIZE_PACKED_ATTRIBUTE __attribute__((packed))
#else
# define CLICK_PACKED_STRUCTURE(open, close) _Cannot_pack_structure__Use_GCC
# define CLICK_SIZE_PACKED_STRUCTURE(open, close) open close /* deprecated */
# define CLICK_SIZE_PACKED_ATTRIBUTE
#endif

/* Define macro for functions whose results should not be ignored. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
# define CLICK_WARN_UNUSED_RESULT /* nothing */
#else
# define CLICK_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

/* Define macro for cold (rarely used) functions. */
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3)
# define CLICK_COLD /* nothing */
#else
# define CLICK_COLD __attribute__((cold))
#endif

/* Define ARCH_IS_BIG_ENDIAN based on CLICK_BYTE_ORDER. */
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
# define ARCH_IS_BIG_ENDIAN	1
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
# define ARCH_IS_BIG_ENDIAN	0
#endif

/* Define macro for htons() on constants (allows htons() in switch cases). */
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
# define click_constant_htons(x)	(x)
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
# define click_constant_htons(x)	((((x) >> 8) & 255) | (((x) & 255) << 8))
#endif

/* EXPORT_ELEMENT, ELEMENT_REQUIRES, ELEMENT_PROVIDES, ELEMENT_HEADER,
   ELEMENT_LIBS, and ELEMENT_MT_SAFE are noops. */
#define EXPORT_ELEMENT(x)
#define ELEMENT_REQUIRES(x)
#define ELEMENT_PROVIDES(x)
#define ELEMENT_HEADER(x)
#define ELEMENT_LIBS(x)
#define ELEMENT_MT_SAFE(x)

/* Assume CLICK_USERLEVEL unless otherwise defined. */
#if !defined(CLICK_USERLEVEL) && !defined(CLICK_TOOL) && !defined(CLICK_LINUXMODULE) && !defined(CLICK_BSDMODULE)
# define CLICK_USERLEVEL 1
#endif

/* Define stuff under a Linux module. */
#ifdef CLICK_LINUXMODULE
# include <click/config-linuxmodule.h>
#endif

/* Define stuff under a FreeBSD module. */
#ifdef CLICK_BSDMODULE
# include <click/config-bsdmodule.h>
#endif

/* Define stuff under nsclick. */
#ifdef CLICK_NS
# include <click/config-ns.h>
#endif

/* Define stuff under tools or a user-level driver. */
#if defined(CLICK_USERLEVEL) || defined(CLICK_TOOL)
# include <click/config-userlevel.h>
#endif

/* Ensure declaration of DefaultArg template. */
#ifdef __cplusplus
CLICK_DECLS
template <typename T> struct DefaultArg;

/** @class uninitialized_type
    @brief Type tag indicating an object should not be initialized. */
struct uninitialized_type {
};
CLICK_ENDDECLS
#endif

/* Define aliasing versions of integer and pointer types. */
typedef uint16_t click_aliasable_uint16_t CLICK_MAY_ALIAS;
typedef int16_t click_aliasable_int16_t CLICK_MAY_ALIAS;
typedef uint32_t click_aliasable_uint32_t CLICK_MAY_ALIAS;
typedef int32_t click_aliasable_int32_t CLICK_MAY_ALIAS;
#if HAVE_INT64_TYPES
typedef uint64_t click_aliasable_uint64_t CLICK_MAY_ALIAS;
typedef int64_t click_aliasable_int64_t CLICK_MAY_ALIAS;
#endif
typedef void *click_aliasable_void_pointer_t CLICK_MAY_ALIAS;

#endif /* CLICK_CONFIG_H */
