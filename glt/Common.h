#pragma once

#if (defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__))
#	define _WINDOWS
#endif

#define _OUTPUT_EVENT(e)			std::cout << e << std::endl;
#define _OUTPUT_WARNING(e)			std::cerr << e << std::endl;
#define _THROW_RUNTIME_ERROR(e)		throw std::runtime_error(e);

#define _DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    TypeName &operator =(const TypeName &) = delete;

#define _SINGLETON(TypeName) static TypeName* getInstance() { static TypeName Instance; return &Instance; } 

#define _CALLBACK_0(Selector, Target, ...) std::bind(&Selector, Target, ##__VA_ARGS__)
#define _CALLBACK_1(Selector, Target, ...) std::bind(&Selector, Target, std::placeholders::_1, ##__VA_ARGS__)
#define _CALLBACK_2(Selector, Target, ...) std::bind(&Selector, Target, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)
#define _CALLBACK_3(Selector, Target, ...) std::bind(&Selector, Target, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)
#define _CALLBACK_4(Selector, Target, ...) std::bind(&Selector, Target, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, ##__VA_ARGS__)

namespace glt
{
	struct SWindowCreateInfo
	{
		int WindowWidth = 0, WindowHeight = 0;
		int WindowPosX = 0, WindowPosY = 0;
		std::string WindowTitle = "";
		bool IsWindowFullScreen = false;
		bool IsWindowResizable = false;

		bool isValid() const { return (WindowWidth > 0 && WindowHeight > 0); }	//TODO:
	};
}