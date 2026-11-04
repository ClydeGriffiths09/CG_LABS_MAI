#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord; // UV координаты

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragWorldPos;

// Глобальный UBO (Камера, Свет)
layout(binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    vec3 lightDir;
    float lightIntensity;
    vec3 lightColor;
    float padding1;
    vec3 ambientColor;
    float shininess;
    vec3 specularColor;
    float padding2;
    vec3 cameraPos;
    float padding3;
} global;

// Динамический UBO (Модель)
layout(binding = 1) uniform ModelUBO {
    mat4 model;
    vec4 color;
    int isLightSource;
    float padding[3];
} object;

void main() {
    vec4 worldPos = object.model * vec4(inPosition, 1.0);
    fragNormal = mat3(transpose(inverse(object.model))) * inNormal;
    fragTexCoord = inTexCoord;
    fragWorldPos = worldPos.xyz;
    gl_Position = global.proj * global.view * worldPos;
}