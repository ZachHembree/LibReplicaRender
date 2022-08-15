#include "D3D11/dev/IndexBuffer.hpp"
#include "D3D11/dev/Formats.hpp"

using namespace Replica::D3D11;

IndexBuffer::IndexBuffer(const Device& device, 
	const IDynamicCollection<USHORT>& data, 
	BufferUsages usage, 
	BufferAccessFlags cpuAccess
) :
	count((UINT)data.GetLength()),
	BufferBase(BufferTypes::Index, usage, cpuAccess, device, data)
{ }

/// <summary>
/// Returns the number of elements in the buffer
/// </summary>
UINT IndexBuffer::GetLength() const { return count; }

/// <summary>
/// Binds an index buffer to the input assembler. Used with DrawIndexed().
/// </summary>
void IndexBuffer::Bind(UINT offset)
{
	pDev->GetContext()->IASetIndexBuffer(pBuf.Get(), (DXGI_FORMAT)Formats::R16_UINT, offset);
}
