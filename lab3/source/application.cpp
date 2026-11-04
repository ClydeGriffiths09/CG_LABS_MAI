#include "application.hpp"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>

// Подключаем stb_image для загрузки картинок
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace application {

VkPipeline pipeline = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

VkBuffer vertexBuffer = VK_NULL_HANDLE;
VmaAllocation vertexBufferAlloc = VK_NULL_HANDLE;
VkBuffer indexBuffer = VK_NULL_HANDLE;
VmaAllocation indexBufferAlloc = VK_NULL_HANDLE;
VkBuffer modelUniformBuffer = VK_NULL_HANDLE;
VmaAllocation modelUniformBufferAlloc = VK_NULL_HANDLE;
VkBuffer globalUniformBuffer = VK_NULL_HANDLE;
VmaAllocation globalUniformBufferAlloc = VK_NULL_HANDLE;

VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
VkImage textureImage = VK_NULL_HANDLE;
VmaAllocation textureImageAlloc = VK_NULL_HANDLE;
VkImageView textureImageView = VK_NULL_HANDLE;
VkSampler textureSampler = VK_NULL_HANDLE;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

// Camera
float cameraDistance = 8.0f;
float cameraYaw = 0.7f;
float cameraPitch = 0.4f;
float fov = 45.0f;
bool useOrtho = false;
float orthoSize = 4.0f;

// Object Rotation
float cubeRotX = 0.0f;
float cubeRotY = 0.0f;
float cubeRotZ = 0.0f;

// Light
glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, -1.0f, 0.3f));
float lightIntensity = 1.5f; 
glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);
bool showLightSource = true;

// Material
glm::vec3 ambientColor = glm::vec3(0.3f, 0.3f, 0.35f); 
float shininess = 64.0f; 
glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec4 objectAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

// Mouse
bool isMouseDragging = false;
glm::vec2 lastMousePos = glm::vec2(0.0f, 0.0f);
float mouseSensitivity = 0.005f;

size_t alignedModelSize;

