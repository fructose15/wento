/*
 * Wento Windows Configuration Header (config.h)
 * Creator: Fructose
 *
 * Pre-configured for Windows builds (MSVC cl.exe or MinGW-w64 g++).
 * Compile on Windows with -DWENTO_USE_WIN32_CONFIG
 */
#ifndef WENTO_CONFIG_H
#define WENTO_CONFIG_H

#define WENTO_VERSION "1.0.0"
#define WENTO_CREATOR "Fructose"
#define WENTO_PLATFORM_NAME "Windows"

#if defined(_WIN32) || defined(_WIN64) || defined(WENTO_USE_WIN32_CONFIG)
  #define WENTO_OS_WINDOWS 1
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
#else
  #define WENTO_OS_WINDOWS 0
#endif

/* Audio Synthesis Defaults */
#define WENTO_SAMPLE_RATE 44100
#define WENTO_CHANNELS 1
#define WENTO_BITS_PER_SAMPLE 16

/* Windows Audio & HTTPS Transport Settings */
#define WENTO_WIN_AUDIO_BACKEND "winmm"
#define WENTO_WIN_HTTP_BACKEND "winhttp"
#define WENTO_HAS_CURL 0
#define WENTO_PATH_SEP '\\'
#define WENTO_DEFAULT_REGISTRY_URL "https://wento-registry.local/api/notes"

/* MSVC Pragmas for Automatic System Library Linking */
#ifdef _MSC_VER
  #pragma comment(lib, "winmm.lib")
  #pragma comment(lib, "winhttp.lib")
#endif

#endif /* WENTO_CONFIG_H */
