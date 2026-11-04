#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

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

layout(binding = 1) uniform ModelUBO {
    mat4 model;
    vec4 color;
    int isLightSource;
    float padding[3];
} object;

// Текстура
layout(binding = 2) uniform sampler2D albedoTexture;

void main() {
    // Если рисуем сам источник света - просто отдаем его цвет
    if (object.isLightSource == 1) {
        outColor = object.color;
        return;
    }

    vec3 normal = normalize(fragNormal);
    vec3 lightDirection = normalize(-global.lightDir);
    vec3 viewDir = normalize(global.cameraPos - fragWorldPos);

    // Сэмплируем текстуру и умножаем на цвет-оттенок из UI
    vec4 texColor = texture(albedoTexture, fragTexCoord);
    vec3 baseColor = texColor.rgb * object.color.rgb;

    vec3 ambient = global.ambientColor * baseColor;
    
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * global.lightColor * global.lightIntensity * baseColor;
    
    vec3 halfDir = normalize(lightDirection + viewDir);
    float specAngle = max(dot(normal, halfDir), 0.0);
    float specular = pow(specAngle, global.shininess);
    vec3 specularContrib = specular * global.specularColor * global.lightColor * global.lightIntensity;
    
    vec3 result = ambient + diffuse + specularContrib;
    outColor = vec4(result, 1.0);
}