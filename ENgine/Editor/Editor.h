#pragma once

#include "Root/Root.h"
#include "FreeCamera.h"
#include "Project.h"
#include "Gizmo.h"

#include "imgui.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include "eastl/bonus/ring_buffer.h"
#include "Root/Assets/Assets.h"

/**
\ingroup gr_code_editor
*/

namespace Oak
{
	class CLASS_DECLSPEC Editor : Object
	{
		friend class Project;
		friend class FreeCamera;
		friend class EditorDrawer;
		friend class Root;

		struct ProjectEntry
		{
			eastl::string name;
			eastl::string path;
		};

		eastl::vector<ProjectEntry> projects;

		HWND hwnd;
		ID3D11Device* d3dDevice = nullptr;
		ID3D11DeviceContext* d3dDeviceContext = nullptr;
		IDXGISwapChain* swapChain = nullptr;
		ID3D11RenderTargetView* mainRenderTargetView = nullptr;

		TaskExecutor::SingleTaskPool* renderTaskPool = nullptr;
		FreeCamera freeCamera;

		bool showAbout = false;
		bool showProjectSettings = false;

		Assets::Folder* selectedFolder = nullptr;
		Assets::AssetHolder* selectedAssetHolder = nullptr;
		Asset* selectedAsset;

		AssetTextureRef draggedTextureAsset;
		AssetAnimGraph2DRef draggedAssetAnimGraph2D;

		bool projectTreePopup = false;
		bool sceneTreePopup = false; 
		bool viewportCaptured = false;
		bool vireportHowered = false;
		bool entityDeletedViaPopup = false;

		bool projectRunning = false;
		bool allowSceneDropTraget = true;

		struct LogCategory
		{
			int selItem = -1;
			eastl::ring_buffer<eastl::string, eastl::vector<eastl::string>> logs;
			eastl::vector<const char*> logsPtr;

			LogCategory() : logs(128)
			{
				logsPtr.reserve(128);
			}
		};

		eastl::map<eastl::string, LogCategory*> logCategories;

		void CaptureLog(const char* name, const char* text);

	public:

		SceneEntity* selectedEntity = nullptr;
		Project project;
		Gizmo gizmo;
		bool viewportFocused = false;

		const char* OpenFileDialog(const char* extName, const char* ext, bool open);

		bool Init(HWND hwnd);
		bool Update();
		void Render(float dt);
		bool ProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void OnResize(int width, int height);

		void SelectEntity(SceneEntity* entity);

		void Release();

	private:

		void SetupImGUI();
		bool ShowEditor();
		void ShowAbout();
		void ShowProjectSettings();
		void ShowViewport();
		void ShowSelectProject();
		void ProjectTreePopup(bool contextItem);
		void SceneTreePopup(bool contextItem);
		void SceneDropTraget(SceneEntity* entity);
		void EntitiesTreeView(const eastl::vector<SceneEntity*>& entities);
		void AssetsFolder(Assets::Folder* folder);

		void StartProject();
		void StopProject();

		void SaveProjectsList();

		template<typename Func>
		void PushButton(const char* label, bool pushed, Func callback)
		{
			float b = 1.0f;
			float c = 0.5f;
			int i = 4;
			bool needPopStyle = false;

			if (pushed)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, b, b));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, b, b));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, c, c));
				needPopStyle = true;
			}

			if (ImGui::Button(label, ImVec2(50.0f, 25.0f)))
			{
				callback();
			}

			if (needPopStyle)
			{
				ImGui::PopStyleColor(3);
				needPopStyle = false;
			}

			ImGui::SameLine();
		}

		bool CreateDeviceD3D();
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();

		void UpdateOak();
	};

	extern CLASS_DECLSPEC Editor editor;
}
