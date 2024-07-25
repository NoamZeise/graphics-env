#include <graphics/manager.h>
#include <graphics/glm_helper.h>
#include <graphics/pipeline.h>

int main(int argc, char** argv) {
    // setup renderer with desired state
    ManagerState state;
    state.windowTitle = "minimum example";
    state.defaultRenderer = RenderFramework::Vulkan;
    Manager manager(state);

    // example pipeline api

    ShaderPool* shaderPool = manager.render->CreateShaderPool();

    struct ShaderTransforms {
	glm::mat4 view;
	glm::mat4 projection;	
    };
    ShaderSet* transforms = shaderPool->CreateSet(shader::vert);    
    transforms->addUniformBuffer(0, sizeof(ShaderTransforms));
    
    struct ShaderColours {
	glm::vec4 baseColour;
	float shininess;
    };
    ShaderSet* colours = shaderPool->CreateSet(shader::frag);
    colours->addUniformBuffer(0, sizeof(ShaderColours));

    // must be done before passing to pipeline
    shaderPool->CreateGpuResources();

    Pipeline::Config pipelineConfig;
    pipelineConfig.depthTest = false;
    Pipeline* pipeline2d = manager.render->NewPipeline(
	    pipelineConfig,
	    vertex::v2D.input,
	    Pipeline::ReadShaderCode("shaders/vulkan/custom_pipeline.vert"),
	    Pipeline::ReadShaderCode("shaders/vulkan/custom_pipeline.frag"));

    pipeline2d->addShaderLayout(0, transforms);
    pipeline2d->addShaderLayout(1, colours);

    RenderPass* renderpass = manager.render->CreateRenderPass(
	    { AttachmentDesc(
			0, AttachmentType::Colour, AttachmentUse::Screen, MsaaSample::Count1),
	    },
	    RenderPass::Config{});

    // create using screen extent - automatically recreates on
    // screen extent change
    renderpass->Create();

    // pipeline is recreated when renderpass is recreated
    pipeline2d->CreatePipeline(renderpass);
       
    ResourcePool* pool = manager.render->pool();
    // load resources to cpu
    Resource::Texture tex = pool->tex()->load("textures/tile.png");
    // load resources to gpu
    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    while(!glfwWindowShouldClose(manager.window)) {
	// update timers, input, etc
	manager.update();

	// Press Esc to exit
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	
	if(manager.winActive()) {
	    // draw texture where mouse is
            manager.render->DrawQuad(
		    tex,
		    glmhelper::calcMatFromRect(
			    glm::vec4(manager.mousePos().x, manager.mousePos().y, 200.0f, 200.0f)
			    , 0.0f, 0.5f),
		    glm::vec4(1.0f),
		    glmhelper::getTextureOffset(
			    tex.dim, glm::vec4(0.0f, 0.0f, tex.dim.x * 5, tex.dim.y * 5)));
	    manager.render->EndDraw();
	}
    }
}
