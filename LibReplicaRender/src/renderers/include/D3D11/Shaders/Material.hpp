#pragma once
#include "ShaderInstanceBase.hpp"

namespace Replica::D3D11
{
	class ShaderLibrary;
	class EffectVariant;
	class ResourceSet;

	class Material : public ShaderInstanceBase
	{
	public:		
		using ShaderInstanceBase::ShaderInstanceBase;

		/// <summary>
		/// Returns the number of passes in the effect
		/// </summary>
		uint GetPassCount() const;

		/// <summary>
		/// Returns the number of shaders specified for a given pass
		/// </summary>
		uint GetShaderCount(int pass) const;

		/// <summary>
		/// Prepares shaders and resources for the given pass before a draw call
		/// </summary>
		void Setup(Context& ctx, int pass);

		/// <summary>
		/// Cleans up state of the given pass
		/// </summary>
		void Reset(Context& ctx, int pass);

	private:
		mutable const EffectVariant* pEffect;

		const EffectVariant& GetEffect() const;

		void SetVariantID(int vID) override;
	};
}