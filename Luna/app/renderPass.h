#pragma once

#include "lib/type.h"

namespace luna {
class Task;
class Renderer;
class RenderPassContext;
class RenderPass : public Object
{
	LUNA_DECLARE_ABSTRACT(RenderPass, Object);

	friend class luna::Renderer;
public:
	virtual void preSettings(RenderPassContext& rpc) {}
	virtual void render(RenderPassContext& rpc) const{}
	virtual void postSettings(RenderPassContext& rpc) {}

protected:
	RenderPass() {}
};

}
