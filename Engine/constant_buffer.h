#pragma once
#include "common.h"

class API ConstantBuffer
{
public:
	ConstantBuffer(ID3D11Device* device, size_t size);
	void BeginUpdate(ID3D11DeviceContext* deviceContext, void** data);
	void EndUpdate(ID3D11DeviceContext* deviceContext);
	void Use(ID3D11DeviceContext* deviceContext);
	~ConstantBuffer();

private:
	ID3D11Buffer* buf;
};
