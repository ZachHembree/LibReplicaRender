#pragma once
#include "DeviceChild.hpp"
#include "Formats.hpp"

namespace Replica::D3D11
{
	struct IAElement;

	/// <summary>
	/// Defines the layout of vertex data supplied to the input-assembler
	/// </summary>
	class InputLayout : public DeviceChild
	{
	public:

		InputLayout() noexcept;

		/// <summary>
		/// Constructs layout definition associated with given shader bytecode
		/// </summary>
		InputLayout(Device& dev, 
			const byte* pVS,
			const size_t srcSize,
			const IDynamicArray<IAElement>& layout
		);

		/// <summary>
		/// Returns pointer to COM layout interface. Null if uninitialized.
		/// </summary>
		ID3D11InputLayout* Get() const;

		/// <summary>
		/// Binds the layout to the input assembler for the associated context
		/// </summary>
		void Bind(Context& ctx);

	private:
		ComPtr<ID3D11InputLayout> pLayout;
	};

	/// <summary>
	/// Describes a single element for vertex data supplied to the input assembler
	/// </summary>
	struct IAElement
	{
		/// <summary>
		/// HLSL semantic associated with the element
		/// </summary>
		LPCSTR SemanticName;

		/// <summary>
		/// Optional index modifying the name. Ex: Color0, Color1...
		/// </summary>
		UINT SemanticIndex;

		/// <summary>
		/// Data format R8G8B8, UNORM, etc.
		/// </summary>
		Formats Format;

		/// <summary>
		/// Input assembler slot (0 - 15)
		/// </summary>
		UINT InputSlot;

		/// <summary>
		/// Optional. Byte offset of element from start of vertex.
		/// </summary>
		UINT AlignedByteOffset;

		/// <summary>
		/// Indicates whether a slot specifies per-vertex or per-instance data
		/// </summary>
		InputClass InputSlotClass;

		/// <summary>
		/// Number of instances to draw using the same per-instance data before
		/// moving to the next element in the buffer. Must be 0 for per-vertex data.
		/// </summary>
		UINT InstanceDataStepRate;

		IAElement(string_view semantic = "UNKNOWN",
			Formats format = Formats::UNKNOWN,
			UINT semanticIndex = 0u,
			UINT iaSlot = 0u,
			InputClass slotClass = InputClass::PerVertex,
			UINT instStepRate = 0u,
			UINT offset = D3D11_APPEND_ALIGNED_ELEMENT
		);

		IAElement(string_view semantic,
			UINT semanticIndex,
			Formats format,
			UINT iaSlot = 0u,
			InputClass slotClass = InputClass::PerVertex,
			UINT instStepRate = 0u,
			UINT offset = D3D11_APPEND_ALIGNED_ELEMENT
		);
	};
}