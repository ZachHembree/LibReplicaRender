#ifndef REP_WIND_H
#define REP_WIND_H

#include "RepLeanWin.h"
#include "RepMain.h"
#include "RepWinException.hpp"

#include <glm/glm.hpp>
#include "DynamicCollections.hpp"
#include <string_view>

namespace Replica
{
	typedef glm::tvec2<DWORD> WndStyle;
	typedef glm::ivec2 ivec2;
	using std::string_view;
	using std::wstring_view;
	using std::string;
	using std::wstring;

	class WindowComponentBase;

	/// <summary>
	/// Minimal wrapper class for Win32 Window
	/// </summary>
	class MinWindow
	{
		public:
			MinWindow(
				wstring_view name, 
				ivec2 initSize, 
				WndStyle initStyle, 
				const HINSTANCE hInst, 
				const wchar_t* iconRes
			);

			MinWindow(MinWindow&& other) noexcept;

			MinWindow(const MinWindow&) = delete;

			MinWindow& operator=(MinWindow&& rhs) noexcept;

			MinWindow& operator=(const MinWindow&) = delete;

			~MinWindow();

			/// <summary>
			/// Registers component object to the window.
			/// </summary>
			virtual void RegisterComponent(WindowComponentBase* component);

			/// <summary>
			/// Updates window message loop until the window is closed
			/// </summary>
			virtual MSG RunMessageLoop();

			/// <summary>
			/// Returns the name of the window
			/// </summary>
			const wstring_view GetName() const noexcept;

			/// <summary>
			/// Returns handle to executable process associated with the window
			/// </summary>
			HINSTANCE GetProcHandle() const noexcept;

			/// <summary>
			/// Returns handle for window object provided by the Win32 API
			/// </summary>
			HWND GetWndHandle() const noexcept;

			/// <summary>
			/// Returns the size of the window's body in pixels.
			/// </summary>
			ivec2 GetSize() const;
			
			/// <summary>
			/// Resizes the window to the given dimensions in pixels.
			/// </summary>
			void SetSize(ivec2 size);

			/// <summary>
			/// Returns the position of the window's top right corner in pixels
			/// </summary>
			ivec2 GetPos() const;

			/// <summary>
			/// Sets the position of the window's top right corner in pixels
			/// </summary>
			void SetPos(ivec2 pos);

			/// <summary>
			/// Returns the contents of the titlebar
			/// </summary>
			wstring GetWindowTitle() const;

			/// <summary>
			/// Writes the given text to the titlebar
			/// </summary>
			void SetWindowTitle(wstring_view text);

			/// <summary>
			/// Returns style and extended style as a vector
			/// </summary>
			WndStyle GetStyle() const;

			/// <summary>
			/// Sets the style of the window. Ex-style optional.
			/// </summary>
			void SetStyle(WndStyle style);

			/// <summary>
			/// Hides the window's borders
			/// </summary>
			void SetStyleBorderless();

			/// <summary>
			/// Resets the window to its initial style
			/// </summary>
			void ResetStyle();

			/// <summary>
			/// Returns the current resolution of the monitor occupied by the window
			/// </summary>
			ivec2 GetMonitorResolution() const;

		protected:	
			static MinWindow* pLastInit;

			const wstring_view name;
			WndStyle lastStyle;
			HINSTANCE hInst;
			HWND hWnd;
			MSG wndMsg;
			ivec2 size;

			/// <summary>
			/// Component objects associated with the window
			/// </summary>
			UniqueVector<WindowComponentBase*> components;

			/// <summary>
			/// Processes next window message without removing it from the queue.
			/// Returns false only on exit.
			/// </summary>
			virtual bool PollWindowMessages();

			/// <summary>
			/// Proceedure for processing window messages sent from Win32 API
			/// </summary>
			LRESULT OnWndMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

			/// <summary>
			/// Handles messaging setup on creation of new windows
			/// </summary>
			static LRESULT CALLBACK HandleWindowSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

			/// <summary>
			/// Forwards messages from the Win32 API to the appropriate window instance
			/// </summary>
			static LRESULT CALLBACK WindowMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}

#endif