#if (defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__))
	#if defined(GLT_EXPORTS)
		#define GLT_DECLSPEC __declspec(dllexport)
	#elif defined(GLT_STATIC_LIBRARY)
		#define GLT_DECLSPEC
	#else
		#define GLT_DECLSPEC __declspec(dllimport)
	#endif
#else
	#define GLT_DECLSPEC
#endif