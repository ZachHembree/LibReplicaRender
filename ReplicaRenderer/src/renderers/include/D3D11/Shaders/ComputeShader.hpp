#pragma once
#include "D3D11/Shaders/ShaderBase.hpp"

namespace Replica::D3D11
{
	struct ComputeShaderDef : public ShaderDefBase
	{ };

	class ComputeShader : public ShaderBase
	{
	public:
		using ShaderBase::Bind;
		using ShaderBase::SetSampler;
		using ShaderBase::SetTexture;

		ComputeShader();

		ComputeShader(Device& dev, const ComputeShaderDef& csDef);

		ID3D11ComputeShader* Get() const;

		/// <summary>
		/// Dispatch the shader in the given context with the given number of
		/// groups. Automatically binds the shader as needed.
		/// </summary>
		void Dispatch(Context& ctx, ivec3 groups);

		/// <summary>
		/// Binds the compute shader using the given context
		/// </summary>
		void Bind(Context& ctx) override;

		/// <summary>
		/// Unbinds compute shader
		/// </summary>
		void Unbind() override;

	private:
		ComPtr<ID3D11ComputeShader> pCS;
		ResourceMap<ID3D11UnorderedAccessView> uavBuffers;
	};
}