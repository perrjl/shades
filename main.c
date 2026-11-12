#include <SDL3/SDL.h>

typedef struct {
    float resolution[2];
    float time;
    float pad;
} FragUniforms;

// loads shaders (SPIRV only)
SDL_GPUShader* load_shader(SDL_GPUDevice* device, const char* path, SDL_GPUShaderStage stage, Uint32 num_uniform_buffers) {
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format;
    const char *ext, *entrypoint;

    if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    } else {
        SDL_Log("No supported shader format for this GPU driver");
        return NULL;
    }

    size_t size;
    void *code = SDL_LoadFile(path, &size);
    if (code == NULL) {
        SDL_Log("Couldn't load %s: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code_size = size,
        .code = code,
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_uniform_buffers = num_uniform_buffers,
    };
    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &info);
    if (!shader) {
        SDL_Log("Couldn't create shader from %s: %s", path, SDL_GetError());
    }
    SDL_free(code);
    return shader;
}

SDL_GPUGraphicsPipeline* build_pipeline(SDL_GPUDevice *device, SDL_Window *window) {
    SDL_GPUShader *vs = load_shader(device, "vertex.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0);
    SDL_GPUShader *fs = load_shader(device, "fragment.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1);
    SDL_GPUGraphicsPipeline *pipeline = NULL;

    SDL_GPUColorTargetDescription target = {
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
    };
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .vertex_shader = vs,
        .fragment_shader = fs,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = { .color_target_descriptions = &target, .num_color_targets = 1 },
    };
    pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline) {
        SDL_Log("Couldn't create pipeline: %s", SDL_GetError());
    }

    // shader has been stored in the pipeline, delete local copies that are now unused
    SDL_ReleaseGPUShader(device, vs);
    SDL_ReleaseGPUShader(device, fs);
    return pipeline;
}

int main(int argc, const char* argv[]){
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL3 GPU shader", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return -1;
    }

    SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL,
                                                true /* debug mode */, NULL);
    if (!device || !SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("GPU setup failed: %s", SDL_GetError());
        return -1;
    }
    SDL_Log("GPU driver: %s", SDL_GetGPUDeviceDriver(device));

    SDL_GPUGraphicsPipeline *pipeline = build_pipeline(device, window);
    if (!pipeline) {
        return -1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.key == SDLK_R) {
                    /* Hot reload: keep the old pipeline if the new shader fails. */
                    SDL_GPUGraphicsPipeline* new_pipeline = build_pipeline(device, window);
                    if (new_pipeline) {
                        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
                        pipeline = new_pipeline;
                        SDL_Log("Reloaded shader");
                    }
                }
            }
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) {
            SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            break;
        }

        SDL_GPUTexture* texture;
        uint32_t width, height;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &texture, &width, &height)) {
            SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
            SDL_CancelGPUCommandBuffer(cmd);
            break;
        }

        if (texture) {
            FragUniforms uniforms = {
                .resolution = { (float)width, (float)height },
                .time = (float)SDL_GetTicks() / 1000.0f,
            };
            SDL_PushGPUFragmentUniformData(cmd, 0, &uniforms, sizeof(uniforms));

            SDL_GPUColorTargetInfo target = {
                .texture = texture,
                .clear_color = { 0.0f, 0.0f, 0.0f, 1.0f },
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };
            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &target, 1, NULL);
            SDL_BindGPUGraphicsPipeline(pass, pipeline);
            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0); /* 3 vertices, no vertex buffer needed */
            SDL_EndGPURenderPass(pass);
        }

        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
