#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "JsonUtil.h"
#include "Export.h"

namespace glt
{
	class CModel;

	using ModelGroup = std::vector<std::shared_ptr<CModel>>;

	class GLT_DECLSPEC CScene
	{
	public:
		void load(const std::string& vFilePath);
		const ModelGroup& getModelGroup(const std::string& vGroupName) const;

	private:
		std::unordered_map<std::string, ModelGroup> m_ModelGroupMap;

		void __parseModels(const rapidjson::Value& vValue);
	};
}