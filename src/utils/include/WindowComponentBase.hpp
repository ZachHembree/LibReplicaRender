#pragma once
#include "MinWindow.hpp"

namespace Replica
{
	class MinWindow;

	/// <summary>
	/// Base class for window components
	/// </summary>
	class WindowComponentBase
	{
	friend MinWindow;

	public:
		const MinWindow* parent;

		/// <summary>
		/// Called after each message poll in the window class
		/// </summary>
		virtual void Update() { }

		bool GetIsRegistered();

	protected:

		WindowComponentBase(MinWindow* window);

	private:
		bool isRegistered;

		WindowComponentBase(const WindowComponentBase&) = delete;

		WindowComponentBase(WindowComponentBase&&) = delete;

		WindowComponentBase& operator=(const WindowComponentBase&) = delete;

		WindowComponentBase& operator=(WindowComponentBase&&) = delete;
	};
}