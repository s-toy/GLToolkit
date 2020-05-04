#include "jsonUtil.h"
#include "Utility.h"
#include "FileLocator.h"

using namespace glt;

//*********************************************************************
//FUNCTION:
CJsonReader::CJsonReader(const std::string& vFilename)
{
	auto FileContent = readFileToString(CFileLocator::getInstance()->locateFile(vFilename));
	m_Doc.Parse<0>(FileContent.c_str());
	_ASSERTE(!m_Doc.HasParseError());
	_ASSERTE(!m_Doc.IsArray());
}

//*********************************************************************
//FUNCTION:
CJsonReader::~CJsonReader()
{
}

//**********************************************************************************************************
//FUNCTION:
std::string CJsonReader::readString(const std::string& vAttributName)
{
	const rapidjson::Value &p = m_Doc;
	_ASSERTE(p.HasMember(vAttributName.c_str()));
	return p[vAttributName.c_str()].GetString();
}

//**********************************************************************************************************
//FUNCTION:
int CJsonReader::readInt(const std::string& vAttributName)
{
	const rapidjson::Value &p = m_Doc;
	_ASSERTE(p.HasMember(vAttributName.c_str()));
	return p[vAttributName.c_str()].GetInt();
}

//**********************************************************************************************************
//FUNCTION:
float CJsonReader::readFloat(const std::string& vAttributName)
{
	const rapidjson::Value &p = m_Doc;
	_ASSERTE(p.HasMember(vAttributName.c_str()));
	return p[vAttributName.c_str()].GetFloat();
}

//**********************************************************************************************************
//FUNCTION:
bool CJsonReader::readBool(const std::string& vAttributName)
{
	const rapidjson::Value &p = m_Doc;
	_ASSERTE(p.HasMember(vAttributName.c_str()));
	return p[vAttributName.c_str()].GetBool();
}

//*********************************************************************
//FUNCTION:
CJsonWriter::CJsonWriter(const std::string& vFilename)
{
	m_FileName = vFilename;

	auto FileContent = readFileToString(CFileLocator::getInstance()->locateFile(vFilename));
	m_Doc.Parse<0>(FileContent.c_str());
	_ASSERTE(!m_Doc.HasParseError());
	_ASSERTE(!m_Doc.IsArray());
}

//*********************************************************************
//FUNCTION:
CJsonWriter::~CJsonWriter()
{
}

//**********************************************************************************************************
//FUNCTION:
void CJsonWriter::writeBool(const std::string& vAttributName, bool vValue)
{
	rapidjson::Value &p = m_Doc;
	_ASSERTE(p.HasMember(vAttributName.c_str()));
	p[vAttributName.c_str()].SetBool(vValue);
}

//**********************************************************************************************************
//FUNCTION:
void CJsonWriter::saveFile()
{
	rapidjson::StringBuffer Buffer;
	rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
	m_Doc.Accept(Writer);

	writeStringToFile(m_FileName, Buffer.GetString());
}