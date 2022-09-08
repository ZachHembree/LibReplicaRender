#pragma once
#include "D3DUtils.hpp"

namespace Replica
{
	class MinWindow;
}

namespace Replica::D3D11
{
	class Context;
	class Renderer;
	class Device;

	/// <summary>
	/// Interface for types that can be drawn
	/// </summary>
	class RenderComponentBase : private MoveOnlyObjBase
	{
	friend Renderer;

	public:
		RenderComponentBase(RenderComponentBase&&) = delete;
		RenderComponentBase& operator=(RenderComponentBase&&) = delete;

		/// <summary>
		/// Updates before draw, but after the state and resources for the previous
		/// frame have been cleaned up.
		/// </summary>
		virtual void Update(Context& ctx) { }

		virtual void DrawEarly(Context& ctx) { }

		virtual void Draw(Context& ctx) { }

		virtual void DrawLate(Context& ctx) { }

		/// <summary>
		/// Called after swap chain present but before any resources have been released
		/// or cleaned up.
		/// </summary>
		virtual void AfterDraw(Context& ctx) { }

		/// <summary>
		/// Returns reference to the renderer this component is attached to
		/// </summary>
		Renderer& GetRenderer();

		/// <summary>
		/// Returns device interface to the renderer
		/// </summary>
		Device& GetDevice();

		/// <summary>
		/// Returns interface reference to window being rendered to
		/// </summary>
		MinWindow& GetWindow();

		/// <summary>
		/// Returns true if the component has been registered to a renderer
		/// </summary>
		bool GetIsRegistered();

		/// <summary>
		/// Registers the component to the given renderer
		/// </summary>
		virtual void Register(Renderer& renderer);

		/// <summary>
		/// Unregisters the component from its parent renderer
		/// </summary>
		virtual void Unregister();

	protected:
		Renderer* pRenderer;
		bool isRegistered;

		RenderComponentBase();

		RenderComponentBase(Renderer& renderer);

		~RenderComponentBase();
	};
}