#include "D3D11/dev/InputLayout.hpp"

using namespace Replica::D3D11;
using namespace Microsoft::WRL;

InputLayout::InputLayout() noexcept : Child(nullptr) { }

InputLayout::InputLayout(InputLayout&& other) noexcept :
	Child(other.pDev),
	pLayout(std::move(other.pLayout))
{ }

InputLayout& InputLayout::operator=(InputLayout&& other) noexcept
{
	this->pLayout = std::move(other.pLayout);
	this->pDev = other.pDev;

	return *this;
}

InputLayout::InputLayout(const Device& dev, 
	const ComPtr<ID3DBlob>& vsBlob, 
	const std::initializer_list<IAElement>& layout
) : Child(&dev)
{ 
	UniqueArray desc(layout);
	dev.Get()->CreateInputLayout(
		reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(desc.GetPtr()),
		(UINT)desc.GetLength(),
		vsBlob->GetBufferPointer(),
		(UINT)vsBlob->GetBufferSize(),
		&pLayout
	);
}

ID3D11InputLayout* InputLayout::Get() const { return pLayout.Get(); };

void InputLayout::Bind()
{
	pDev->GetContext()->IASetInputLayout(Get());
}

IAElement::IAElement(LPCSTR semantic,
	Formats format,
	UINT semanticIndex,
	UINT iaSlot,
	InputClass slotClass,
	UINT instStepRate,
	UINT offset
) :
	SemanticName(semantic),
	SemanticIndex(semanticIndex),
	Format(format),
	InputSlot(iaSlot),
	AlignedByteOffset(offset),
	InputSlotClass(slotClass),
	InstanceDataStepRate(instStepRate)
{ }

IAElement::IAElement(LPCSTR semantic,
	UINT semanticIndex,
	Formats format,
	UINT iaSlot,
	InputClass slotClass,
	UINT instStepRate,
	UINT offset
) :
	SemanticName(semantic),
	SemanticIndex(semanticIndex),
	Format(format),
	InputSlot(iaSlot),
	AlignedByteOffset(offset),
	InputSlotClass(slotClass),
	InstanceDataStepRate(instStepRate)
{ }