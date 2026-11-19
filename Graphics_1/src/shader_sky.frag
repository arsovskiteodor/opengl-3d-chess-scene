#version 330 core

in vec2 v_uv;

uniform sampler2D u_texture;
uniform bool u_isAnimated;   // Is this rendering fire (true) or rings (false)

out vec4 FragColor;

void main() {
    vec4 color = texture(u_texture, v_uv);

    if (u_isAnimated) {
        // Fire effect: Apply alpha blending
        if (color.a < 0.1) {
            discard; // Discard transparent fragments for fire
        }
    }

    FragColor = color;
}
