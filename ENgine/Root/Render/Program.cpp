
#include "Root/Root.h"

namespace Oak
{
	std::map<std::string, Math::Vector4> Program::vectors;
	std::map<std::string, Math::Matrix> Program::matrixes;
	std::map<std::string, Texture*> Program::textures;

	void Program::SetVector(const char* param, Math::Vector4* v)
	{
		Math::Vector4& vec = vectors[param];
		memcpy(&vec.x, &v->x, 4 * 4);
	}

	void Program::SetMatrix(const char* param, Math::Matrix* mat)
	{
		Math::Matrix& mt = matrixes[param];
		memcpy(&mt.m[0][0], &mat->m[0][0], 16 * 4);
	}

	void Program::SetTexture(const char* param, Texture* texture)
	{
		textures[param] = texture;
	}

	bool Program::Init()
	{
		if (GetVsName())
		{
			vshader = root.render.GetDevice()->CreateShader(Shader::Vertex, GetVsName());
		}

		if (GetPsName())
		{
			pshader = root.render.GetDevice()->CreateShader(Shader::Pixel, GetPsName());
		}

		return true;
	}

	void Program::SetVector(Shader::Type shader_type, const char* param, Math::Vector4* v, int count)
	{
		if (shader_type == Shader::Type::Vertex)
		{
			if (vshader) vshader->SetVector(param, v, count);
		}

		if (shader_type == Shader::Type::Pixel)
		{
			if (pshader) pshader->SetVector(param, v, count);
		}
	}

	void Program::SetMatrix(Shader::Type shader_type, const char* param, Math::Matrix* mat, int count)
	{
		if (shader_type == Shader::Type::Vertex)
		{
			if (vshader) vshader->SetMatrix(param, mat, count);
		}

		if (shader_type == Shader::Type::Pixel)
		{
			if (pshader) pshader->SetMatrix(param, mat, count);
		}
	}

	void Program::SetTexture(Shader::Type shader_type, const char* param, Texture* texture)
	{
		if (shader_type == Shader::Type::Vertex)
		{
			if (vshader) vshader->SetTexture(param, texture);
		}

		if (shader_type == Shader::Type::Pixel)
		{
			if (pshader) pshader->SetTexture(param, texture);
		}
	}
}

