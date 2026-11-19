#version 330 

layout(location = 0) in vec3 a_vertex; 
layout(location = 1) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec2 u_textureGrid;  // Number of rows and columns in the sprite sheet
uniform int u_currentFrame;  // Current frame index for animation
uniform bool u_isAnimated;   // Is the texture animated (true for fire, false for others)

out vec2 v_uv;

void main() {
    vec2 frameUV = vec2(0.0, 0.0);
    if (u_isAnimated) {
        // Calculate UV offset based on the current frame
        int row = u_currentFrame / int(u_textureGrid.y);
        int col = u_currentFrame % int(u_textureGrid.y);
        frameUV = vec2(col, row) / u_textureGrid;

        // Adjust UV coordinates for the frame
        v_uv = a_uv / u_textureGrid + frameUV;
    } else {
        // Use standard UVs for non-animated textures
        v_uv = a_uv;
    }

    gl_Position = u_projection * u_view * u_model * vec4(a_vertex, 1.0);
}
