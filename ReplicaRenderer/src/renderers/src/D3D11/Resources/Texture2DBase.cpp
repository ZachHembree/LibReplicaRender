#include "D3D11/Device.hpp"
#include "D3D11/Resources/Formats.hpp"
#include "D3D11/Resources/BufferBase.hpp"
#include "D3D11/Resources/Texture2DBase.hpp"
#include <WICTextureLoader.h>
#include <DirectXTex.h>

using namespace DirectX;
using namespace Replica::D3D11;

Texture2DBase::Texture2DBase(
	Device& dev,
	ivec2 dim,
	Formats format,
	ResourceUsages usage,
	ResourceBindFlags bindFlags,
	ResourceAccessFlags accessFlags,
	UINT mipLevels,
	UINT arraySize,
	void* data,
	UINT stride
) :
	ResourceBase(dev)
{
	desc = {};
	desc.Width = dim.x;
	desc.Height = dim.y;
	desc.Format = (DXGI_FORMAT)format;

	desc.MipLevels = mipLevels;
	desc.ArraySize = 1;

	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = (D3D11_BIND_FLAG)bindFlags;
	desc.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)accessFlags;
	desc.MiscFlags = 0u;

	// Multisampling
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if (data != nullptr)
	{ 
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;
		initData.SysMemPitch = dim.x * stride;
		GFX_THROW_FAILED(GetDevice()->CreateTexture2D(&desc, &initData, &pRes));
	}
	else if (dim.x != 0 && dim.y != 0)
	{
		GFX_THROW_FAILED(GetDevice()->CreateTexture2D(&desc, nullptr, &pRes));
	}
}

Texture2DBase::Texture2DBase() : desc({}) {}

ivec2 Texture2DBase::GetSize() const { return ivec2(desc.Width, desc.Height); }

Formats Texture2DBase::GetFormat() const { return (Formats)desc.Format; }

ResourceUsages Texture2DBase::GetUsage() const { return (ResourceUsages)desc.Usage; }

ResourceBindFlags Texture2DBase::GetBindFlags() const { return (ResourceBindFlags)desc.BindFlags; }

ResourceAccessFlags Texture2DBase::GetAccessFlags() const { return (ResourceAccessFlags)desc.CPUAccessFlags; }

/// <summary>
/// Returns interface to resource
/// </summary>
ID3D11Resource* Texture2DBase::GetResource() { return pRes.Get(); }

void Texture2DBase::UpdateMapUnmap(Context& ctx, void* data, size_t stride, ivec2 dim)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GFX_THROW_FAILED(ctx->Map(
		pRes.Get(),
		0u,
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&msr
	));

	memcpy(msr.pData, data, stride * dim.x * dim.y);
	ctx->Unmap(pRes.Get(), 0u);
}

void Texture2DBase::UpdateSubresource(Context& ctx, void* data, size_t stride, ivec2 dim)
{
	D3D11_BOX dstBox;
	dstBox.left = 0;
	dstBox.right = dim.x;
	dstBox.top = 0;
	dstBox.bottom = dim.y;
	dstBox.front = 0;
	dstBox.back = 1;

	ctx->UpdateSubresource(
		pRes.Get(),
		0,
		&dstBox,
		data,
		stride * dim.x,
		stride * dim.x * dim.y
	);
}

void Texture2DBase::LoadImageWIC(wstring_view file, ScratchImage& buffer)
{
	GFX_THROW_FAILED(LoadFromWICFile(
		file.data(),
		WIC_FLAGS::WIC_FLAGS_FORCE_RGB,
		nullptr,
		buffer
	));
}

/// <summary>
/// Returns true if the object is valid and has been initialized
/// </summary>
bool Texture2DBase::GetIsValid() const { return pDev != nullptr && pRes.Get() != nullptr; }

ID3D11Resource** const Texture2DBase::GetResAddress() { return reinterpret_cast<ID3D11Resource**>(pRes.GetAddressOf()); }