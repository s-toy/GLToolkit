#pragma once
#include <vector>
#include <memory>
#include <any>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Common.h"

namespace glt
{
	typedef struct
	{
		glm::vec3  Position;
		glm::vec3  Normal;
		glm::vec2  TexCoords;
		glm::ivec4 BoneIDs;
		glm::vec4  BoneWeights;
	} SVertex;

	typedef struct
	{
		EUniformType Type;
		std::string Name;
		std::any Value;
	} SUniformInfo;

	struct SAABB
	{
		glm::vec3 Min;
		glm::vec3 Max;
	};

	class CShaderProgram;
	class CTexture2D;

	class CMesh
	{
	public:
		CMesh(const std::vector<SVertex>& vVertices, const std::vector<unsigned int>& vIndices, const std::vector<std::shared_ptr<CTexture2D>>& vTextures,
			const std::vector<SUniformInfo>& vUniforms, const SAABB& vAABB);

		const SAABB& getAABB() const { return m_AABB; }

	protected:
		void _draw(const CShaderProgram& vShaderProgram) const;

	private:
		void __setupMesh();

	private:
		std::vector<SVertex> m_Vertices;
		std::vector<GLuint> m_Indices;
		std::vector<std::shared_ptr<CTexture2D>> m_Textures;
		std::vector<SUniformInfo> m_Uniforms;

		std::shared_ptr<CVertexBuffer>	m_pVertexBuffer;
		std::shared_ptr<CIndexBuffer>	m_pIndexBuffer;
		std::shared_ptr<CVertexArray>	m_pVertexArray;

		SAABB m_AABB;

		friend class CModel;
	};
}