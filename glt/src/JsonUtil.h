#pragma once
#undef max 
#undef min

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "math.h"

namespace glt
{
	class CJsonReader
	{
	public:
		using TObject = rapidjson::GenericObject<true, rapidjson::Value>;

		CJsonReader(const std::string& vFilename);
		~CJsonReader();

		std::string		readString(const std::string& vAttributName);
		int				readInt(const std::string& vAttributName);
		float			readFloat(const std::string& vAttributName);
		bool			readBool(const std::string& vAttributName);

		const rapidjson::Document& getDocument() { return m_Doc; }

	private:
		rapidjson::Document m_Doc;
	};

	class CJsonWriter
	{
	public:
		CJsonWriter(const std::string& vFilename);
		~CJsonWriter();

		void writeBool(const std::string& vAttributName, bool vValue);

		void saveFile();

	private:
		std::string m_FileName;
		rapidjson::Document m_Doc;
	};
}