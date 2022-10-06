#pragma once
#include "Texture2D.hpp"

namespace Replica::D3D11
{
	class ResizeableTexture2D : public virtual IResizeableTexture2D, public Texture2D
	{
	public:
		/// <summary>
		/// Constructs and alocates a texture initialized with the given data with either
		/// Default or Immutable usage, depending whether the read-only flag is set.
		/// </summary>
		template<typename T>
		ResizeableTexture2D(
			Device& dev,
			ivec2 dim,
			IDynamicArray<T> data,
			Formats format = Formats::R8G8B8A8_UNORM,
			uint mipLevels = 1u
		)
			: ResizeableTexture2D(dev, dim, data.GetPtr(), sizeof(T), format, mipLevels)
		{ }

		/// <summary>
		/// Constructs and alocates a texture initialized with the given data with either
		/// Default or Immutable usage, depending whether the read-only flag is set.
		/// </summary>
		ResizeableTexture2D(
			Device& dev,
			ivec2 dim,
			void* data,
			uint stride,
			Formats format = Formats::R8G8B8A8_UNORM,
			uint mipLevels = 1u
		);

		/// <summary>
		/// Constructs and alocates an empty texture with default or dynamic usage.
		/// </summary>
		ResizeableTexture2D(
			Device& dev,
			Formats format = Formats::R8G8B8A8_UNORM,
			ivec2 dim = ivec2(0),
			uint mipLevels = 1u,
			bool isDynamic = false
		);

		/// <summary>
		/// Constructs an uninitialized texture object
		/// </summary>
		ResizeableTexture2D();

		/// <summary>
		/// Sets the offset for this target in pixels
		/// </summary>
		void SetRenderOffset(ivec2 offset);

		/// <summary>
		/// Returns the offset set for this target in pixels
		/// </summary>
		ivec2 GetRenderOffset() const override;

		/// <summary>
		/// Sets the size of the render area for the render texture.
		/// Cannot exceed the size of the underlying buffer.
		/// </summary>
		void SetRenderSize(ivec2 renderSize);

		/// <summary>
		/// Returns the size of the render area in pixels
		/// </summary>
		ivec2 GetRenderSize() const override;

		/// <summary>
		/// Returns combined scaled (DRS) texel size and dim fp vector.
		/// XY == Texel Size; ZW == Dim
		/// </summary>
		vec4 GetRenderTexelSize() const;

		/// <summary>
		/// Sets the renderSize to size ratio on (0, 1].
		/// </summary>
		void SetRenderScale(vec2 scale);

		/// <summary>
		/// Returns the renderSize to size ratio on (0, 1].
		/// </summary>
		vec2 GetRenderScale() const override;

		/// <summary>
		/// Initializes new Texture2D from WIC-compatible image 
		/// (BMP, GIF, ICO, JPEG, PNG, TIFF)
		/// </summary>
		static ResizeableTexture2D FromImageWIC(
			Device& dev,
			wstring_view file
		);

		/// <summary>
		/// Updates texture with contents of an arbitrary pixel data buffer, assuming compatible formats.
		/// Allocates new Texture2D if the dimensions aren't the same.
		/// </summary>
		template<typename T>
		void SetTextureData(Context& ctx, IDynamicArray<T>& data, ivec2 dim)
		{
			SetTextureData(ctx, data.GetPtr(), sizeof(T), dim);
		}

		/// <summary>
		/// Updates texture with contents of an arbitrary pixel data buffer, assuming compatible formats.
		/// Allocates new Texture2D if the dimensions aren't the same.
		/// </summary>
		void SetTextureData(Context& ctx, void* data, size_t stride, ivec2 dim) override;

	protected:
		mutable vec2 renderOffset;
		mutable vec2 renderScale;

		ResizeableTexture2D(
			Device& dev,
			ivec2 dim,
			Formats format = Formats::R8G8B8A8_UNORM,
			ResourceUsages usage = ResourceUsages::Default,
			ResourceBindFlags bindFlags = ResourceBindFlags::ShaderResource,
			ResourceAccessFlags accessFlags = ResourceAccessFlags::None,
			uint mipLevels = 1u,
			void* data = nullptr,
			uint stride = 0
		);

		void UpdateMapUnmap(Context& ctx, void* data, size_t stride, ivec2 dim);

		void UpdateSubresource(Context& ctx, void* data, size_t stride, ivec2 dim);
	};
}