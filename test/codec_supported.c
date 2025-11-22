#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/libbase64.h"

#ifdef _MSC_VER
#include <isa_availability.h>
#ifdef __cplusplus
extern "C" int __isa_available;
#else
extern int __isa_available;
#endif // __cplusplus
#endif // _MSC_VER

#if __has_builtin(__builtin_cpu_supports)
	#define CPU_SUPPORTS(x, msvc) __builtin_cpu_supports(x)
#elif _MSC_VER
	#define CPU_SUPPORTS(x, msvc) (__isa_available >= msvc)
#else
	#define CPU_SUPPORTS(x, msvc) 1
#endif // __has_builtin(__builtin_cpu_supports)

static int
cpu_has_x86_extension (const char *codec)
{
#if (__x86_64__ || __i386__ || _M_X86 || _M_X64)
	if (strcmp(codec, "SSSE3") == 0) {
		return CPU_SUPPORTS("ssse3", __ISA_AVAILABLE_SSE42);
	}
	if (strcmp(codec, "SSE41") == 0) {
		return CPU_SUPPORTS("sse4.1", __ISA_AVAILABLE_SSE42);
	}
	if (strcmp(codec, "SSE42") == 0) {
		return CPU_SUPPORTS("sse4.2", __ISA_AVAILABLE_SSE42);
	}
	if (strcmp(codec, "AVX") == 0) {
		return CPU_SUPPORTS("avx", __ISA_AVAILABLE_AVX);
	}
	if (strcmp(codec, "AVX2") == 0) {
		return CPU_SUPPORTS("avx2", __ISA_AVAILABLE_AVX2);
	}
	if (strcmp(codec, "AVX512") == 0) {
#if _MSC_VER
		return __isa_available >= __ISA_AVAILABLE_AVX512;
#else
		return CPU_SUPPORTS("avx512vl", 0) && CPU_SUPPORTS("avx512vbmi", 0);
#endif
	}
	return 1;
#else
	(void)codec;
	return 1;
#endif
}

static char *_codecs[] =
{ "AVX2"
, "NEON32"
, "NEON64"
, "plain"
, "SSSE3"
, "SSE41"
, "SSE42"
, "AVX"
, "AVX512"
, NULL
} ;

char **codecs = _codecs;

int
codec_supported (size_t index)
{
	if (index >= (sizeof(_codecs) / sizeof(_codecs[0])) - 1) {
		return 0;
	}

	// Early out if CPU extension is not present:
	if (!cpu_has_x86_extension(_codecs[index])) {
		return 0;
	}

	// Check if given codec is supported by trying to decode a test string:
	char *a = "aGVsbG8=";
	char b[10];
	size_t outlen;
	char envVariable[32];
	sprintf(envVariable, "BASE64_TEST_SKIP_%s", _codecs[index]);
	const char* envOverride = getenv(envVariable);
	if ((envOverride != NULL) && (strcmp(envOverride, "1") == 0)) {
		return 0;
	}
	int flags = 1 << index;
	return (base64_decode(a, strlen(a), b, &outlen, flags) != -1) ? flags : 0;
}
