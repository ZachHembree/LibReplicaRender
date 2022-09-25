#include "ReplicaWin32.hpp"
#include "ReplicaInternalD3D11.hpp"

using namespace Replica;
using namespace Replica::D3D11;

Renderer::Renderer(MinWindow& window) :
	WindowComponentBase(window),
	device(*this), // Create device and context
	swap(window, device), // Create swap chain for window
	defaultDS(device, swap.GetSize()),
	useDefaultDS(true),
	fitToWindow(true)
{ 
	MeshDef quadDef = Primitives::GeneratePlane<VertexPos2D>(ivec2(0), 2.0f);
	defaultMeshes[L"FSQuad"] = Mesh(device, quadDef);

	defaultEffects[L"Default"] = Effect(device, g_DefaultEffect);
	defaultEffects[L"PosTextured2D"] = Effect(device, g_PosTextured2DEffect);
	defaultEffects[L"Textured2D"] = Effect(device, g_Textured2DEffect);
	defaultEffects[L"DebugFlat3D"] = Effect(device, g_DebugFlat3DEffect);
	defaultEffects[L"Textured3D"] = Effect(device, g_Textured3DEffect);

	defaultCompute[L"TexCopyScaledSamp2D"] = ComputeShader(device, g_CS_TexCopyScaledSamp2D);
	defaultCompute[L"TexCopySamp2D"] = ComputeShader(device, g_CS_TexCopySamp2D);
	defaultCompute[L"TexCopy2D"] = ComputeShader(device, g_CS_TexCopy2D);

	defaultSamplers[L"PointClamp"] = Sampler(device, TexFilterMode::POINT, TexClampMode::CLAMP);
	defaultSamplers[L"PointMirror"] = Sampler(device, TexFilterMode::POINT, TexClampMode::MIRROR);
	defaultSamplers[L"PointBorder"] = Sampler(device, TexFilterMode::POINT, TexClampMode::BORDER);

	defaultSamplers[L"LinearClamp"] = Sampler(device, TexFilterMode::LINEAR, TexClampMode::CLAMP);
	defaultSamplers[L"LinearMirror"] = Sampler(device, TexFilterMode::LINEAR, TexClampMode::MIRROR);
	defaultSamplers[L"LinearBorder"] = Sampler(device, TexFilterMode::LINEAR, TexClampMode::BORDER);
}

/// <summary>
/// Returns the interface to the device the renderer is running on
/// </summary>
Device& Renderer::GetDevice() { return device; }

/// <summary>
/// Returns reference to the swap chain interface
/// </summary>
SwapChain& Renderer::GetSwapChain() { return swap; }

/// <summary>
/// Returns the viewport used with the back buffer
/// </summary>
Viewport Renderer::GetMainViewport() const
{
	return
	{
		swap.GetBackBuf().GetOffset(), 
		swap.GetBackBuf().GetSize(),
		defaultDS.GetRange() 
	};
}

/// <summary>
/// Sets the viewport used with the back buffer
/// </summary>
void Renderer::SetMainViewport(Viewport& vp)
{
	SetOutputResolution(vp.size);
	swap.GetBackBuf().SetOffset(vp.offset);
	defaultDS.SetRange(vp.zDepth);
}

/// <summary>
/// Returns the rendering resolution used by the renderer
/// </summary>
vec2 Renderer::GetOutputResolution() const 
{
	return swap.GetBackBuf().GetSize();
}

/// <summary>
/// Sets the render resolution to the given value
/// </summary>
void Renderer::SetOutputResolution(vec2 res)
{
	const ivec2 lastBackSize = swap.GetSize(),
		stencilSize = defaultDS.GetSize();

	if (res.x > lastBackSize.x || res.y > lastBackSize.y)
		swap.ResizeBuffers(res);

	if (res.x > stencilSize.x || res.y > stencilSize.y)
		defaultDS = DepthStencilTexture(device, res);

	swap.GetBackBuf().SetRenderSize(res);
}

/// <summary>
/// Returns true if the render resolution is set to match
/// that of the window body being rendered to.
/// </summary>
bool Renderer::GetIsFitToWindow() const { return fitToWindow; }

