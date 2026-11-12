#version 450
// Example fragment shader. Edit it, rebuild the "shaders" target, then press R in the app.

layout(location = 0) in vec2 v_uv;         // 0..1 across the window, (0,0) = bottom-left
layout(location = 0) out vec4 out_color;

// SDL_GPU convention: fragment-stage uniform buffers live in set 3.
layout(set = 3, binding = 0) uniform Uniforms {
    vec2  resolution;   // window size in pixels
    float time;         // seconds since start
} u;

void main()
{
    vec2 p = (v_uv - 0.5) * vec2(u.resolution.x / u.resolution.y, 1.0);
    float t = u.time;

    float v = sin(p.x * 10.0 + t)
            + sin(p.y * 10.0 + t * 1.3)
            + sin(length(p) * 12.0 - t * 2.0);

    vec3 col = 0.5 + 0.5 * cos(v + vec3(0.0, 2.0, 4.0));
    out_color = vec4(col, 1.0);
}
