#include "dx_descriptorheap.h"

#include "dx_utils.h"
#include "dx_app.h"

DescriptorHeap::DescriptorHeap(int descriptorsNum,
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorsType,
	D3D12_DESCRIPTOR_HEAP_FLAGS flags,
	ComPtr<ID3D12Device> dev) :
		alloc(descriptorsNum),
		TYPE(descriptorsType)
{
	device = dev;

	DESCRIPTOR_SIZE = Renderer::Inst().GetDescriptorSize(TYPE);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;

	heapDesc.NumDescriptors = descriptorsNum;
	heapDesc.Type = TYPE;
	heapDesc.Flags = flags;
	heapDesc.NodeMask = 0;

	ThrowIfFailed(device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(heap.GetAddressOf())));
}

int DescriptorHeap::Allocate(ComPtr<ID3D12Resource> resource)
{
	const int allocatedIndex = alloc.Allocate();

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandle(allocatedIndex);

	switch (TYPE)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		device->CreateRenderTargetView(resource.Get(), nullptr, handle);
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		device->CreateDepthStencilView(resource.Get(), nullptr, handle);
		break;
	default:
		assert(false && "Invalid descriptor heap type");
		break;
	}

	return allocatedIndex;
}

void DescriptorHeap::Delete(int index)
{
	alloc.Delete(index);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHandle(int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), index, DESCRIPTOR_SIZE);
}