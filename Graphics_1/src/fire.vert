#version 330

in vec4 position;      // Per vertex: vertex position of the billboard quad
in vec4 bdpos;         // Per instance: billboard position in world space
in vec4 rgb;           // Per instance: particle color
in float scale;        // Per instance: scale of the particle
in float angle;        // Per instance: rotation angle of the particle

out vec3 fragColor;    // To fragment shader: color

uniform mat4 modelcamviewprojmat; // Model-View-Projection matrix for rendering
uniform mat4 camtrf;              // Camera transformation matrix

void main() {
    vec4 scaledPos = position * scale; // Scale the quad size
    vec3 z = normalize(bdpos.xyz - camtrf[3].xyz); // Billboard orientation based on camera position
    vec3 camY = camtrf[1].xyz;
    vec3 xAxis = cross(camY, z);
    vec3 yAxis = cross(z, xAxis);

    // Apply rotation around the Z-axis of the billboard
    vec3 xRot = xAxis * cos(angle) + yAxis * sin(angle);
    vec3 yRot = xAxis * -sin(angle) + yAxis * cos(angle);

    mat4 trf = mat4(1.0);
    trf[0].xyz = xRot;
    trf[1].xyz = yRot;
    trf[2].xyz = z;
    trf[3] = bdpos;

    vec4 transformedPos = trf * scaledPos;
    fragColor = rgb.rgb; // Pass the particle color to the fragment shader
    gl_Position = modelcamviewprojmat * transformedPos;
}
