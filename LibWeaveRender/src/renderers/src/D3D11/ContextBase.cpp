#include "pch.hpp"
#include "WeaveEffects/ShaderData.hpp"
#include "D3D11/ContextUtils.hpp"
#include "D3D11/ContextBase.hpp"
#include "D3D11/ContextState.hpp"
#include "D3D11/Viewport.hpp"
#include "D3D11/Primitives.hpp"
#include "D3D11/Resources/ResourceSet.hpp"
#include "D3D11/Resources/VertexBuffer.hpp"
#include "D3D11/Shaders/ShaderVariants.hpp"
#include "D3D11/Shaders/Material.hpp"

using namespace Weave;
using namespace Weave::D3D11;

ContextBase::ContextBase() = default;

ContextBase::ContextBase(Device& dev, ComPtr<ID3D11DeviceContext>&& pContext) :
	DeviceChild(dev),
	pContext(std::move(pContext)),
	pState(new ContextState())
{
	pState->Init();
}

ContextBase::ContextBase(ContextBase&&) noexcept = default;

ContextBase& ContextBase::operator=(ContextBase&&) noexcept = default;

void ContextBase::BindDepthStencilBuffer(IDepthStencil& depthStencil) 
{
	if (pState->pDepthStencil != &depthStencil)
	{
		pState->pDepthStencil = &depthStencil;
		pContext->OMSetDepthStencilState(depthStencil.GetState(), 1u);
		pContext->OMSetRenderTargets(pState->rtCount, pState->renderTargets.GetData(), depthStencil.GetDSV());
	}
}

void ContextBase::UnbindDepthStencilBuffer() 
{
	if (pState->pDepthStencil != nullptr)
	{
		pState->pDepthStencil = nullptr;
		pContext->OMSetDepthStencilState(nullptr, 1);
		pContext->OMSetRenderTargets(pState->rtCount, pState->renderTargets.GetData(), nullptr);
	}
}

uint ContextBase::GetRenderTargetCount() const { return pState->rtCount; }

void ContextBase::BindRenderTarget(IRenderTarget& rt, IDepthStencil& ds, sint slot)
{
	Span rtSpan(&rt);
	BindRenderTargets(rtSpan, &ds, slot);
}

void ContextBase::BindRenderTarget(IRenderTarget& rt, sint slot)
{
	Span rtSpan(&rt);
	BindRenderTargets(rtSpan, slot);
}

/// <summary>
/// Updates RTV state cache and returns true if any changes were made
/// </summary>
static bool TryUpdateRTV(ContextState& state, IDynamicArray<IRenderTarget>& rts, sint offset)
{
	bool wasChanged = false;
	const uint newCount = (uint)rts.GetLength();

	if ((offset + newCount) > state.rtCount)
	{
		state.rtCount = offset + newCount;
		wasChanged = true;
	}

	Span rtvState(&state.renderTargets[offset], newCount);

	for (uint i = 0; i < newCount; i++)
	{
		ID3D11RenderTargetView* pRTV = rts[i].GetRTV();

		if (pRTV != rtvState[i])
		{
			rtvState[i] = pRTV;
			wasChanged = true;
		}
	}

	return wasChanged;
}

static bool TryUpdateViewports(ContextState& state, const IDynamicArray<IRenderTarget>& rts, sint offset)
{
	// Update viewports for render textures
	Span vpState(&state.viewports[offset], rts.GetLength());
	const vec2 depthRange = state.pDepthStencil != nullptr ? state.pDepthStencil->GetRange() : vec2(0);
	const uint newCount = (uint)rts.GetLength();
	bool wasChanged = false;

	for (uint i = 0; i < newCount; i++)
	{
		Viewport vp
		{
			.offset = rts[i].GetRenderOffset(),
			.size = rts[i].GetRenderSize(),
			.zDepth = depthRange
		};

		if (vp != vpState[i])
		{
			vpState[i] = vp;
			wasChanged = true;
		}
	}

	state.vpCount = std::max(state.vpCount, (uint)(offset + newCount));
	return wasChanged;
}

