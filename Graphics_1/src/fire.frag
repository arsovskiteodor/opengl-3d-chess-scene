#version 330 core
in vec2 fragTexCoord;
out vec4 FragColor;

uniform sampler2D fireTexture; // Fire texture
uniform float time; // Time for animation

void main() {
    // Sample fire intensity from texture
    float intensity = texture(fireTexture, fragTexCoord).r;

    // Map intensity to fire colors
    vec3 fireColor = vec3(0.0);
    if (intensity > 0.7) {
        fireColor = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.0), (intensity - 0.7) / 0.3);
    } else if (intensity > 0.3) {
        fireColor = mix(vec3(0.5, 0.0, 0.0), vec3(1.0, 0.5, 0.0), (intensity - 0.3) / 0.4);
    } else {
        fireColor = vec3(0.0);
    }

    // Add flicker effect
    float flicker = 0.8 + 0.2 * sin(time * 5.0 + fragTexCoord.y * 10.0);
    fireColor *= flicker;

    FragColor = vec4(fireColor, intensity);
}
