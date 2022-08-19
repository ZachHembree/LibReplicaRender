#pragma once
#include "D3D11/D3DUtils.hpp"
#include "D3D11/dev/DeviceChild.hpp"
#include "DynamicCollections.hpp"

namespace Replica::D3D11
{
	class BufferBase;
	class VertexBuffer;
	class IndexBuffer;
	class ConstantBuffer;
	class SwapChain;
	class InputLayout;
	class VertexShader;
	class PixelShader;

	/// <summary>
	/// Determines how vertex topology is interpreted by the input assembler
	/// </summary>
	enum class PrimTopology
	{
		UNDEFINED = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		POINTLIST = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		LINELIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		LINESTRIP = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		TRIANGLELIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		TRIANGLESTRIP = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		LINELIST_ADJ = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
		LINESTRIP_ADJ = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
		TRIANGLELIST_ADJ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
		TRIANGLESTRIP_ADJ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
		CP1_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		CP2_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
		CP3_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
		CP4_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
		CP5_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST,
		CP6_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST,
		CP7_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST,
		CP8_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST,
		CP9_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST,
		CP10_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST,
		CP11_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST,
		CP12_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST,
		CP13_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST,
		CP14_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST,
		CP15_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST,
		CP16_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST,
		CP17_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST,
		CP18_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST,
		CP19_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST,
		CP20_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST,
		CP21_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST,
		CP22_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST,
		CP23_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST,
		CP24_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST,
		CP25_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST,
		CP26_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST,
		CP27_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST,
		CP28_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST,
		CP29_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST,
		CP30_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST,
		CP31_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST,
		CP32_PATCHLIST = D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST
	};

	/// <summary>
	/// Class for issuing commands to an associated D3D11 device
	/// </summary>
	class Context : public DeviceChild
	{
	public:
		Context(Device* pDev, ComPtr<ID3D11DeviceContext>& pContext);

		Context();

		Context(Context&& other) noexcept;

		Context& operator=(Context&& other) noexcept;

		/// <summary>
		/// Binds the given vertex shader
		/// </summary>
		void SetVS(VertexShader* vs);

		/// <summary>
		/// Binds the given pixel shader
		/// </summary>
		void SetPS(PixelShader* ps);

		/// <summary>
		/// Removes the given vertex shader, if bound
		/// </summary>
		void RemoveVS(VertexShader* vs);

		/// <summary>
		/// Removes the given pixel shader, if bound
		/// </summary>
		void RemovePS(PixelShader* ps);

		/// <summary>
		/// Unbinds all resources
		/// </summary>
		void Reset();

		/// <summary>
		/// Returns pointer to context interface
		/// </summary>
		ID3D11DeviceContext* Get() const;
		
		/// <summary>
		/// Binds the given viewport to the rasterizer stage
		/// </summary>
		void RSSetViewport(const vec2 size, const vec2 offset = vec2(0, 0), const vec2 depth = vec2(0, 1));

		/// <summary>
		/// Binds a vertex buffer to the given slot
		/// </summary>
		void IASetVertexBuffer(VertexBuffer& vertBuffer, int slot = 0);

		/// <summary>
		/// Determines how vertices are interpreted by the input assembler
		/// </summary>
		void IASetPrimitiveTopology(PrimTopology topology);

		/// <summary>
		/// Binds an array of buffers starting at the given slot
		/// </summary>
		void IASetVertexBuffers(IDynamicCollection<VertexBuffer>& vertBuffers, int startSlot = 0);

		/// <summary>
		/// Draw indexed, non-instanced primitives
		/// </summary>
		void DrawIndexed(UINT length, UINT start = 0, UINT baseVertexLocation = 0);

	private:
		ComPtr<ID3D11DeviceContext> pContext;
		VertexShader* currentVS;
		PixelShader* currentPS;
	};
}