
#pragma once

#include "Root/Render/Render.h"

namespace Oak
{
	class DebugLines : public Object
	{
		enum
		{
			MaxSize = 10000
		};

		struct Vertex
		{
			Math::Vector3 p;
			uint32_t color;

			Vertex(Math::Vector3 set_p, uint32_t set_color)
			{
				p = set_p;
				color = set_color;
			};
		};

		std::vector<Vertex> lines;
		std::vector<Vertex> lines_with_depth;
		std::vector<Vertex> lines_2d;

		Program*    prg;
		Program*    prg_depth;
		VertexDecl* vdecl;
		DataBuffer* buffer;

	public:

		void Init(TaskExecutor::SingleTaskPool* debugTaskPool);
		void AddLine(Math::Vector3 from, Color from_clr, Math::Vector3 to, Color to_clr, bool use_depth);
		void AddLine2D(Math::Vector2 from, Color from_clr, Math::Vector2 to, Color to_clr);
		void DrawCircle(int axis, Math::Vector3 pos, Color color, float radius);
		void DrawLines(Program* prog, std::vector<Vertex>& lines, bool is2d);
		void Draw(float dt);
		void Release();
	};
}