#include "constant_buffer.h"

inline ConstantBuffer::ConstantBuffer(ID3D11Device* device, size_t size)
{
	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = size;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&desc, nullptr, &buf);

	assert(SUCCEEDED(hr));
}

void ConstantBuffer::BeginUpdate(ID3D11DeviceContext* deviceContext, void** data)
{
	D3D11_MAPPED_SUBRESOURCE s;
	deviceContext->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &s);
	*data = s.pData;
}

void ConstantBuffer::EndUpdate(ID3D11DeviceContext* deviceContext)
{
	deviceContext->Unmap(buf, 0);
}

void ConstantBuffer::Use(ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetConstantBuffers(0, 1, &buf);
}

ConstantBuffer::~ConstantBuffer()
{
	buf->Release();
}
