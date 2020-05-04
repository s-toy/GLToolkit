#include "Utility.h"
#include <fstream>
#include <streambuf>

using namespace glt;

//*********************************************************************
//FUNCTION: https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
std::string glt::readFileToString(const std::string& vFilePath)
{
	std::ifstream File(vFilePath);
	_ASSERTE(File.is_open());
	std::string FileContent((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
	return FileContent;
}

//*********************************************************************
//FUNCTION:
void glt::writeStringToFile(const std::string& vFilePath, const std::string& vContent)
{
	std::ofstream File(vFilePath);
	_ASSERTE(File.is_open());
	File << vContent;
}