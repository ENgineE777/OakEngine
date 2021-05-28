
#pragma once

#include <cinttypes>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>


#ifdef PLATFORM_WIN

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>

#endif

namespace Oak
{
	enum class AliasAction
	{
		Pressed /*!< Hardware button in pressed state */,
		JustPressed /*!< Harwdware button was just pressed */
	};

	/**
	\ingroup gr_code_root_controls
	*/

	/**
	\brief Controls

	This service class allows to read user input. System based on work with alliases. That means
	no hardcoded keys. User will works with alliases that can be accosiated with harwdare keys. This
	aproach allow to elimanate harcoded names of a hardware keys.
	Alises store in JSON file and can be loaded via call of Controls::Load.
	For debug porposes there are methods DebugKeyPressed and DebugHotKeyPressed for direct for with
	names of hardware keys.
	Servics also allows to work with differen inpunt devices like keybord, mouse and gamepad.
	All list of supported hardware kes can be found in files settings/controls/hardware_win and
	settings/controls/hardware_mobile.

	Sample of declaration of alias

	\code
	"name" : "Character.MOVE_VERT", // name of alias, index of alias shoudl be obtained via call Controls::GetAlias
	"AliasesRef" : [
		{ "names" : ["KEY_W"], "modifier" : 1.0 }, //assoiate key 'W' with alias
		{ "names" : ["KEY_S"], "modifier" : -1.0 }, //assoiate key 'S' with alias, multiplier -1.0f should aplyed to returned value via call Controls::GetAliasValue
		{ "names" : ["JOY_LEFT_STICK_V"] }  //assoiate vertical movement of left stick of gamepad
	]
	\endcode

	Sample of declaration of alias which uses combination of keys.

	\code
	"name" : "Character.GRAB", // name of alias, index of alias shoudl be obtained via call Controls::GetAlias
	"AliasesRef" : [
		{ "names" : ["KEY_W", "KEY_F"], //assoiate combination of keys 'W' and 'F' with alias
	]
	\endcode

	*/

	class Controls
	{
	public:

	private:

	#ifndef DOXYGEN_SKIP

		enum class Device
		{
			Keyboard,
			Mouse,
			Joystick,
			Touch
		};

		enum Consts
		{
			TouchCount = 10
		};

		bool focused = true;

		eastl::vector<int> supressed_aliases;

	#ifdef PLATFORM_WIN
		uint8_t                 btns[256];

		LPDIRECTINPUT8          pDI;
		LPDIRECTINPUTDEVICE8    pKeyboard;
		LPDIRECTINPUTDEVICE8    pMouse;

		uint32_t                dwElements;
		byte                    diks[256];
		DIMOUSESTATE2           dims2;
		uint32_t                dwMsElements;
		byte                    ms_bts[10];
		int                     ms_x, ms_y, ms_z;
		int                     prev_ms_x, prev_ms_y, prev_ms_z;
		XINPUT_STATE            joy_prev_states[XUSER_MAX_COUNT];
		XINPUT_STATE            joy_states[XUSER_MAX_COUNT];
		bool                    joy_active[XUSER_MAX_COUNT];

		inline float GetJoyTrigerValue(float val)
		{
			return val / 255.0f;
		}

		inline float GetJoyStickValue(float val)
		{
			val = fmaxf(-1, (float)val / 32767);
			float deadzone = 0.05f;
			val = (abs(val) < deadzone ? 0 : (abs(val) - deadzone) * (val / abs(val)));

			return val /= 1.0f - deadzone;
		}
	#endif

		struct HardwareAlias
		{
			eastl::string name;
			Device device;
			int index;
			float value;
		};

		struct AliasRefState
		{
			eastl::string name;
			int         aliasIndex = -1;
			bool        refer2hardware = false;
			int         device_index = -1;
		};

		struct AliasRef
		{
			float       modifier = 1.0f;
			eastl::vector<AliasRefState> refs;
		};

		struct Alias
		{
			eastl::string name;
			bool visited = false;
			eastl::vector<AliasRef> aliasesRef;
		};