void generateParallelepipedWithNormals() {
    glm::vec2 uv00(0.0f, 0.0f);
    glm::vec2 uv10(1.0f, 0.0f);
    glm::vec2 uv11(1.0f, 1.0f);
    glm::vec2 uv01(0.0f, 1.0f);

    vertices = {
        // Front face (z = 1)
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, uv00},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, uv10},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, uv11},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, uv01},
        
        // Back face (z = -1)
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, uv10},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, uv00},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, uv01},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, uv11},
        
        // Top face (y = 1)
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, uv01},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, uv11},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, uv10},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, uv00},
        
        // Bottom face (y = -1)
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, uv01},
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, uv11},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, uv10},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, uv00},
        
        // Right face (x = 1)
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, uv10},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, uv11},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, uv01},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, uv00},
        
        // Left face (x = -1)
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, uv00},
        {{-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, uv01},
        {{-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, uv11},
        {{-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, uv10}
    };

    indices = {
        0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
        8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20
    };
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VmaMemoryUsage memoryUsage, VkBuffer& buffer,
                  VmaAllocation& allocation, bool hostVisible = false) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    if (hostVisible) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }

    vmaCreateBuffer(graphics::internal::context.allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    auto& ctx = graphics::internal::context;
    
    // Создаем временный пул команд
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = ctx.graphics_queue_index;

    VkCommandPool tempPool;
    vkCreateCommandPool(ctx.device, &poolInfo, nullptr, &tempPool);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = tempPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(ctx.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(ctx.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx.graphics_queue);

    vkFreeCommandBuffers(ctx.device, tempPool, 1, &commandBuffer);
    vkDestroyCommandPool(ctx.device, tempPool, nullptr);
}

void createTexture() {
    auto& ctx = graphics::internal::context;
    int texWidth, texHeight, texChannels;
    
    // ИСПРАВЛЕНО: stbi_uc вместо stbi_uchar
    stbi_uc* pixels = stbi_load("shaders/TeresaLisbon.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image! (Place TeresaLisbon.png in shaders/)");
    }
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAlloc;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingBuffer, stagingBufferAlloc, true);

    void* data;
    vmaMapMemory(ctx.allocator, stagingBufferAlloc, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(ctx.allocator, stagingBufferAlloc);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateImage(ctx.allocator, &imageInfo, &allocInfo, &textureImage, &textureImageAlloc, nullptr);

    // Создаем временный пул команд для копирования текстуры
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = ctx.graphics_queue_index;

    VkCommandPool tempPool;
    vkCreateCommandPool(ctx.device, &poolInfo, nullptr, &tempPool);

    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandPool = tempPool;
    cmdAllocInfo.commandBufferCount = 1;
    
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(ctx.device, &cmdAllocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {(uint32_t)texWidth, (uint32_t)texHeight, 1};
    vkCmdCopyBufferToImage(cmd, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmd);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(ctx.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx.graphics_queue);
    
    vkFreeCommandBuffers(ctx.device, tempPool, 1, &cmd);
    vkDestroyCommandPool(ctx.device, tempPool, nullptr);

    vmaDestroyBuffer(ctx.allocator, stagingBuffer, stagingBufferAlloc);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCreateImageView(ctx.device, &viewInfo, nullptr, &textureImageView);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkCreateSampler(ctx.device, &samplerInfo, nullptr, &textureSampler);
}

std::vector<char> loadShader(const char* filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error(std::string("Failed to open shader: ") + filename);
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> code(fileSize);
    file.seekg(0);
    file.read(code.data(), fileSize);
    return code;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule module;
    vkCreateShaderModule(device, &createInfo, nullptr, &module);
    return module;
}

glm::vec3 getCameraPosition() {
    float x = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    float y = cameraDistance * sin(cameraPitch);
    float z = cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    return glm::vec3(x, y, z);
}

glm::mat4 getViewMatrix() {
    return glm::lookAt(getCameraPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

bool initialize() {
    generateParallelepipedWithNormals();
    auto& ctx = graphics::internal::context;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(ctx.physical_device, &properties);
    size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
    alignedModelSize = sizeof(ModelUniformBufferObject);
    if (minUboAlignment > 0) {
        alignedModelSize = (alignedModelSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    VkDescriptorSetLayoutBinding bindings[3] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;
    vkCreateDescriptorSetLayout(ctx.device, &layoutInfo, nullptr, &descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

    auto vertCode = loadShader("shaders/lighting.vert.spv");
    auto fragCode = loadShader("shaders/lighting.frag.spv");
    VkShaderModule vertModule = createShaderModule(ctx.device, vertCode);
    VkShaderModule fragModule = createShaderModule(ctx.device, fragCode);

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main"};
    shaderStages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main"};

    VkVertexInputBindingDescription bindingDesc = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributeDescs[3] = {};
    attributeDescs[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)};
    attributeDescs[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)};
    attributeDescs[2] = {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {0.0f, 0.0f, (float)ctx.swapchain_extent.width, (float)ctx.swapchain_extent.height, 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, ctx.swapchain_extent};
    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = 0xf;
    VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = ctx.render_pass;

    vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    vkDestroyShaderModule(ctx.device, fragModule, nullptr);
    vkDestroyShaderModule(ctx.device, vertModule, nullptr);

    VkDeviceSize vertexSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingVertexBuffer;
    VmaAllocation stagingVertexAlloc;
    createBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingVertexBuffer, stagingVertexAlloc, true);
    void* data;
    vmaMapMemory(ctx.allocator, stagingVertexAlloc, &data);
    memcpy(data, vertices.data(), (size_t)vertexSize);
    vmaUnmapMemory(ctx.allocator, stagingVertexAlloc);

    createBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, vertexBuffer, vertexBufferAlloc);
    copyBuffer(stagingVertexBuffer, vertexBuffer, vertexSize);
    vmaDestroyBuffer(ctx.allocator, stagingVertexBuffer, stagingVertexAlloc);

    VkDeviceSize indexSize = sizeof(indices[0]) * indices.size();
    VkBuffer stagingIndexBuffer;
    VmaAllocation stagingIndexAlloc;
    createBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingIndexBuffer, stagingIndexAlloc, true);
    vmaMapMemory(ctx.allocator, stagingIndexAlloc, &data);
    memcpy(data, indices.data(), (size_t)indexSize);
    vmaUnmapMemory(ctx.allocator, stagingIndexAlloc);

    createBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, indexBuffer, indexBufferAlloc);
    copyBuffer(stagingIndexBuffer, indexBuffer, indexSize);
    vmaDestroyBuffer(ctx.allocator, stagingIndexBuffer, stagingIndexAlloc);

    createBuffer(sizeof(GlobalUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, globalUniformBuffer, globalUniformBufferAlloc, true);
    createBuffer(alignedModelSize * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, modelUniformBuffer, modelUniformBufferAlloc, true);

    createTexture();

    VkDescriptorPoolSize poolSizes[3] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = 1;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSizes;
    vkCreateDescriptorPool(ctx.device, &poolInfo, nullptr, &descriptorPool);

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    vkAllocateDescriptorSets(ctx.device, &allocInfo, &descriptorSet);

    VkDescriptorBufferInfo globalBufferInfo = {globalUniformBuffer, 0, sizeof(GlobalUniformBufferObject)};
    VkDescriptorBufferInfo modelBufferInfo = {modelUniformBuffer, 0, sizeof(ModelUniformBufferObject)};
    VkDescriptorImageInfo imageInfo = {textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet descriptorWrites[3] = {};
    descriptorWrites[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &globalBufferInfo, nullptr};
    descriptorWrites[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &modelBufferInfo, nullptr};
    descriptorWrites[2] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr};

    vkUpdateDescriptorSets(ctx.device, 3, descriptorWrites, 0, nullptr);

    return true;
}

void shutdown() {
    auto& ctx = graphics::internal::context;
    vkQueueWaitIdle(ctx.graphics_queue);

    vkDestroyPipeline(ctx.device, pipeline, nullptr);
    vkDestroyPipelineLayout(ctx.device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(ctx.device, descriptorSetLayout, nullptr);

    vmaDestroyBuffer(ctx.allocator, vertexBuffer, vertexBufferAlloc);
    vmaDestroyBuffer(ctx.allocator, indexBuffer, indexBufferAlloc);
    vmaDestroyBuffer(ctx.allocator, modelUniformBuffer, modelUniformBufferAlloc);
    vmaDestroyBuffer(ctx.allocator, globalUniformBuffer, globalUniformBufferAlloc);

    vkDestroySampler(ctx.device, textureSampler, nullptr);
    vkDestroyImageView(ctx.device, textureImageView, nullptr);
    vmaDestroyImage(ctx.allocator, textureImage, textureImageAlloc);

    if (descriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(ctx.device, descriptorPool, nullptr);
}

void update(double time) {
    auto& ctx = graphics::internal::context;

    ImGui::Begin("Lab 3 - Texturing");
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Camera");
    ImGui::SliderFloat("Distance", &cameraDistance, 2.0f, 20.0f);
    ImGui::SliderFloat("Yaw", &cameraYaw, -3.14159f, 3.14159f);
    ImGui::SliderFloat("Pitch", &cameraPitch, -1.5f, 1.5f);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Object Rotation");
    ImGui::SliderFloat("Rot X", &cubeRotX, 0.0f, 360.0f);
    ImGui::SliderFloat("Rot Y", &cubeRotY, 0.0f, 360.0f);
    ImGui::SliderFloat("Rot Z", &cubeRotZ, 0.0f, 360.0f);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Light & Material");
    ImGui::SliderFloat3("Light Dir", &lightDir.x, -1.0f, 1.0f);
    ImGui::Checkbox("Show Light Source", &showLightSource);
    ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f);
    ImGui::ColorEdit4("Tint Color", &objectAlbedo.x);
    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsMouseDown(0) && !io.WantCaptureMouse) {
        if (!isMouseDragging) { lastMousePos = glm::vec2(io.MousePos.x, io.MousePos.y); isMouseDragging = true; }
        else {
            glm::vec2 currentMousePos(io.MousePos.x, io.MousePos.y);
            glm::vec2 delta = currentMousePos - lastMousePos;
            cameraYaw += delta.x * mouseSensitivity;
            cameraPitch += delta.y * mouseSensitivity;
            cameraPitch = glm::clamp(cameraPitch, -1.5f, 1.5f);
            lastMousePos = currentMousePos;
        }
    } else isMouseDragging = false;
    if (io.MouseWheel != 0.0f && !io.WantCaptureMouse) {
        cameraDistance -= io.MouseWheel * 0.5f;
        cameraDistance = glm::clamp(cameraDistance, 2.0f, 20.0f);
    }

    lightDir = glm::normalize(lightDir);

    GlobalUniformBufferObject globalUbo = {};
    globalUbo.view = getViewMatrix();
    float aspect = (float)ctx.swapchain_extent.width / (float)ctx.swapchain_extent.height;
    if (useOrtho) globalUbo.proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
    else globalUbo.proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    globalUbo.proj[1][1] *= -1.0f;
    
    globalUbo.lightDir = lightDir;
    globalUbo.lightIntensity = lightIntensity;
    globalUbo.lightColor = lightColor;
    globalUbo.ambientColor = ambientColor;
    globalUbo.shininess = shininess;
    globalUbo.specularColor = specularColor;
    globalUbo.cameraPos = getCameraPosition();

    void* data;
    vmaMapMemory(ctx.allocator, globalUniformBufferAlloc, &data);
    memcpy(data, &globalUbo, sizeof(globalUbo));
    vmaUnmapMemory(ctx.allocator, globalUniformBufferAlloc);

    vmaMapMemory(ctx.allocator, modelUniformBufferAlloc, &data);
    
    ModelUniformBufferObject cubeUbo = {};
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(cubeRotX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(cubeRotY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(cubeRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    cubeUbo.model = model;
    cubeUbo.color = objectAlbedo;
    cubeUbo.isLightSource = 0;
    memcpy((char*)data, &cubeUbo, sizeof(cubeUbo));

    ModelUniformBufferObject lightUbo = {};
    glm::vec3 lightGizmoPos = -lightDir * 8.0f;
    lightUbo.model = glm::translate(glm::mat4(1.0f), lightGizmoPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
    lightUbo.color = glm::vec4(lightColor * lightIntensity, 1.0f);
    lightUbo.isLightSource = 1;
    memcpy((char*)data + alignedModelSize, &lightUbo, sizeof(lightUbo));

    vmaUnmapMemory(ctx.allocator, modelUniformBufferAlloc);
}

void render(const graphics::internal::FrameData& fd) {
    vkResetCommandBuffer(fd.command_buffer, 0);
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(fd.command_buffer, &beginInfo);

    VkClearValue clearValues[2] = {};
    clearValues[0].color = {{0.1f, 0.1f, 0.15f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = graphics::internal::context.render_pass;
    renderPassInfo.framebuffer = fd.framebuffer;
    renderPassInfo.renderArea = {{0, 0}, graphics::internal::context.swapchain_extent};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(fd.command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(fd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(fd.command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(fd.command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    uint32_t dynamicOffset = 0;
    vkCmdBindDescriptorSets(fd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
    vkCmdDrawIndexed(fd.command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    if (showLightSource) {
        dynamicOffset = static_cast<uint32_t>(alignedModelSize);
        vkCmdBindDescriptorSets(fd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
        vkCmdDrawIndexed(fd.command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(fd.command_buffer);
    vkEndCommandBuffer(fd.command_buffer);
}

} // namespace application