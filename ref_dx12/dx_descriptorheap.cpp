#include "dx_descriptorheap.h"

#include "dx_utils.h"
#include "dx_app.h"
#include "dx_infrastructure.h"

DescriptorHeap::DescriptorHeap(int descriptorsNum,
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorsType,
	D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
		alloc(descriptorsNum),
		TYPE(descriptorsType)
{
	DESCRIPTOR_SIZE = Renderer::Inst().GetDescriptorSize(TYPE);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;

	heapDesc.NumDescriptors = descriptorsNum;
	heapDesc.Type = TYPE;
	heapDesc.Flags = flags;
	heapDesc.NodeMask = 0;

	ThrowIfFailed(Infr::Inst().GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(heap.GetAddressOf())));
}

int DescriptorHeap::Allocate(ComPtr<ID3D12Resource> resource, DescriptorHeap::Desc_t* desc)
{
	const int allocatedIndex = alloc.Allocate();

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleCPU(allocatedIndex);

	switch (TYPE)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
	{
		D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc = (desc == nullptr) ? 
			nullptr : &std::get<D3D12_RENDER_TARGET_VIEW_DESC>(*desc);

		Infr::Inst().GetDevice()->CreateRenderTargetView(resource.Get(), rtvDesc, handle);
		break;
	}
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc = (desc == nullptr) ?
			nullptr : &std::get<D3D12_DEPTH_STENCIL_VIEW_DESC>(*desc);

		Infr::Inst().GetDevice()->CreateDepthStencilView(resource.Get(), dsvDesc, handle);
		break;
	}
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC* cbvSrvDesc = (desc == nullptr) ?
			nullptr : &std::get<D3D12_SHADER_RESOURCE_VIEW_DESC>(*desc);

		Infr::Inst().GetDevice()->CreateShaderResourceView(resource.Get(), cbvSrvDesc, handle);
		break;
	}
	default:
	{
		assert(false && "Invalid descriptor heap type");
		break;
	}
	}

	return allocatedIndex;
}

void DescriptorHeap::Delete(int index)
{
	alloc.Delete(index);
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeapResource()
{
	return heap.Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHandleCPU(int index) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), index, DESCRIPTOR_SIZE);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHandleGPU(int index) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), index, DESCRIPTOR_SIZE);
}
