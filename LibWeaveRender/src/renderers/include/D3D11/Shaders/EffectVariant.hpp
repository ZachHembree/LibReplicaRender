#pragma once
#include "WeaveEffects/ShaderDataHandles.hpp"
#include "../CtxBase.hpp"
#include "../Resources/ResourceBase.hpp"
#include "../Resources/ResourceSet.hpp"

namespace Weave::D3D11
{
	using Effects::EffectDef;

	class Sampler;
	class ShaderVariantManager;
	class ShaderVariantBase;

	class EffectVariant
	{
	public:
		EffectVariant();

		EffectVariant(
			ShaderVariantManager& lib,
			EffectDefHandle effectDef
		);

		/// <summary>
		/// Returns the name ID of the effect
		/// </summary>
		uint GetNameID() const;

		/// <summary>
		/// Returns the number of passes in the effect
		/// </summary>
		uint GetPassCount() const;

		/// <summary>
		/// Returns the number of shaders in the given pass
		/// </summary>
		uint GetShaderCount(const int pass) const;

		void Setup(CtxBase& ctx, int pass, const ResourceSet& res) const;

	private:
		struct EffectPass
		{
			UniqueArray<const ShaderVariantBase*> shaders;
			CtxBase::ActiveShaderMask activeStages;
		};

		EffectDefHandle def;
		UniqueArray<EffectPass> passes;
	};
}
