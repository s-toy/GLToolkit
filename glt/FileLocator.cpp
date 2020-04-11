#include "FileLocator.h"
#include "FileSystem.h"

using namespace glt;

CFileLocator::CFileLocator()
{
}

CFileLocator::~CFileLocator()
{
}

//********************************************************************************************************
//FUNCTION: locate the specified media file and return the full path of the media file. Empty string is returned if the media
//          file cannot be found
//NOTE:     the returned full path has been converted to upper case
std::string CFileLocator::locateFile(const std::string& vFileName)
{
	if (vFileName.empty()) return std::string("");

	std::string MediaFile = "";

	if (CFileSystem::getInstance()->isFileExisted(vFileName))
	{
		MediaFile = vFileName;
	}
	else
	{
		if (CFileSystem::getInstance()->isFileExisted(std::string("..//") + vFileName))
		{
			MediaFile = std::string("..//") + vFileName;
		}
		else
		{
			std::string MediaPath;
			for (auto Path : m_SearchPathSet)
			{
				MediaPath = Path + std::string("\\");
				if (CFileSystem::getInstance()->isFileExisted(MediaPath + vFileName))
				{
					MediaFile = MediaPath + vFileName;
					break;
				}
			}
		}
	}
	return MediaFile;
}

//*********************************************************************************
//FUNCTION:
void CFileLocator::addFileSearchPath(const std::string& vPath)
{
	std::scoped_lock Lock(m_Mutex);
	if (CFileSystem::getInstance()->isFileExisted(vPath)) m_SearchPathSet.insert(vPath);
}

//*********************************************************************************
//FUNCTION:
void CFileLocator::addFileSearchPath(const std::vector<std::string>& vPathSet)
{
	for (auto Path : vPathSet) addFileSearchPath(Path);
}