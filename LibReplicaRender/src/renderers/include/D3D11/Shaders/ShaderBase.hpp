#pragma once
#include "../Resources/DeviceChild.hpp"
#include "../Resources/InputLayout.hpp"
#include "../Resources/ConstantBuffer.hpp"
#include "../Resources/ConstantMap.hpp"
#include "../ResourceMap.hpp"

namespace Replica::D3D11
{
	class Sampler;
	class Texture2D;
	class ConstantBuffer;
	class ConstantMap;
	class ConstantMapDef;

	struct ShaderDefBase
	{
		wstring_view file;
		byte* srcBin;
		size_t srcSize;

		ConstantMapDef constMap;
		ResourceMap<ID3D11SamplerState> samplerMap;
		ResourceMap<ID3D11ShaderResourceView> textureMap;
	};

	class ShaderBase : public DeviceChild
	{
	public:
		/// <summary>
		/// Returns true if the shader is bound
		/// </summary>
		bool GetIsBound();

		/// <summary>
		/// Binds shader to the main context
		/// </summary>
		virtual void Bind();

		/// <summary>
		/// Binds the shader to the given context
		/// </summary>
		virtual void Bind(Context& ctx) = 0;

		/// <summary>
		/// Unbinds the shader from the last context
		/// </summary>
		virtual void Unbind() = 0;

		/// <summary>
		/// Sets the value corresponding to the given name to the
		/// given value.
		/// </summary>
		template<typename T>
		void SetConstant(string_view name, const T& value)
		{
			constants.SetMember(name, value);
		}

		/// <summary>
		/// Sets the value corresponding to the given name to the
		/// given value.
		/// </summary>
		template<>
		void SetConstant<mat4>(string_view name, const mat4& value)
		{
			constants.SetMember(name, transpose(value));
		}

		/// <summary>
		/// Sets the value corresponding to the given name to the
		/// given value.
		/// </summary>
		template<>
		void SetConstant<mat3>(string_view name, const mat3& value)
		{
			constants.SetMember(name, transpose(value));
		}

		/// <summary>
		/// Sets sampler using last context
		/// </summary>
		virtual void SetSampler(string_view name, Sampler& samp);

		/// <summary>
		/// Sets Texture2D using last context
		/// </summary>
		virtual void SetTexture(string_view name, ITexture2D& tex);

	protected:
		ConstantMap constants;
		ResourceMap<ID3D11SamplerState> samplers;
		ResourceMap<ID3D11ShaderResourceView> textures;

		ConstantBuffer cBuf;
		Context* pCtx;
		bool isBound;

		ShaderBase();

		ShaderBase(Device& dev);

		ShaderBase(Device& dev, const ShaderDefBase& def);

		ShaderBase(ShaderBase&& other) noexcept;

		ShaderBase& operator=(ShaderBase&& other) noexcept;
	};
}
