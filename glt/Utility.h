#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace glt
{
	template <class T>
	static bool readFile(const std::string& vFilename, T& voBuffer)
	{
		std::ifstream Fin(vFilename, std::ios::ate | std::ios::binary);
		if (!Fin.is_open()) return false;

		size_t FileSize = (size_t)Fin.tellg();

		using value_type = typename T::value_type;
		size_t ValueSize = sizeof(value_type);
		size_t BufferSize = (FileSize + ValueSize - 1) / ValueSize;
		voBuffer.resize(BufferSize);

		Fin.seekg(0);
		Fin.read(reinterpret_cast<char*>(voBuffer.data()), FileSize);
		Fin.close();

		return true;
	}

	static inline void ltrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
			}));
	}

	// trim from end (in place)
	static inline void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
			}).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string& s) {
		ltrim(s);
		rtrim(s);
	}
}