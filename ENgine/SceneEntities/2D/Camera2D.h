
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"

namespace Oak
{
	class Camera2D : public SceneEntity
	{
		Math::Matrix view;

	public:

		META_DATA_DECL_BASE(Camera3D)

		float zoom = 1.0f;
		SceneEntityRef targetRef;

		Transform transform;

#ifndef DOXYGEN_SKIP

		Camera2D();
		virtual ~Camera2D() = default;

		Transform* GetTransform() override;

		void Init() override;
		void Update(float dt);

#endif
	};
}
