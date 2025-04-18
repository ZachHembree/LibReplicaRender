#include "pch.hpp"
#include <WICTextureLoader.h>
#include <DirectXTex.h>
#include "D3D11/InternalD3D11.hpp"
#include "D3D11/Resources/StagingTexture2D.hpp"
#include "D3D11/Device.hpp"

using namespace DirectX;
using namespace Weave;
using namespace Weave::D3D11;

/// <summary>
/// Constructs an uninitialized texture object
/// </summary>
StagingTexture2D::StagingTexture2D() { }

/// <summary>
/// Constructs and allocates a staging texture with the given format and size.
/// CPU access flags cannot be none.
/// </summary>
StagingTexture2D::StagingTexture2D(Device& dev, Formats format, 
	ivec2 dim, uint mipLevels, ResourceAccessFlags accessFlags
) :
	Texture2DBase(
		dev,
		dim,
		format,
		ResourceUsages::Staging,
		ResourceBindFlags::None,
		accessFlags == ResourceAccessFlags::None ? ResourceAccessFlags::Write : accessFlags,
		1u, 1u,
		nullptr, 0u
	)
{ }

StagingTexture2D::StagingTexture2D(Device& dev, ivec2 dim, void* data, 
	uint stride, Formats format, uint mipLevels, ResourceAccessFlags accessFlags
) :
	Texture2DBase(
		dev,
		dim,
		format,
		ResourceUsages::Staging,
		ResourceBindFlags::None,
		accessFlags == ResourceAccessFlags::None ? ResourceAccessFlags::Write : accessFlags,
		1u, 1u,
		data, stride
	)
{ }

/// <summary>
/// Updates texture with contents of a scratch image, assuming compatible formats.
/// Allocates new Texture2D if the dimensions aren't the same.
/// </summary>
void StagingTexture2D::SetTextureWIC(ContextBase& ctx, wstring_view file, DirectX::ScratchImage& buffer)
{
	LoadImageWIC(file, buffer);
	const Image& img = *buffer.GetImage(0, 0, 0);
	const uint pixStride = 4 * sizeof(byte);
	const ivec2 dim(img.width, img.height);
	const uint totalBytes = pixStride * dim.x * dim.y;
	Span srcBytes(img.pixels, totalBytes);

	SetTextureData(ctx, srcBytes, pixStride, dim);
}

/// <summary>
/// Updates texture with contents of an arbitrary pixel data buffer, assuming compatible formats.
/// Allocates new Texture2D if the dimensions aren't the same.
/// </summary>
void StagingTexture2D::SetTextureData(ContextBase& ctx, const IDynamicArray<byte>& src, uint pixStride, ivec2 srcDim)
{
	const ivec2 dstSize = GetSize();

	if (srcDim.x <= dstSize.x && srcDim.y <= dstSize.y)
	{
		ctx.SetTextureData(*this, src, pixStride, srcDim);
	}
	else
	{
		pRes.Reset();
		*this = std::move(StagingTexture2D(
			GetDevice(),
			srcDim,
			(void*)src.GetData(),
			(UINT)pixStride,
			GetFormat(),
			desc.MipLevels,
			GetAccessFlags()
		));
	}
}

/// <summary>
/// Returns a temporary handle to a mapped buffer backing the texture
/// </summary>
Tex2DBufferHandle StagingTexture2D::GetBufferHandle(Context& ctx)
{
	D3D11_MAP mapFlags = (D3D11_MAP)0;

	if ((int)(GetAccessFlags() & ResourceAccessFlags::Read))
		mapFlags = (D3D11_MAP)(mapFlags | D3D11_MAP_READ);

	if ((int)(GetAccessFlags() & ResourceAccessFlags::Write))
		mapFlags = (D3D11_MAP)(mapFlags | D3D11_MAP_WRITE);

	D3D11_MAPPED_SUBRESOURCE msr;
	D3D_CHECK_HR(ctx->Map(
		pRes.Get(),
		0u,
		mapFlags,
		0u,
		&msr
	));

	pixelStride = msr.RowPitch / GetSize().x;
	return Tex2DBufferHandle(this, msr);
}

/// <summary>
/// Returns and unmaps a previously acquired buffer handle accessed using GetBufferHandle()
/// </summary>
void StagingTexture2D::ReturnBufferHandle(Context& ctx, Tex2DBufferHandle&& handle)
{
	D3D_CHECK_MSG(handle.pParent != nullptr,
		"Cannot return a null Texture Buffer Handle");

	D3D_CHECK_MSG(handle.pParent == this,
		"Cannot return a Texture Buffer Handle to an object"
		"that is not its parent");

	ctx->Unmap(pRes.Get(), 0u);
}

void StagingTexture2D::WriteToFileWIC(Context& ctx, string_view path)
{
	wstring widePath = GetWideString_UTF8_TO_UTF16LE(path);
	WriteToFileWIC(ctx, widePath);
}

void StagingTexture2D::WriteToFileWIC(Context& ctx, wstring_view path)
{
	const ivec2 size = GetSize();
	Tex2DBufferHandle buffer = GetBufferHandle(ctx);
	Image imageData = 
	{
		(uint)size.x, (uint)size.y,
		desc.Format,
		buffer.GetRowPitch(),
		buffer.GetByteSize(),
		buffer.GetData()
	};

	D3D_CHECK_HR(SaveToWICFile(
		imageData, 
		WIC_FLAGS_FORCE_SRGB,
		GetWICCodec(WIC_CODEC_PNG), 
		path.data()
	));

	ReturnBufferHandle(ctx, std::move(buffer));
}

Tex2DBufferHandle::Tex2DBufferHandle() :
	DataBufferSpan(),
	canRead(false),
	canWrite(false),
	msr({})
{ }

Tex2DBufferHandle::Tex2DBufferHandle(StagingTexture2D* pParent, D3D11_MAPPED_SUBRESOURCE msr) :
	DataBufferSpan(pParent, (byte*)msr.pData, msr.RowPitch* pParent->GetSize().y),
	canRead((int)(pParent->GetAccessFlags()& ResourceAccessFlags::Read)),
	canWrite((int)(pParent->GetAccessFlags()& ResourceAccessFlags::Write)),
	msr(msr)
{ }

/// <summary>
/// Returns the number of bytes in a row
/// </summary>
size_t Tex2DBufferHandle::GetRowPitch() const
{
	return msr.RowPitch;
}

/// <summary>
/// Returns the size of the buffer in bytes
/// </summary>
size_t Tex2DBufferHandle::GetByteSize() const
{
	return msr.RowPitch * pParent->GetSize().y;
}