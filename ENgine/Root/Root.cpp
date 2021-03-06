
#include "Root.h"
#include "Support/Timer.h"
#include "Support/StringUtils.h"
#include "Support/Sprite.h"

#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stb_sprintf.h"

#ifdef PLATFORM_WIN
#include <windows.h>
#endif

#ifdef OAK_EDITOR
#include "Editor/Editor.h"
#include <filesystem>
#endif

namespace Oak
{
	Root root;

	Root::Root()
	{
		logsDir[0] = 0;
	}

	bool Root::Init(void* renderData)
	{
		srand((unsigned int)time(nullptr));

		#ifdef PLATFORM_WIN
		char curDir[1024];
		GetCurrentDirectoryA(1024, curDir);
		StringUtils::Printf(logsDir, 1024, "%s/Logs", curDir);
		CreateDirectoryA(logsDir, nullptr);

		time_t t = time(0);

		for (auto& path : std::filesystem::directory_iterator(logsDir))
		{
			std::filesystem::remove_all(path);
		}

		#endif

		if (!files.Init())
		{
			return false;
		}

		if (!controls.Init("ENgine/Controls/hardware_win", true))
		{
			return false;
		}

		if (!controls.LoadAliases("ENgine/Controls/engine_aliases"))
		{
			OAK_ALERT("controls.LoadAliases(\"ENgine/Controls/engine_aliases\") failed");

			return false;
		}

		if (!render.Init("DX11", renderData))
		{
			return false;
		}

		if (!fonts.Init())
		{
			return false;
		}

		Sprite::Init();

		particles.Init();

		sounds.Init();

		assets.Init();

		physics.Init();

		scenes.Init();

		StringUtils::Init();

		Timer::CountDeltaTime();

		return true;
	}

	void Root::Log(const char* name, const char* text, ...)
	{
		static char buffer[4096];

		int timeStampLen = (int)strlen(Timer::GetTimeStamp());
		memcpy(buffer, Timer::GetTimeStamp(), timeStampLen);
		buffer[timeStampLen];

		va_list args;
		va_start(args, text);

		stbsp_vsnprintf(&buffer[timeStampLen], sizeof(buffer) - 4 - timeStampLen, text, args);

		va_end(args);

		#ifdef PLATFORM_WIN
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");

		char path[1024];
		StringUtils::Printf(path, 1024, "%s/%s.txt", logsDir, name);

#ifdef OAK_EDITOR
		if (redirectLog)
		{
			editor.CaptureLog(name, buffer);
		}
#endif

		FILE* f = nullptr;
		fopen_s(&f, path, "a");

		if (f)
		{
			fwrite(buffer, strlen(buffer), 1, f);
			fwrite("\n", strlen("\n"), 1, f);
			fclose(f);
		}
		#endif
	}

	void Root::CountDeltaTime()
	{
		Timer::CountDeltaTime();
		dt = Timer::GetDeltaTime();
	}

	void Root::Update()
	{
		assets.Update();

		controls.Update(dt);

		scenes.Execute(dt);

		scripts.Update();

		physics.Update(dt);

		render.Execute(dt);

		sounds.Update(dt);

		physics.Fetch();

	}

	float Root::GetDeltaTime()
	{
		return dt;
	}

	int Root::GetFPS()
	{
		return Timer::GetFPS();
	}

	void Root::SetRootPath(const char* setRootPath)
	{
		rootPath = setRootPath;
	}

	const char* Root::GetRootPath()
	{
		return rootPath.c_str();
	}

	void Root::Release()
	{
		assets.Release();

		Sprite::Release();

		fonts.Release();
		render.Release();
		controls.Release();
		sounds.Release();

		redirectLog = false;

		memory.LogMemory();
	}
}