void ContextBase::BindRenderTargets(IDynamicArray<IRenderTarget>& rts, sint startSlot)
{
	D3D_ASSERT_MSG(rts.GetLength() <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		"Number of render targets supplied exceeds limit.");

	if (TryUpdateRTV(*pState, rts, startSlot))
		pContext->OMSetRenderTargets(pState->rtCount, pState->renderTargets.GetData(), pState->GetDepthStencilView());

	if (TryUpdateViewports(*pState, rts, startSlot))
		pContext->RSSetViewports(pState->vpCount, (D3D11_VIEWPORT*)pState->viewports.GetData());
}

void ContextBase::BindRenderTargets(IDynamicArray<IRenderTarget>& rts, IDepthStencil* pDS, sint startSlot)
{
	D3D_ASSERT_MSG(rts.GetLength() <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		"Number of render targets supplied exceeds limit.");

	bool wasDsChanged = false;

	if (pDS != pState->pDepthStencil)
	{
		ID3D11DepthStencilState* pDSS = pDS != nullptr ? pDS->GetState() : nullptr;
		pState->pDepthStencil = pDS;
		pContext->OMSetDepthStencilState(pDSS, 1);
		wasDsChanged = true;
	}

	const bool wasRtvChanged = TryUpdateRTV(*pState, rts, startSlot);

	if (wasDsChanged || wasRtvChanged)
		pContext->OMSetRenderTargets(pState->rtCount, pState->renderTargets.GetData(), pState->GetDepthStencilView());
		
	if (TryUpdateViewports(*pState, rts, startSlot))
		pContext->RSSetViewports(pState->vpCount, (D3D11_VIEWPORT*)pState->viewports.GetData());
}

void ContextBase::UnbindRenderTarget(sint slot)
{
	UnbindRenderTargets(slot, 1);
}

void ContextBase::UnbindRenderTargets(sint startSlot, uint count)
{
	if (pState->rtCount == 0 || count == 0)
		return;

	if (count == (uint)-1)
	{
		startSlot = 0;
		count = pState->rtCount;
	}

	SetArrNull(pState->renderTargets, startSlot, count);
	
	if ((startSlot + count) == pState->rtCount)
		pState->rtCount = startSlot;

	if (pState->rtCount == 0)
	{
		pState->pDepthStencil = nullptr;
		pContext->OMSetDepthStencilState(nullptr, 1u);
		pContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	else
		pContext->OMSetRenderTargets(startSlot, pState->renderTargets.GetData(), pState->GetDepthStencilView());
}

void ContextBase::SetPrimitiveTopology(PrimTopology topology)
{
	if (topology != pState->topology)
	{
		pState->topology = topology;
		pContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
	}
}

void ContextBase::BindVertexBuffer(VertexBuffer& vertBuffer, sint slot)
{
	Span vbSpan(&vertBuffer);
	BindVertexBuffers(vbSpan, slot);
}

void ContextBase::BindVertexBuffers(IDynamicArray<VertexBuffer>& vertBuffers, sint startSlot)
{
	Span vbState(pState->vertexBuffers.GetData() + startSlot, vertBuffers.GetLength());
	const uint newCount = (uint)vertBuffers.GetLength();
	bool wasChanged = false;

	for (uint i = 0; i < newCount; i++)
	{
		ID3D11Buffer* pVB = vertBuffers[i].Get();

		if (pVB != vbState[i])
		{
			vbState[i] = pVB;
			wasChanged = true;
		}
	}

	if (wasChanged)
	{
		Span strideState(pState->vbStrides.GetData() + startSlot, newCount);
		Span offsetState(pState->vbOffsets.GetData() + startSlot, newCount);
		pState->vertBufCount = std::max(pState->vertBufCount, (uint)(startSlot + newCount));

		for (uint i = 0; i < newCount; i++)
		{
			strideState[i] = vertBuffers[i].GetStride();
			offsetState[i] = vertBuffers[i].GetOffset();
		}

		pContext->IASetVertexBuffers(startSlot, newCount, vbState.GetData(), strideState.GetData(), offsetState.GetData());
	}
}

void ContextBase::UnbindVertexBuffer(sint slot)
{
	UnbindVertexBuffers(slot, 1);
}

void ContextBase::UnbindVertexBuffers(sint startSlot, uint count)
{
	if (pState->vertBufCount == 0 || count == 0)
		return;

	if (count == (uint)-1)
	{
		startSlot = 0u;
		count = pState->vertBufCount;
	}

	SetArrNull(pState->vertexBuffers, startSlot, count);
	SetArrNull(pState->vbStrides, startSlot, count);
	SetArrNull(pState->vbOffsets, startSlot, count);
	
	if ((startSlot + count) == pState->vertBufCount)
		pState->vertBufCount = startSlot;

	pContext->IASetVertexBuffers(startSlot, count, pState->vertexBuffers.GetData(), pState->vbStrides.GetData(), pState->vbOffsets.GetData());
}

void ContextBase::BindIndexBuffer(IndexBuffer& indexBuffer, uint byteOffset)
{
	pState->pIndexBuffer = indexBuffer.Get();
	pContext->IASetIndexBuffer(pState->pIndexBuffer, (DXGI_FORMAT)Formats::R16_UINT, byteOffset);
}

void ContextBase::UnbindIndexBuffer()
{
	if (pState->pIndexBuffer != nullptr)
	{
		pState->pIndexBuffer = nullptr;
		pContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)Formats::R16_UINT, 0);
	}
}

