#pragma once

#include <functional>
#include <vector>
#include <variant>

#include "dx_buffer.h"
#include "dx_drawcalls.h"
#include "dx_common.h"
#include "dx_passparameters.h"
#include "dx_threadingutils.h"

//#TODO
// 1) Implement proper tex samplers handling. When I need more samplers
// 2) Implement root constants
// 3) Implement const buffers in descriptor table 
// 4) Implement custom render targets and resource creation
// 5) Implement include
// 6) Do proper logging for parsing and execution
// 7) Proper name generation for D3D objects

class Pass_UI
{
public:
	struct PassObj
	{
		std::vector<RootArg::Arg_t> rootArgs;
		const DrawCall_UI_t* originalDrawCall = nullptr;
	};

public:

	Pass_UI() = default;

	Pass_UI(const Pass_UI&) = default;
	Pass_UI& operator=(const Pass_UI&) = default;

	Pass_UI(Pass_UI&&) = default;
	Pass_UI& operator=(Pass_UI&&) = default;

	~Pass_UI();

	void Execute(GPUJobContext& context);
	void Init(PassParameters&& parameters);

	void RegisterPassResources(GPUJobContext& jobCtx);
	void UpdatePassResources(GPUJobContext& jobCtx);
	void ReleasePerFrameResources();

private:

	void Start(GPUJobContext& jobCtx);
	void UpdateDrawObjects(GPUJobContext& jobCtx);

	void SetUpRenderState(GPUJobContext& jobCtx);
	void Draw(GPUJobContext& jobCtx);

private:

	PassParameters passParameters;

	unsigned int perObjectConstBuffMemorySize = 0;
	// One object vertex memory size
	unsigned int perObjectVertexMemorySize = 0;
	// One vertex size
	unsigned int perVertexMemorySize = 0;

	// Pass local args memory size
	unsigned int passMemorySize = 0;

	BufferHandler passConstBuffMemory = Const::INVALID_BUFFER_HANDLER;
	BufferHandler objectConstBuffMemory = Const::INVALID_BUFFER_HANDLER;
	BufferHandler vertexMemory = Const::INVALID_BUFFER_HANDLER;

	std::vector<PassObj> drawObjects;

};

using Pass_t = std::variant<Pass_UI>;
