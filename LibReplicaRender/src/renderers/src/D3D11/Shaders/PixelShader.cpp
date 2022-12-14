#include <d3dcompiler.h>
#include "ReplicaInternalD3D11.hpp"

using namespace Replica::D3D11;
using namespace Microsoft::WRL;

PixelShader::PixelShader()
{ }

PixelShader::PixelShader(Device& dev, const PixelShaderDef& psDef) :
	ShaderBase(dev, psDef)
{
	if (psDef.file.size() > 0)
	{
		ComPtr<ID3DBlob> psBlob;
		GFX_THROW_FAILED(D3DReadFileToBlob(psDef.file.data(), &psBlob));
		GFX_THROW_FAILED(dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pPS));
	}
	else if (psDef.srcSize > 0)
	{
		GFX_THROW_FAILED(dev->CreatePixelShader(psDef.srcBin, psDef.srcSize, nullptr, &pPS));
	}
	else
		GFX_THROW("A shader cannot be instantiated without valid source.")
}

ID3D11PixelShader* PixelShader::Get() const
{
	return pPS.Get();
}

void PixelShader::Bind(Context& ctx)
{
	if (!ctx.GetIsPsBound(this))
	{ 
		const auto& ss = samplers.GetResources();
		const auto& tex = textures.GetResources();

		ctx->PSSetSamplers(0, (UINT)ss.GetLength(), ss.GetPtr());
		ctx->PSSetShaderResources(0, (UINT)tex.GetLength(), tex.GetPtr());
		ctx->PSSetConstantBuffers(0u, 1, cBuf.GetAddressOf());

		this->pCtx = &ctx;
		ctx.SetPS(this);
		isBound = true;
	}

	if (ctx.GetIsPsBound(this))
	{
		constants.UpdateConstantBuffer(cBuf, ctx);
	}
}

void PixelShader::Unbind()
{
	if (isBound && pCtx->GetIsPsBound(this))
	{ 
		isBound = false;
		pCtx->SetPS(nullptr);
		ID3D11DeviceContext& ctx = pCtx->Get();

		ID3D11SamplerState* nullSamp[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT](nullptr);
		ctx.PSSetSamplers(0, samplers.GetCount(), nullSamp);

		ID3D11ShaderResourceView* nullSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT](nullptr);
		ctx.PSSetShaderResources(0, textures.GetCount(), nullSRV);

		ID3D11Buffer* nullCB[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT](nullptr);
		ctx.PSSetConstantBuffers(0, 1, nullCB);

		pCtx = nullptr;
	}	
}