void ContextBase::ClearRenderTarget(IRenderTarget& rt, vec4 color)
{
	pContext->ClearRenderTargetView(rt.GetRTV(), reinterpret_cast<float*>(&color));
}

bool ContextBase::GetIsBound(const ShaderVariantBase& shader) const
{
	const ShadeStages stage = shader.GetStage();
	return pState->GetStage(stage).pShader == &shader;
}

void ContextBase::BindResources(const ComputeShaderVariant& shader, const ResourceSet& resSrc)
{
	if (shader.GetUAVMap() != nullptr && resSrc.GetUAVs().GetLength() > 0)
	{
		const UnorderedAccessMap& uavMap = *shader.GetUAVMap();
		const UnorderedAccessMap::DataT& uavSrc = resSrc.GetUAVs();

		// Populate the state cache array based on the map and source resources
		pState->uavCount = (uint)uavMap.GetCount();
		uavMap.GetResources(uavSrc, pState->uavs);
		pContext->CSSetUnorderedAccessViews(0, pState->uavCount, pState->uavs.GetData(), nullptr);
	}
}

void ContextBase::BindResources(const VertexShaderVariant& shader, const ResourceSet& resSrc)
{
	ID3D11InputLayout* pLayout = shader.GetInputLayout().Get();
	pState->pInputLayout = pLayout;
	pContext->IASetInputLayout(pLayout);
}

static void ClearUAVs(ID3D11DeviceContext* pContext, uint count)
{
	if (count > 0)
	{
		D3D_ASSERT_MSG(count <= D3D11_1_UAV_SLOT_COUNT, "UAV slot count exceeded maximum.");
		ALLOCA_SPAN_NULL(nullRW, count, ID3D11UnorderedAccessView*);
		pContext->CSSetUnorderedAccessViews(0, count, nullRW.GetData(), nullptr);
	}
}

