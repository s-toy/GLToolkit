#include "FileSystem.h"
#include <filesystem>
#include <algorithm>
#include <cctype>
#include "FileLocator.h"

using namespace glt;

CFileSystem::CFileSystem()
{
}

CFileSystem::~CFileSystem()
{
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::isDirectory(const std::string& vPathName)
{
	return std::filesystem::is_directory(std::filesystem::path(vPathName));
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::isRegularFile(const std::string& vFileName)
{
	return std::filesystem::is_regular_file(std::filesystem::path(vFileName));
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::isFileExisted(const std::string& vPathName)
{
	return std::filesystem::exists(std::filesystem::path(vPathName));
}

//*********************************************************************************
//FUNCTION:
std::string CFileSystem::getCurrentPath()
{
	return std::filesystem::current_path().string();
}

//*********************************************************************************
//FUNCTION:
std::vector<std::string> CFileSystem::getFileInDirectory(const std::string& vDirectory, const std::string& vExtension)
{
	_ASSERTE(!isDirectory(vDirectory));

	std::string UpperExtension = vExtension;
	_STR_TO_UPPER(UpperExtension);

	if (!UpperExtension.empty() && UpperExtension[0] != '.') UpperExtension = "." + UpperExtension;

	std::filesystem::path PathName(vDirectory);
	std::vector<std::string> FileNameSet;
	for (auto& e : std::filesystem::recursive_directory_iterator(PathName))
	{
		std::filesystem::path TempPath = e.path();

		_SIMPLE_IF(!std::filesystem::is_regular_file(TempPath), continue);

		std::string FileExtension = TempPath.extension().generic_string();
		_STR_TO_UPPER(FileExtension);

		if (vExtension.empty() || FileExtension == UpperExtension) FileNameSet.push_back(TempPath.string());
	}

	return FileNameSet;
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::removeFile(const std::string& vFileName)
{
	_SIMPLE_IF(!isRegularFile(vFileName), return false);

	try
	{
		std::filesystem::remove(std::filesystem::path(vFileName));
		return true;
	}
	catch (...)
	{
		return false;
	}
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::removeDirectory(const std::string& vPathName)
{
	_SIMPLE_IF(!isDirectory(vPathName), return false);

	try
	{
		std::filesystem::remove_all(std::filesystem::path(vPathName));
		return true;
	}
	catch (...)
	{
		return false;
	}
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::renameFile(const std::string& vFileName, const std::string& vNewFileName)
{
	_SIMPLE_IF(!isRegularFile(vFileName), return false);

	try
	{
		std::filesystem::rename(std::filesystem::path(vFileName), std::filesystem::path(vNewFileName));
		return true;
	}
	catch (...)
	{
		return false;
	}
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::copyFile(const std::string& vFileName, const std::string& vTargetDirectory)
{
	std::string FileName = CFileLocator::getInstance()->locateFile(vFileName);
	_SIMPLE_IF(FileName.empty(), return false);
	_SIMPLE_IF((!isRegularFile(FileName) || !isDirectory(vTargetDirectory)), return false);

	try
	{
		std::string TargetFileName = vTargetDirectory + "\\" + vFileName;
		std::filesystem::copy_file(std::filesystem::path(FileName), std::filesystem::path(TargetFileName), std::filesystem::copy_options::overwrite_existing);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

//*********************************************************************************
//FUNCTION:
bool CFileSystem::renameDirectory(const std::string& vDirectory, const std::string& vNewDirectory)
{
	_SIMPLE_IF(!isDirectory(vDirectory), return false);

	try
	{
		std::filesystem::rename(std::filesystem::path(vDirectory), std::filesystem::path(vNewDirectory));
		return true;
	}
	catch (...)
	{
		return false;
	}
}