#ifndef RENDERPIPELINE_INTERFACE_H
#define RENDERPIPELINE_INTERFACE_H

typedef struct {
    GLuint FramebufferTex;
    GLuint FramebufferRB;
    GLuint Framebuffer;
} framebuffer;

typedef struct {
    framebuffer OldFramebuffer;
    framebuffer NewFramebuffer;
    GLuint QuadVAO;
} pipeline_state;


typedef void (*tick_stage_func)(
    SDL_Window* Window,
    pipeline_state* PipelineState);
typedef void (*cleanup_stage_func)(void);

typedef struct {
    library*           Library;
    tick_stage_func    TickStage;
    cleanup_stage_func CleanupStage;
} render_stage;

#endif // RENDERPIPELINE_INTERFACE_H
