#include "pch.hpp"
#include <DirectXTex.h>
#include "D3D11/InternalD3D11.hpp"
#include "D3D11/Device.hpp"
#include "D3D11/Resources/ResizeableTexture2D.hpp"

using namespace DirectX;
using namespace Weave;
using namespace Weave::D3D11;

ResizeableTexture2D::ResizeableTexture2D() :
	renderScale(1),
	renderOffset(0)
{ }

ResizeableTexture2D::ResizeableTexture2D(
	Device& dev,
	ivec2 dim,
	Formats format,
	ResourceUsages usage,
	ResourceBindFlags bindFlags,
	ResourceAccessFlags accessFlags,
	uint mipLevels,
	void* data,
	uint stride
) :
	Texture2D(
		dev,
		dim,
		format,
		usage,
		bindFlags,
		accessFlags,
		mipLevels,
		data, stride
	),
	renderScale(1),
	renderOffset(0)
{ }

ResizeableTexture2D::ResizeableTexture2D(
	Device& dev,
	ivec2 dim,
	void* data,
	uint stride,
	Formats format,
	uint mipLevels
) :
	ResizeableTexture2D(
		dev,
		dim,
		format,
		ResourceUsages::Default,
		ResourceBindFlags::ShaderResource,
		ResourceAccessFlags::None,
		1u,
		data, stride
	)
{ }

ResizeableTexture2D::ResizeableTexture2D(
	Device& dev,
	Formats format,
	ivec2 dim,
	uint mipLevels,
	bool isDynamic
) :
	ResizeableTexture2D(
		dev,
		dim,
		format,
		isDynamic ? ResourceUsages::Dynamic : ResourceUsages::Default,
		ResourceBindFlags::ShaderResource,
		isDynamic ? ResourceAccessFlags::Write : ResourceAccessFlags::None,
		1u,
		nullptr, 0u
	)
{ }

/// <summary>
/// Sets the offset for this target in pixels
/// </summary>
void ResizeableTexture2D::SetRenderOffset(ivec2 offset)
{
	this->renderOffset = glm::clamp(vec2(offset) / vec2(GetSize()), vec2(-1), vec2(1));
}

/// <summary>
/// Returns the offset set for this target in pixels
/// </summary>
ivec2 ResizeableTexture2D::GetRenderOffset() const
{
	renderOffset = glm::clamp(renderOffset, vec2(0), 1.0f - renderScale);
	return ivec2(glm::round(renderOffset * vec2(GetSize())));
}

void ResizeableTexture2D::SetRenderSize(ivec2 renderSize)
{
	const vec2 size = glm::max(GetSize(), ivec2(1));
	vec2 newSize = glm::clamp(vec2(renderSize), vec2(1), size);
	SetRenderScale(newSize / size);
}

/// <summary>
/// Returns the size of the render area in pixels
/// </summary>
ivec2 ResizeableTexture2D::GetRenderSize() const
{
	renderScale = glm::clamp(renderScale, vec2(0), 1.0f - renderOffset);

	const vec2 size = GetSize();
	vec2 renderSize = glm::round(renderScale * size);
	return glm::clamp(ivec2(renderSize), ivec2(1), ivec2(size));
}

/// <summary>
/// Returns combined scaled (DRS) texel size and dim fp vector.
/// XY == Texel Size; ZW == Dim
/// </summary>
vec4 ResizeableTexture2D::GetRenderTexelSize() const
{
	const vec2 renderSize = GetRenderSize();
	return vec4(1.0f / renderSize.x, 1.0f / renderSize.y, renderSize);
}

/// <summary>
/// Sets the renderSize to size ratio on (0, 1].
/// </summary>
void ResizeableTexture2D::SetRenderScale(vec2 scale)
{
	scale = glm::round(scale * 1E6f) * 1E-6f;
	renderScale = glm::clamp(scale, vec2(1E-6f), vec2(1));
}

/// <summary>
/// Returns the renderSize to size ratio on (0, 1].
/// </summary>
vec2 ResizeableTexture2D::GetRenderScale() const
{
	return renderScale;
}

void ResizeableTexture2D::SetTextureData(ContextBase& ctx, const IDynamicArray<byte>& src, uint pixStride, ivec2 srcDim)
{
	const ivec2 currentSize = GetSize();

	if (srcDim.x <= currentSize.x && srcDim.y <= currentSize.y)
	{
		ctx.SetTextureData(*this, src, pixStride, srcDim, GetRenderOffset());
	}
	else
	{
		pRes.Reset();
		*this = std::move(ResizeableTexture2D(
			GetDevice(),
			srcDim,
			GetFormat(),
			GetUsage(),
			GetBindFlags(),
			GetAccessFlags(),
			desc.MipLevels,
			(void*)src.GetData(), (uint)pixStride
		));
	}
}

ResizeableTexture2D ResizeableTexture2D::FromImageWIC(Device& dev, wstring_view file)
{
	ScratchImage buf;
	LoadImageWIC(file, buf);

	if (buf.GetImage(0, 0, 0)->format != DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		ScratchImage converted;
		DirectX::Convert(*buf.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);
		buf = std::move(converted);
	}

	const Image& img = *buf.GetImage(0, 0, 0);

	return ResizeableTexture2D(dev,
		ivec2(img.width, img.height),
		img.pixels,
		4 * sizeof(uint8_t),
		Formats::R8G8B8A8_UNORM
	);
}