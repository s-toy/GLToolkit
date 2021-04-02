#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
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
		const glm::vec3 getCameraPos() const { return m_CameraPos; }
		bool hasCameraPos() const { return m_HasCameraPos; }

	private:
		std::unordered_map<std::string, ModelGroup> m_ModelGroupMap;
		glm::vec3 m_CameraPos;
		bool m_HasCameraPos = false;

		void __parseModels(const rapidjson::Value& vValue);
	};
}