
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Root/Meshes/Meshes.h"

namespace Oak
{
	class ModelEntity : public SceneEntity
	{
	public:

		eastl::string meshPath;
		Mesh::Instance* mesh = nullptr;

		META_DATA_DECL_BASE(ModelEntity)

	#ifndef DOXYGEN_SKIP

		ModelEntity();
		virtual ~ModelEntity() = default;

		void Init() override;
		void ApplyProperties() override;
		void Update(float dt);
	#endif
	};
}