#version 450
// Generates one oversized triangle that covers the whole screen.
// No vertex buffer: positions come from gl_VertexIndex (0, 1, 2).

layout(location = 0) out vec2 v_uv;

void main()
{
    vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); // (0,0) (2,0) (0,2)
    v_uv = pos;                                                      // 0..1 on screen, (0,0) = bottom-left
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
