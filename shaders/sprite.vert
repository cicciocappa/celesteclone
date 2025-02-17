#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

// Define the struct before using it in the buffer
struct SpriteData {
    vec2 position;
    vec2 size;
    float rotation;
    float padding[3];
    vec4 color;
};

layout (std140, binding = 0) buffer SpriteBuffer {
    SpriteData sprites[];
};

uniform mat4 projection;

out vec4 fragColor;

// Define the matrix transformation functions
mat4 translate(mat4 m, vec3 v) {
    mat4 result = m;
    result[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
    return result;
}

mat4 rotate(mat4 m, float angle, vec3 axis) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    
    axis = normalize(axis);
    
    mat4 rot = mat4(
        t * axis.x * axis.x + c,           t * axis.x * axis.y - s * axis.z,  t * axis.x * axis.z + s * axis.y,  0.0,
        t * axis.x * axis.y + s * axis.z,  t * axis.y * axis.y + c,           t * axis.y * axis.z - s * axis.x,  0.0,
        t * axis.x * axis.z - s * axis.y,  t * axis.y * axis.z + s * axis.x,  t * axis.z * axis.z + c,           0.0,
        0.0,                               0.0,                               0.0,                               1.0
    );
    
    return m * rot;
}

mat4 scale(mat4 m, vec3 v) {
    mat4 result = mat4(1.0);
    result[0][0] = v.x;
    result[1][1] = v.y;
    result[2][2] = v.z;
    return m * result;
}

void main() {
    // Calculate the model matrix
    mat4 model = mat4(1.0);

    // Translation
    model = translate(model, vec3(sprites[gl_InstanceID].position, 0.0));

    // Rotation (around the Z-axis)
    model = rotate(model, sprites[gl_InstanceID].rotation, vec3(0.0, 0.0, 1.0));

    // Scale
    model = scale(model, vec3(sprites[gl_InstanceID].size, 1.0));

    // Apply transformations
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    fragColor = sprites[gl_InstanceID].color;
}