void ContextBase::BindShader(const ShaderVariantBase& shader, const ResourceSet& resSrc)
{
	const ShadeStages stage = shader.GetStage();
	ContextState::StageState& stageState = pState->GetStage(stage);

	if (stageState.pShader == &shader)
		return;
	else
		stageState.pShader = &shader;

	// Bind Shader Object
	SetShader(stage, pContext.Get(), shader.Get(), nullptr, 0);

	// Bind Constant Buffers (and upload data)
	if (shader.GetConstantMap() != nullptr && resSrc.GetConstantCount() > 0)
	{
		const ConstantGroupMap& constMap = *shader.GetConstantMap();
		const ConstantGroupMap::Data& constSrc = resSrc.GetMappedConstants(constMap);
		IDynamicArray<ConstantBuffer>& cbufs = shader.GetConstantBuffers();
		stageState.cbufCount = (uint)cbufs.GetLength();

		// Upload data to each ConstantBuffer object and store its ID3D11Buffer* in the state cache
		for (int i = 0; i < cbufs.GetLength(); i++)
		{
			SetBufferData(cbufs[i], constSrc[i]);
			stageState.cbuffers[i] = cbufs[i].Get();
		}

		SetConstantBuffers(stage, pContext.Get(), 0, stageState.cbufCount, stageState.cbuffers.GetData());
	}

	// Bind Samplers
	if (shader.GetSampMap() != nullptr && resSrc.GetSamplers().GetLength() > 0)
	{
		const SamplerMap& sampMap = *shader.GetSampMap();
		const SamplerMap::DataT& sampSrc = resSrc.GetSamplers();

		// Populate the state cache array based on the map and source resources
		stageState.sampCount = (uint)sampMap.GetCount();
		sampMap.GetResources(sampSrc, stageState.samplers);
		SetSamplers(stage, pContext.Get(), 0, stageState.sampCount, stageState.samplers.GetData());
	}

	// Bind Shader Resource Views (SRVs)
	if (shader.GetResViewMap() != nullptr && resSrc.GetSRVs().GetLength() > 0)
	{
		// Avoid RAW contention and force sync
		ClearUAVs(pContext.Get(), pState->uavCount);

		const ResourceViewMap& srvMap = *shader.GetResViewMap();
		const ResourceViewMap::DataT& srvSrc = resSrc.GetSRVs();

		// Populate the state cache array based on the map and source resources
		stageState.srvCount = (uint)srvMap.GetCount();
		srvMap.GetResources(srvSrc, stageState.resViews);
		SetShaderResources(stage, pContext.Get(), 0, stageState.srvCount, stageState.resViews.GetData());
	}

	// CS specific handling
	if (stage == ShadeStages::Compute)
		BindResources(static_cast<const ComputeShaderVariant&>(shader), resSrc);
	// VS specific handling
	else if (stage == ShadeStages::Vertex)
		BindResources(static_cast<const VertexShaderVariant&>(shader), resSrc);
}

void ContextBase::UnbindResources(const ComputeShaderVariant& shader)
{
	// Unbind Unordered Access Views (UAVs)
	if (shader.GetUAVMap() != nullptr)
	{
		const uint count = shader.GetUAVMap()->GetCount();
		SetArrNull(pState->uavs, count);
		pContext->CSSetUnorderedAccessViews(0, count, pState->uavs.GetData(), nullptr); // Use nullptr for initial counts
	}
}

void ContextBase::UnbindResources(const VertexShaderVariant& shader)
{
	pState->pInputLayout = nullptr;
	pContext->IASetInputLayout(pState->pInputLayout);
}

void ContextBase::UnbindShader(const ShaderVariantBase& shader)
{
	const ShadeStages stage = shader.GetStage();
	ContextState::StageState& stageState = pState->GetStage(stage);

	// State Check: Only unbind if this shader is the one currently active
	if (stageState.pShader == &shader)
	{
		// Unbind Constant Buffers
		if (shader.GetConstantMap() != nullptr)
		{
			SetArrNull(stageState.cbuffers, stageState.cbufCount);
			SetConstantBuffers(stage, pContext.Get(), 0, stageState.cbufCount, stageState.cbuffers.GetData());
		}

		// Unbind Samplers
		if (shader.GetSampMap() != nullptr)
		{
			SetArrNull(stageState.samplers, stageState.sampCount);
			SetSamplers(stage, pContext.Get(), 0, stageState.sampCount, stageState.samplers.GetData());
		}

		// Unbind Shader Resource Views (SRVs)
		if (shader.GetResViewMap() != nullptr)
		{
			SetArrNull(stageState.resViews, stageState.srvCount);
			SetShaderResources(stage, pContext.Get(), 0, stageState.srvCount, stageState.resViews.GetData());
		}

		if (stage == ShadeStages::Compute)
			UnbindResources(static_cast<const ComputeShaderVariant&>(shader));
		else if (stage == ShadeStages::Vertex)
			UnbindResources(static_cast<const VertexShaderVariant&>(shader));

		// Unbind Shader Object
		SetShader(stage, pContext.Get(), nullptr, nullptr, 0);
		stageState.pShader = nullptr;
	}
}