/// <summary>
/// Set to true if the renderer should automatically match the
/// window resolution.
/// </summary>
void Renderer::SetIsFitToWindow(bool value) { fitToWindow = value; }

/// <summary>
/// Returns true if the default depth stencil buffer is enabled
/// </summary>
bool Renderer::GetIsDepthStencilEnabled() { return useDefaultDS; }

/// <summary>
/// Enable/disable default depth-stencil buffer
/// </summary>
void Renderer::SetIsDepthStencilEnabled(bool value) { useDefaultDS = value; }

/// <summary>
/// Returns reference to a default effect
/// </summary>
Effect& Renderer::GetDefaultEffect(wstring_view name) const
{
	return (Effect&)defaultEffects.at(name);
}

/// <summary>
/// Returns reference to a default compute shader
/// </summary>
ComputeShader& Renderer::GetDefaultCompute(wstring_view name) const
{
	return (ComputeShader&)defaultCompute.at(name);
}

/// <summary>
/// Retursn a reference to a default mesh
/// </summary>
Mesh& Renderer::GetDefaultMesh(wstring_view name) const
{
	return (Mesh&)defaultMeshes.at(name);
}

/// <summary>
/// Retursn a reference to a default texture sampler
/// </summary>
Sampler& Renderer::GetDefaultSampler(wstring_view name) const
{
	return (Sampler&)defaultSamplers.at(name);
}

bool Renderer::OnWndMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	return true;
}

/// <summary>
/// Registers a new render component. Returns false if the component is already registered.
/// </summary>
bool Renderer::RegisterComponent(RenderComponentBase& component)
{
	if (!component.GetIsRegistered())
	{
		component.pRenderer = this;
		component.isRegistered = true;
		pComponents.push_back(&component);
		return true;
	}
	else
		return false;
}

/// <summary>
/// Unregisters the given component from the renderer. Returns false on fail.
/// </summary>
bool Renderer::UnregisterComponent(RenderComponentBase& component)
{
	int index = -1;

	for (int i = (int)pComponents.GetLength() - 1; i >= 0; i--)
	{
		if (pComponents[i] == &component)
		{
			index = i;
			break;
		}
	}

	if (index != -1)
	{
		component.isRegistered = false;
		component.pRenderer = nullptr;
		pComponents.RemoveAt(index);
		return true;
	}
	else
		return false;
}

void Renderer::Update()
{
	// Reset binds
	Context& ctx = device.GetContext();
	const ivec2 bodySize = GetWindow().GetBodySize();

	ctx.Reset();

	// Clear back buffer
	swap.GetBackBuf().Clear(ctx);

	if (useDefaultDS)
		defaultDS.Clear(ctx);

	// Set viewport bounds
	if (fitToWindow)
		SetOutputResolution(bodySize);

	// Bind back buffer as render target
	if (useDefaultDS)
		ctx.SetRenderTarget(swap.GetBackBuf(), defaultDS);
	else
		ctx.SetRenderTarget(swap.GetBackBuf());

	BeforeDraw(ctx);

	DrawEarly(ctx);
	Draw(ctx);
	DrawLate(ctx);

	// Present frame
	swap.Present(1u, 0);

	AfterDraw(ctx);
}

void Renderer::BeforeDraw(Context& ctx)
{ 
	for (RenderComponentBase* pComp : pComponents)
	{
		pComp->Setup(ctx);
	}
}

void Renderer::DrawEarly(Context& ctx)
{ 
	for (RenderComponentBase* pComp : pComponents)
	{
		pComp->DrawEarly(ctx);
	}
}

void Renderer::Draw(Context& ctx)
{
	for (RenderComponentBase* pComp : pComponents)
	{
		pComp->Draw(ctx);
	}
}

void Renderer::DrawLate(Context& ctx)
{
	for (RenderComponentBase* pComp : pComponents)
	{
		pComp->DrawLate(ctx);
	}	
}

void Renderer::AfterDraw(Context& ctx)
{
	for (RenderComponentBase* pComp : pComponents)
	{
		pComp->AfterDraw(ctx);
	}
}
