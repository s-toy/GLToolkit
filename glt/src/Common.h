#pragma once
#include <string>
#include <iostream>

#if (defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__))
#	define _WINDOWS
#endif

#define _OUTPUT_EVENT(e)			std::cout << e << std::endl;
#define _OUTPUT_WARNING(e)			std::cerr << e << std::endl;
#define _THROW_RUNTIME_ERROR(e)		throw std::runtime_error(e);

#define _SAFE_DELETE(p)									if(p) { delete p; p = nullptr; }
#define _EARLY_RETURN(condition, prompt, return_value)	if (condition) { _OUTPUT_WARNING(prompt); return return_value; }
#define _EARLY_EXIT(condition, prompt)					if (condition) { _OUTPUT_WARNING(prompt); return;}
#define _SIMPLE_IF(condition, opTrue)					if (condition) { opTrue; }
#define _SIMPLE_IF_ELSE(condition, opTrue, opFalse)		if (condition) { opTrue; } else { opFalse; }

#define _STR_TO_UPPER(s)		std::transform((s).begin(), (s).end(), (s).begin(), ::toupper);

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
	struct SVector2
	{
		int x = 0, y = 0;
	};

	struct SRect
	{
		int x = 0, y = 0, w = 0, h = 0;
	};

	struct SWindowInfo
	{
		SWindowInfo() {}
		SWindowInfo(uint32_t vWidth, uint32_t vHeight, const std::string& vTitle) :Width(vWidth), Height(vHeight), Title(vTitle) {}

		std::string Title = "GL APPLICATION";

		uint32_t MonitorID = 0;
		uint32_t PosX = 100;
		uint32_t PosY = 100;
		uint32_t Width = 800;
		uint32_t Height = 600;

		bool IsFullScreen = false;
		bool IsResizable = false;

		bool operator==(const SWindowInfo& r) const
		{
			return Title == r.Title && MonitorID == r.MonitorID && PosX == r.PosX && PosY == r.PosY
				&& Width == r.Width && Height == r.Height && IsFullScreen == r.IsFullScreen && IsResizable == r.IsResizable;
		}
	};

	enum class EUniformType : unsigned char 
	{
		FLOAT = 0,
		VEC2F,
		VEC3F,
		VEC4F
	};

	template <class ... Args>
	static std::string format(const char *vFormat, Args... args)
	{
		if (nullptr == vFormat) return std::string{};

		int n = snprintf(nullptr, 0, vFormat, args...);
		if (0 == n) return std::string{};

		std::string Ret(n, '\0');
		snprintf(&*Ret.begin(), n + 1, vFormat, args...);

		return Ret;
	}
}