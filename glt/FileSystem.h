#pragma once
#include <vector>
#include <string>
#include "Export.h"
#include "Common.h"

namespace glt
{
	class GLT_DECLSPEC CFileSystem
	{
	public:
		~CFileSystem();
		_SINGLETON(CFileSystem);

		bool isDirectory(const std::string& vPathName);
		bool isRegularFile(const std::string& vFileName);
		bool isFileExisted(const std::string& vPathName);
		bool removeFile(const std::string& vFileName);
		bool removeDirectory(const std::string& vPathName);
		bool renameFile(const std::string& vFileName, const std::string& vNewFileName);
		bool renameDirectory(const std::string& vDirectory, const std::string& vNewDirectory);
		bool copyFile(const std::string& vFileName, const std::string& vTargetDirectory);

		std::string getCurrentPath();

		std::vector<std::string> getFileInDirectory(const std::string& vDirectory, const std::string& vExtension = "");

	protected:
		CFileSystem();
	};
}