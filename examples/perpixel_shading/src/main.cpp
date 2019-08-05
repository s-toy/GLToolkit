#include "MyApplication.h"

int main()
{
//#ifdef _DEBUG
//	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
//#endif
//
//#ifdef _DEBUG
//	GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
//	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
//	{
//		glEnable(GL_DEBUG_OUTPUT);
//		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//		glDebugMessageCallback(glDebugOutput, nullptr);
//		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
//	}
//#endif // DEBUG

	CMyApplication App;
	App.run();

	return 0;
}