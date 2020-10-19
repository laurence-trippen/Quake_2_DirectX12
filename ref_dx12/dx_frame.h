#pragma once

#include <d3d12.h>
#include <vector>
#include <string>
#include <atomic>
#include <memory>

#include "dx_common.h"
#include "dx_objects.h"
#include "dx_buffer.h"
#include "dx_drawcalls.h"
#include "dx_camera.h"
#include "dx_threadingutils.h"
#include "dx_texture.h"

class Semaphore;

class Frame
{
public:

	Frame() = default;

	Frame(const Frame&) = delete;
	Frame& operator=(const Frame&) = delete;

	Frame(Frame&& other) = delete;
	Frame& operator=(Frame&& other) = delete;

	~Frame();

	void Init();
	void ResetSyncData();
	
	std::shared_ptr<Semaphore> GetFinishSemaphore() const;
	
	// Used for rendering. Receive on frame beginning
	// Released on the frame end
	AssertBufferAndView* colorBufferAndView = nullptr;

	// Owned by frame
	ComPtr<ID3D12Resource> depthStencilBuffer;
	int depthBufferViewIndex = Const::INVALID_INDEX;

	// Not owned by frame, but rather receive on frame beginning
	// Released on the frame end
	std::vector<int> acquiredCommandListsIndices;
	
	// Utils
	std::vector<entity_t> entitiesToDraw;
	std::vector<particle_t> particlesToDraw;

	std::atomic<bool> isInUse = false;
	std::vector<DynamicObject> dynamicObjects;
	LockVector_t<ComPtr<ID3D12Resource>> uploadResources;
	LockVector_t<BufferHandler> streamingObjectsHandlers;

	std::vector<DrawCall_UI_t> uiDrawCalls;
	
	std::vector<Texture*> deferredTextures;

	int frameNumber = Const::INVALID_INDEX;

	tagRECT scissorRect;
	Camera camera;
	XMFLOAT4X4 uiProjectionMat;
	XMFLOAT4X4 uiViewMat;

	// Synchronization 
	
	// These two values are used in the very end when we call ExecuteCommandList
	int executeCommandListFenceValue = -1;
	HANDLE executeCommandListEvenHandle = INVALID_HANDLE_VALUE;

	std::shared_ptr<Semaphore> frameFinishedSemaphore;

};