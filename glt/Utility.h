#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/matrix4x4.h>

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

	static inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
	{
		glm::mat4 to;
		to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
		to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
		to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
		to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
		return to;
	}
}