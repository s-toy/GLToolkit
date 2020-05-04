#include "Scene.h"
#include "Model.h"
#include "JsonUtil.h"

using namespace glt;

//************************************************************
//FUNCTION:
void CScene::load(const std::string& vFilePath)
{
	CJsonReader JsonReader(vFilePath);
	const auto& Doc = JsonReader.getDocument();
	if (Doc.HasMember("models")) __parseModels(Doc["models"]);
}

//************************************************************
//FUNCTION:
const ModelGroup& CScene::getModelGroup(const std::string& vGroupName) const
{
	_ASSERTE(m_ModelGroupMap.find(vGroupName) != m_ModelGroupMap.end());
	return m_ModelGroupMap.at(vGroupName);
}

//************************************************************
//FUNCTION:
void CScene::__parseModels(const rapidjson::Value& vValue)
{
	for (auto iter = vValue.MemberBegin(); iter != vValue.MemberEnd(); ++iter)
	{
		std::string GroupName = iter->name.GetString();

		std::vector<std::shared_ptr<CModel>> Models;
		const auto& GroupItems = iter->value.GetArray();
		for (auto& Item : GroupItems)
		{
			_ASSERTE(Item.HasMember("filePath"));
			std::string FilePath = Item["filePath"].GetString();
			std::shared_ptr<CModel> pModel = std::make_shared<CModel>(FilePath);
			if (Item.HasMember("position"))
			{
				auto Array = Item["position"].GetArray();
				_ASSERTE(Array.Size() == 3);
				pModel->setPosition(glm::vec3(Array[0].GetFloat(), Array[1].GetFloat(), Array[2].GetFloat()));
			}
			if (Item.HasMember("scale"))
			{
				auto Array = Item["scale"].GetArray();
				_ASSERTE(Array.Size() == 3);
				pModel->setScale(glm::vec3(Array[0].GetFloat(), Array[1].GetFloat(), Array[2].GetFloat()));
			}
			if (Item.HasMember("rotation"))
			{
				const auto& v = Item["rotation"];
				_ASSERTE(v.HasMember("angle") && v.HasMember("axis"));
				float Angle = v["angle"].GetFloat();
				auto  Axis = v["axis"].GetArray();
				_ASSERTE(Axis.Size() == 3);
				pModel->setRotation(Angle, glm::vec3(Axis[0].GetFloat(), Axis[1].GetFloat(), Axis[2].GetFloat()));
			}
			if (Item.HasMember("parameters"))
			{
				auto Array = Item["parameters"].GetArray();
				_ASSERTE(Array.Size() == 4);
				pModel->setParameters(glm::vec4(Array[0].GetFloat(), Array[1].GetFloat(), Array[2].GetFloat(), Array[3].GetFloat()));
			}

			Models.push_back(pModel);
		}

		_ASSERTE(m_ModelGroupMap.find(GroupName) == m_ModelGroupMap.end());
		m_ModelGroupMap[GroupName] = Models;
	}
}