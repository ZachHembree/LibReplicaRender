#include "pch.hpp"
#include "ShaderLibMap.hpp"
#include "ShaderLibGen/ShaderRegistryMap.hpp"

/* Variant ID generation

� Definitions:
	Let { F, M, Vid, Fc, Mc, Fv, Vc } be positive integers.
	F, M, and Vid represent flags, mode, and variant ID, respectively.
	Fc and Mc denote the flag count and mode count, respectively.
	Fv is the flag variant count, where Fv = 2 ^ Fc.
	Vc is the total variant count, where Vc = Fv * Mc.
	Define F = (Vid % Fv) and M = floor(Vid / Fv) <=> Vid = F + (M * Fv).

� Storage Details:

	Flag (F) names and mode (M) names are stored in contiguous arrays, maintaining their original 
	order. This ensures the correct bit order and provides counts Fc and Mc. Using these, Fv can be 
	recalculated to support ID <=> Flag/Mode conversion.

	Fv = 2 ^ Fc
	F(Vid) = (Vid % Fv)
	M(Vid) = floor(Vid / Fv)
	Vid(F, M) = F + (M * Fv)
*/

namespace Replica::Effects
{
	ShaderLibMap::ShaderLibMap()
	{ }

	ShaderLibMap::ShaderLibMap(const ShaderLibDef& def) : 
		pRegMap(new ShaderRegistryMap(def.regData)),
		platform(def.platform),
		flagNames(def.flagNames),
		modeNames(def.modeNames),
		variants(def.variants)
	{
		InitMap();
	}

	ShaderLibMap::ShaderLibMap(ShaderLibDef&& def) : 
		pRegMap(new ShaderRegistryMap(std::move(def.regData))),
		platform(std::move(def.platform)),
		flagNames(std::move(def.flagNames)),
		modeNames(std::move(def.modeNames)),
		variants(std::move(def.variants))
	{
		InitMap();
	}

	ShaderLibMap::~ShaderLibMap() = default;

	void ShaderLibMap::InitMap()
	{
		// Initialize tables
		// Compile flags
		for (int i = 0; i < flagNames.GetLength(); i++)
		{
			const string& name = flagNames[i];
			flagNameMap.emplace(name, i);
		}
		// Shader modes
		for (int i = 0; i < modeNames.GetLength(); i++)
		{
			const string& name = modeNames[i];
			modeNameMap.emplace(name, i);
		}

		// Variant lookup tables
		variantMaps = UniqueArray<VariantNameMap>(variants.GetLength());

		for (int vID = 0; vID < variants.GetLength(); vID++)
		{
			const auto& variant = variants[vID];
			variantMaps[vID] = VariantNameMap();

			// Shaders
			for (int i = 0; i < variant.shaders.GetLength(); i++)
			{
				const ShaderVariantDef& vidPair = variant.shaders[i];
				const ShaderDef& shader = pRegMap->GetShader(vidPair.shaderID);
				NameIndexMap& map = variantMaps[vID].nameShaderMap;
				map.emplace(GetString(shader.nameID), i);
			}

			// Effects
			for (int i = 0; i < variant.effects.GetLength(); i++)
			{
				const EffectVariantDef& vidPair = variant.effects[i];
				const EffectDef& effect = pRegMap->GetEffect(vidPair.effectID);
				NameIndexMap& map = variantMaps[vID].effectNameMap;
				map.emplace(GetString(effect.nameID), i);
			}
		}
	}

	string_view ShaderLibMap::GetString(uint stringID) const { return pRegMap->GetString(stringID); }

	/// <summary>
	/// Returns the shader by shaderID and variantID
	/// </summary>
	ShaderDefHandle ShaderLibMap::GetShader(int shaderID, int vID) const
	{ 
		REP_ASSERT_MSG(shaderID >= 0 && shaderID < GetVariant(vID).shaders.GetLength(), "Shader ID invalid");
		const ShaderVariantDef& def = GetVariant(vID).shaders[shaderID];
		return ShaderDefHandle(*pRegMap, def.shaderID);
	}

	/// <summary>
	/// Returns the effect with the given effectID and variantID
	/// </summary>
	EffectDefHandle ShaderLibMap::GetEffect(int effectID, int vID) const
	{ 
		REP_ASSERT_MSG(effectID >= 0 && effectID < GetVariant(vID).effects.GetLength(), "Effect ID invalid");
		const EffectVariantDef& def = GetVariant(vID).effects[effectID];
		return EffectDefHandle(*pRegMap, def.effectID);
	}

	int ShaderLibMap::TryGetShaderID(string_view name, int vID) const
	{
		const NameIndexMap& nameMap = variantMaps[vID].nameShaderMap;
		const auto& it = nameMap.find(name);

		if (it != nameMap.end())
		{
			const VariantDef& variantDef = GetVariant(vID);
			const int shaderID = it->second;
			return shaderID;
		}
		else
			return -1;
	}

	int ShaderLibMap::TryGetEffectID(string_view name, int vID) const
	{
		const NameIndexMap& nameMap = variantMaps[vID].effectNameMap;
		const auto& it = nameMap.find(name);

		if (it != nameMap.end())
		{
			const VariantDef& variantDef = GetVariant(vID);
			const int effectID = it->second;
			return effectID;
		}
		else
			return -1;
	}

	int ShaderLibMap::TryGetMode(const string_view name) const
	{
		const auto& result = modeNameMap.find(name);

		if (result != modeNameMap.end())
			return (int)(result->second);

		return -1;
	}

	size_t ShaderLibMap::GetFlagVariantCount() const { return (1ull << flagNames.GetLength()); }

	size_t ShaderLibMap::GetModeCount() const { return modeNames.GetLength(); }

	size_t ShaderLibMap::GetVariantCount() const { return modeNames.GetLength() * GetFlagVariantCount(); }

	size_t ShaderLibMap::GetShaderCount(int vID) const { return GetVariant(vID).shaders.GetLength(); }

	size_t ShaderLibMap::GetEffectCount(int vID) const { return GetVariant(vID).effects.GetLength(); }

	int ShaderLibMap::GetFlagID(const int variantID) const
	{
		PARSE_ASSERT_MSG(variantID >= 0, "Invalid variant ID");
		return variantID % (int)GetFlagVariantCount();
	}

	int ShaderLibMap::GetModeID(const int variantID) const
	{
		PARSE_ASSERT_MSG(variantID >= 0, "Invalid variant ID");
		return variantID / (int)GetFlagVariantCount();
	}

	int ShaderLibMap::GetVariantID(const int flagID, const int modeID) const
	{
		PARSE_ASSERT_MSG(flagID >= 0 && modeID >= 0, "Invalid flag or mode ID");
		return flagID + (modeID * (int)GetFlagVariantCount());
	}

	bool ShaderLibMap::GetIsDefined(string_view name, const int vID) const
	{
		const auto flagIt = flagNameMap.find(name);

		if (flagIt != flagNameMap.end())
		{
			const int flags = GetFlagID(vID);	
			return (flags & (1 << flagIt->second)) > 0;
		}
		else
		{
			const auto modeIt = modeNameMap.find(name);

			if (modeIt != modeNameMap.end())
			{
				const int mode = GetModeID(vID);
				return mode == modeIt->second;
			}
			else
				return false;
		}
	}

	const VariantDef& ShaderLibMap::GetVariant(const int vID) const
	{
		REP_ASSERT_MSG(vID >= 0 && vID < variants.GetLength(), "Variant ID invalid");
		return variants[vID];
	}
}
