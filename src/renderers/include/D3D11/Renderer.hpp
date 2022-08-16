#pragma once
#include "GfxException.hpp"
#include <d3d11.h>
#include <wrl.h>
#include "WindowComponentBase.hpp"
#include "Input.hpp"
#include "D3D11/dev/VertexBuffer.hpp"
#include "D3D11/dev/IndexBuffer.hpp"
#include "D3D11/dev/ConstantBuffer.hpp"
#include "D3D11/SwapChain.hpp"
#include "D3D11/dev/Device.hpp"
#include "D3D11/dev/RenderTarget.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "Dxgi.lib")

namespace Replica
{
	class InputComponent;
}

namespace Replica::D3D11
{ 
	class Device;
	class BufferBase;
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class SwapChain;

	class Renderer : public WindowComponentBase
	{
	public:
		Renderer(MinWindow* window);

		void Update() override;

	private:
		InputComponent input;
		Device device;
		SwapChain swap;
		RenderTarget backBuf;

		VertexBuffer vBuf;
		IndexBuffer iBuf;
	};
}