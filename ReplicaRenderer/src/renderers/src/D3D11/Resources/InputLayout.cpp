#include "DynamicCollections.hpp"
#include "D3D11/Resources/InputLayout.hpp"
#include "D3D11/Device.hpp"

using namespace Replica::D3D11;
using namespace Microsoft::WRL;

InputLayout::InputLayout() noexcept : DeviceChild(nullptr) { }

InputLayout::InputLayout(Device& dev, 
	const ComPtr<ID3DBlob>& vsBlob, 
	const IDynamicCollection<IAElement>& description
) : DeviceChild(&dev)
{ 
	GFX_THROW_FAILED(dev.Get()->CreateInputLayout(
		reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(description.GetPtr()),
		(UINT)description.GetLength(),
		vsBlob->GetBufferPointer(),
		(UINT)vsBlob->GetBufferSize(),
		&pLayout
	));
}

ID3D11InputLayout* InputLayout::Get() const { return pLayout.Get(); };

void InputLayout::Bind(Context& ctx)
{
	ctx.Get()->IASetInputLayout(Get());
}

IAElement::IAElement(string_view semantic,
	Formats format,
	UINT semanticIndex,
	UINT iaSlot,
	InputClass slotClass,
	UINT instStepRate,
	UINT offset
) :
	SemanticName(semantic.data()),
	SemanticIndex(semanticIndex),
	Format(format),
	InputSlot(iaSlot),
	AlignedByteOffset(offset),
	InputSlotClass(slotClass),
	InstanceDataStepRate(instStepRate)
{ }

IAElement::IAElement(string_view semantic,
	UINT semanticIndex,
	Formats format,
	UINT iaSlot,
	InputClass slotClass,
	UINT instStepRate,
	UINT offset
) :
	SemanticName(semantic.data()),
	SemanticIndex(semanticIndex),
	Format(format),
	InputSlot(iaSlot),
	AlignedByteOffset(offset),
	InputSlotClass(slotClass),
	InstanceDataStepRate(instStepRate)
{ }