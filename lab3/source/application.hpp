#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include "graphics_internal.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace application {

bool initialize();
void shutdown();
void update(double time);
void render(const graphics::internal::FrameData& fd);

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct alignas(16) GlobalUniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;
    float lightIntensity;
    glm::vec3 lightColor;
    float padding1;
    glm::vec3 ambientColor;
    float shininess;
    glm::vec3 specularColor;
    float padding2;
    glm::vec3 cameraPos;
    float padding3;
};

struct alignas(16) ModelUniformBufferObject {
    glm::mat4 model;
    glm::vec4 color;
    int isLightSource;
    float padding[3];
};

extern VkPipeline pipeline;
extern VkPipelineLayout pipelineLayout;
extern VkDescriptorSetLayout descriptorSetLayout;
extern VkDescriptorPool descriptorPool;

extern VkBuffer vertexBuffer;
extern VmaAllocation vertexBufferAlloc;
extern VkBuffer indexBuffer;
extern VmaAllocation indexBufferAlloc;
extern VkBuffer modelUniformBuffer;
extern VmaAllocation modelUniformBufferAlloc;
extern VkBuffer globalUniformBuffer;
extern VmaAllocation globalUniformBufferAlloc;

extern VkDescriptorSet descriptorSet;
extern VkImage textureImage;
extern VmaAllocation textureImageAlloc;
extern VkImageView textureImageView;
extern VkSampler textureSampler;

extern std::vector<Vertex> vertices;
extern std::vector<uint32_t> indices;

// Камера
extern float cameraDistance;
extern float cameraYaw;
extern float cameraPitch;
extern float fov;
extern bool useOrtho;
extern float orthoSize;

// Вращение
extern float cubeRotX;
extern float cubeRotY;
extern float cubeRotZ;

// Свет
extern glm::vec3 lightDir;
extern float lightIntensity;
extern glm::vec3 lightColor;
extern bool showLightSource;

// Материал
extern glm::vec3 ambientColor;
extern float shininess;
extern glm::vec3 specularColor;
extern glm::vec4 objectAlbedo;

// Мышь
extern bool isMouseDragging;
extern glm::vec2 lastMousePos;
extern float mouseSensitivity;

} // namespace application