void ContextBase::UnbindStage(ShadeStages stage) 
{
	const ShaderVariantBase* pShader = pState->GetStage(stage).pShader;

	if (pShader != nullptr)
		UnbindShader(*pShader);
}

void ContextBase::Dispatch(const ComputeShaderVariant& cs, ivec3 groups, const ResourceSet& res) 
{
	BindShader(cs, res);
	pContext->Dispatch(groups.x, groups.y, groups.z);
}

void ContextBase::Draw(Mesh& mesh, Material& mat)
{
	Span meshSpan(&mesh);
	Draw(meshSpan, mat);
}

void ContextBase::Draw(IDynamicArray<Mesh>& meshes, Material& mat)
{
	for (Mesh& mesh : meshes)
	{
		mesh.Setup(*this);

		for (uint pass = 0; pass < mat.GetPassCount(); pass++)
		{
			mat.Setup(*this, pass);
			pContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
			mat.Reset(*this, pass);
		}
	}
}

static void WriteBufferSubresource(ID3D11DeviceContext& ctx, BufferBase& dst, const IDynamicArray<byte>& src)
{
	ctx.UpdateSubresource(dst.Get(), 0, nullptr, src.GetData(), 0, 0);
}

static void WriteBufferMapUnmap(ID3D11DeviceContext& ctx, BufferBase& dst, const IDynamicArray<byte>& src)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	D3D_CHECK_HR(ctx.Map(
		dst.Get(),
		0u,
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&msr
	));

	memcpy(msr.pData, src.GetData(), dst.GetSize());
	ctx.Unmap(dst.Get(), 0u);
}

void ContextBase::SetBufferData(BufferBase& dst, const IDynamicArray<byte>& src)
{
	if (dst.GetSize() > 0)
	{
		D3D_ASSERT_MSG(dst.GetUsage() != ResourceUsages::Immutable, "Cannot update Buffers without write access.");
		D3D_ASSERT_MSG(src.GetLength() == dst.GetSize(), "Buffer data source and destination size must be the same.");

		if (dst.GetUsage() == ResourceUsages::Dynamic)
			WriteBufferMapUnmap(*pContext.Get(), dst, src);
		else
			WriteBufferSubresource(*pContext.Get(), dst, src);
	}
}

static void WriteTexSubresource(ID3D11DeviceContext& ctx, ID3D11Resource& dst, const IDynamicArray<byte>& src, uint pixStride, uivec2 srcDim, uivec2 dstOffset)
{
	D3D11_BOX dstBox;
	dstBox.left = dstOffset.x;
	dstBox.right = dstOffset.x + srcDim.x;
	dstBox.top = dstOffset.y;
	dstBox.bottom = dstOffset.y + srcDim.y;
	dstBox.front = 0;
	dstBox.back = 1;

	ctx.UpdateSubresource(
		&dst,
		0,
		&dstBox,
		src.GetData(),
		(UINT)(pixStride * srcDim.x),
		(UINT)(pixStride * srcDim.x * srcDim.y)
	);
}

