#include "pch.hpp"
#include "D3D11/Shaders/Material.hpp"
#include "D3D11/Shaders/EffectVariant.hpp"
#include "D3D11/ShaderLibrary.hpp"
#include "D3D11/Resources/ResourceSet.hpp"

using namespace Replica;
using namespace Replica::D3D11;

uint Material::GetPassCount() const { return GetEffect().GetPassCount(); }

uint Material::GetShaderCount(int pass) const { return GetEffect().GetShaderCount(pass); }

void Material::Setup(Context& ctx, int pass) { GetEffect().Setup(ctx, pass, *pRes); }

void Material::Reset(Context& ctx, int pass) { GetEffect().Reset(ctx, pass); }

const EffectVariant& Material::GetEffect() const 
{ 
	GFX_ASSERT(nameID != -1, "Cannot access null effect");

	if (pEffect == nullptr)
	{
		const uint effectID = pLib->GetLibMap().TryGetEffectID(nameID, vID);
		pEffect = &pLib->GetEffect(effectID);
	}

	return *pEffect; 
}

void Material::SetVariantID(int vID)
{
	if (vID != this->vID)
	{ 
		const uint effectID = pLib->GetLibMap().TryGetEffectID(nameID, vID);
		pEffect = &pLib->GetEffect(effectID);
		this->vID = vID;
	}
}