		bool allowDebugKeys = false;
		eastl::vector<HardwareAlias> haliases;
		eastl::map<eastl::string, int> debeugMap;

		eastl::vector<Alias> aliases;
		eastl::map<eastl::string, int> aliasesMap;

		void  ResolveAliases();
		void  CheckDeadEnds(Alias& alias);
		bool  GetHardwareAliasState(int alias, AliasAction action, int device_index, bool ignore_focus);
		float GetHardwareAliasValue(int alias, bool delta, int device_index, bool ignore_focus);

		struct TouchState
		{
			int state = 0;
			int x = 0;
			int y = 0;
			int prev_x = 0;
			int prev_y = 0;
		};

		TouchState buffered_touches[TouchCount];
		TouchState touches[TouchCount];

		#endif

	public:

		#ifndef DOXYGEN_SKIP

		struct AliasMappig
		{
			eastl::string name;
			int    alias = -1;

			struct BindName
			{
				int device_index = -1;
				eastl::string name;
			};

			eastl::vector<eastl::vector<BindName>> bindedNames;

			AliasMappig(const char* name);
			bool IsContainHAlias(const char* halias);
		};

		bool  Init(const char* haliases, bool allowDebugKeys);
		#endif

		/**
			\brief Load set of aliases for a file.

			\param[in] aliases Full path to a file with alias

			\return Returns True if aliases was successfully loade. Otherwise false will be returned.
		*/
		bool LoadAliases(const char* aliases);

		/**
			\brief Get id of alias

			\param[in] name Name of a alias

			\return Id of alias. -1 will be returned in case alias doesn't exist.

		*/
		int GetAlias(const char* name);

		/**
			\brief Get state of a alias.

			\param[in] alias Id of a alias
			\param[in] action State of a alias.

			\return Returns true in case alias in requsted state. Otherwise false will be returned.
		*/
		bool  GetAliasState(int alias, AliasAction action = AliasAction::JustPressed);

		/**
			\brief Get value of a alias. Keys are returning ether 0 or 1. Sticks of gamepad can have value from -1.0f to 1.0f

			\param[in] alias Id of a alias
			\param[in] delta Indicates if should returned falue or difference this last frame

			\return Value of associated hardware keys

		*/
		float GetAliasValue(int alias, bool delta);

		/**
			\brief Get name of cuurentlu active hardware alias. This method needed for redifining controls via UI of a game

			\param[out] device_index Internal index of hardware device

			\return Name of hardware key

		*/
		const char* GetActivatedKey(int& device_index);

		/**
			\brief Check state of hardware alias

			\param[in] name Name of a hardware alias
			\param[in] action State of a alias.
			\param[in] ignore_focus Input are reciving only in focused state. This flag allows to recive stete even in unfocused state.

			\return State of hardwrae alias.
		*/
		bool  DebugKeyPressed(const char* name, AliasAction action = AliasAction::JustPressed, bool ignore_focus = false);

		/**
			\brief Check state of up to 3 hardware alias at same time. This allows to check hot kee constoted from severel keys.

			\param[in] name Name of a first hardware alias
			\param[in] name2 Name of a second hardware alias
			\param[in] name3 Name of a third hardware alias

			\return State of combination hardare aliases.
		*/
		bool  DebugHotKeyPressed(const char* name, const char* name2, const char* name3 = nullptr);

		/**
			\brief Check if gamepad is connected.

			\return True will be returned if at least one gamepad is connecteds. False will be returned if non og gamepads are connected.
		*/
		bool IsGamepadConnected();

		#ifndef DOXYGEN_SKIP

		void SupressAlias(int alias_index);

		void SetFocused(bool set_focused);

		#ifdef PLATFORM_WIN
		void OverrideMousePos(int mx, int my);
		#endif

		void TouchStart(int index, int x, int y);
		void TouchUpdate(int index, int x, int y);
		void TouchEnd(int index);

		void Update(float dt);

		void Release();

		#endif
	};
}