static void WriteTexMapUnmap(ID3D11DeviceContext& ctx, ID3D11Resource& dst, const IDynamicArray<byte>& src, uint pixStride, uivec2 srcDim, uivec2 dstOffset)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	D3D_CHECK_HR(ctx.Map(
		&dst,
		0u, // Subresource 0
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&msr
	));

	const size_t srcRowPitch = (size_t)srcDim.x * pixStride;
	const size_t dstRowPitch = msr.RowPitch;
	byte* pDst = static_cast<byte*>(msr.pData);
	const size_t dstRowOffset = dstOffset.x * pixStride;

	// Copy source image into texture row-by-row to account for variability in buffer alignment
	// and horizontal offset specified in dstOffset
	for (uint row = 0; row < srcDim.y; row++)
	{
		memcpy(
			pDst + ((row + dstOffset.y) * dstRowPitch) + dstRowOffset,
			&src[row * srcRowPitch],
			srcRowPitch
		);
	}

	ctx.Unmap(&dst, 0u);
}

void ContextBase::SetTextureData(ITexture2DBase& dst, const IDynamicArray<byte>& src, uint pixStride, uivec2 srcDim, uivec2 dstOffset)
{
	D3D_CHECK_MSG(dst.GetUsage() != ResourceUsages::Immutable, "Cannot update Textures without write access.");
	const uivec2 dstSize = dst.GetSize();
	const uivec2 dstMax = srcDim + dstOffset;
	D3D_ASSERT_MSG(dstMax.x <= dstSize.x && dstMax.y <= dstSize.y, "Texture write destination out of bounds");

	if (dst.GetUsage() == ResourceUsages::Dynamic)
		WriteTexMapUnmap(*pContext.Get(), *dst.GetResource(), src, pixStride, srcDim, dstOffset);
	else
		WriteTexSubresource(*pContext.Get(), *dst.GetResource(), src, pixStride, srcDim, dstOffset);
}

void ContextBase::ClearDepthStencil(IDepthStencil& ds, DSClearFlags clearFlags, float depthClear, UINT8 stencilClear)
{
	pContext->ClearDepthStencilView(ds.GetDSV(), (UINT)clearFlags, depthClear, stencilClear);
}

void ContextBase::ResetShaders() 
{
	for (int i = 0; i < g_ShadeStageCount; i++)
		UnbindStage((ShadeStages)i);
}

void ContextBase::Reset()
{
	ResetShaders();
	UnbindVertexBuffers();
	SetPrimitiveTopology(PrimTopology::UNDEFINED);
	UnbindRenderTargets();
	UnbindViewports();
}

const uint ContextBase::GetViewportCount() const { return (uint)pState->vpCount; }

const Viewport& ContextBase::GetViewport(uint index) const { return pState->viewports[index]; }

const Span<Viewport> ContextBase::GetViewports() const { return Span(pState->viewports.GetData(), pState->vpCount); }

void ContextBase::BindViewport(sint index, const vec2& size, const vec2& offset, const vec2& depth)
{
	Viewport vp = { offset, size, depth };
	BindViewports(Span(&vp), index);
}

void ContextBase::BindViewport(sint index, const Viewport& vp)
{
	BindViewports(Span((Viewport*)&vp), index);
}

void ContextBase::UnbindViewports(sint index, uint count)
{
	if (pState->vpCount == 0 || count == 0)
		return;

	if (count == -1)
	{
		index = 0;
		count = pState->vpCount;
	}

	D3D_ASSERT_MSG((index + count) <= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
		"Viewport range specified out of range.");

	pState->vpCount = count;
	pContext->RSSetViewports(pState->vpCount, (D3D11_VIEWPORT*)pState->viewports.GetData());
}

void ContextBase::BindViewports(const IDynamicArray<Viewport>& viewports, sint offset)
{
	D3D_ASSERT_MSG((offset + viewports.GetLength()) <= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
		"Number of viewports supplied exceeds limit.");

	const uint newCount = (uint)viewports.GetLength();
	Span dstSpan(&pState->viewports[offset], newCount);

	if (dstSpan != viewports)
	{
		memcpy(dstSpan.GetData(), viewports.GetData(), GetArrSize(viewports));
		pState->vpCount = std::max(pState->vpCount, (uint)(offset + newCount));
		pContext->RSSetViewports(pState->vpCount, (D3D11_VIEWPORT*)pState->viewports.GetData());
	}
}
