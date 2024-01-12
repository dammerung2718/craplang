#include <assert.h>
#include <string.h>
#include "shared.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/* CONFIGURATION MADAFAKARS */
const int WIDTH = 640;
const int HEIGHT = 480;
const char TITLE[] = "Hello World";

/* return required extensions */
const char **requiredExtensions(uint32_t *length) {
  const char **glfwExts = glfwGetRequiredInstanceExtensions(length);

  (*length)++;
  const char **exts = malloc(*length * sizeof(char *));
  memcpy(exts, glfwExts, (*length - 1) * sizeof(char *));
  exts[*length - 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

  return exts;
}

/* finds queue families */
struct QueueIndices {
  int *graphicsFamily;
  int *presentFamily;
};

struct QueueIndices findRequiredQueueFamilies(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR surface) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           NULL);

  VkQueueFamilyProperties *queueFamilies =
      calloc(queueFamilyCount, sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies);

  struct QueueIndices queueIndices;
  for (int i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queueIndices.graphicsFamily = malloc(sizeof(int));
      *queueIndices.graphicsFamily = i;
    }

    VkBool32 presentSupport = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                         &presentSupport);
    if (presentSupport) {
      queueIndices.presentFamily = malloc(sizeof(int));
      *queueIndices.presentFamily = i;
    }
  }

  assert(queueIndices.graphicsFamily != NULL);
  assert(queueIndices.presentFamily != NULL);

  return queueIndices;
}

/*
 * for macOS, we only have 1 anyways so this is okay
 */
VkPhysicalDevice pickPhysicalDevice(VkInstance instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  assert(deviceCount == 1);

  VkPhysicalDevice *physicalDevices =
      calloc(deviceCount, sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

  return physicalDevices[0];
}

int alreadyVisited(int *visited, int len, int index) {
  for (int i = 0; i < len; i++) {
    if (visited[i] == index) {
      return 1;
    }
  }

  return 0;
}

VkDeviceQueueCreateInfo *
createQueueCreateInfos(struct QueueIndices queueIndices, float *queuePriority,
                       int *uniqueIndicesLen) {
  const int indicesLen = 2;

  int uniqueIndices[1024] = {0};
  int indices[indicesLen] = {*queueIndices.graphicsFamily,
                             *queueIndices.presentFamily};

  *uniqueIndicesLen = 0;

  for (int i = 0; i < indicesLen; i++) {
    int index = indices[i];
    if (alreadyVisited(uniqueIndices, *uniqueIndicesLen, index)) {
      continue;
    }
    uniqueIndices[(*uniqueIndicesLen)++] = index;
  }

  VkDeviceQueueCreateInfo *createInfos =
      calloc(*uniqueIndicesLen, sizeof(VkDeviceQueueCreateInfo));
  for (int i = 0; i < *uniqueIndicesLen; i++) {
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = uniqueIndices[i];
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriority;
    createInfos[i] = queueCreateInfo;
  }

  return createInfos;
}

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;

  VkSurfaceFormatKHR *formats;
  uint32_t formatsLen;

  VkPresentModeKHR *presentModes;
  uint32_t presentModesLen;
};

struct SwapChainSupportDetails
querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  struct SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &details.capabilities);

  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                       &details.formatsLen, NULL);
  details.formats = calloc(details.formatsLen, sizeof(VkSurfaceFormatKHR));
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                       &details.formatsLen, details.formats);

  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &details.presentModesLen, NULL);
  details.presentModes =
      calloc(details.presentModesLen, sizeof(VkPresentModeKHR));
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &details.presentModesLen, details.presentModes);

  return details;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(struct SwapChainSupportDetails details) {
  for (int i = 0; i < details.formatsLen; i++) {
    VkSurfaceFormatKHR format = details.formats[i];
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }

  return details.formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(struct SwapChainSupportDetails details) {
  for (int i = 0; i < details.presentModesLen; i++) {
    if (details.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return details.presentModes[i];
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

VkExtent2D chooseSwapExtent(struct SwapChainSupportDetails details,
                            GLFWwindow *window) {
  if (details.capabilities.currentExtent.width != UINT32_MAX) {
    return details.capabilities.currentExtent;
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D actualExtent = {width, height};
  clamp(actualExtent.width, details.capabilities.minImageExtent.width,
        details.capabilities.maxImageExtent.width);
  actualExtent.height =
      clamp(actualExtent.height, details.capabilities.minImageExtent.height,
            details.capabilities.maxImageExtent.height);

  return actualExtent;
}

VkShaderModule createShaderModule(struct FileContents code, VkDevice device) {
  VkShaderModuleCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size;
  createInfo.pCode = (uint32_t *)code.data;

  VkShaderModule shaderModule;
  VkResult result =
      vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);
  if (result != VK_SUCCESS) {
    die("failed to create shader module! %d", result);
  }

  return shaderModule;
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                         VkRenderPass renderPass, VkFramebuffer *framebuffers,
                         VkExtent2D extent, VkPipeline graphicsPipeline) {
  VkCommandBufferBeginInfo beginInfo = {0};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;               // Optional
  beginInfo.pInheritanceInfo = NULL; // Optional

  VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  if (result != VK_SUCCESS) {
    die("failed to begin recording command buffer! %d", result);
  }

  VkRenderPassBeginInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = framebuffers[imageIndex];
  renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
  renderPassInfo.renderArea.extent = extent;

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)extent.width;
  viewport.height = (float)extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.offset = (VkOffset2D){0, 0};
  scissor.extent = extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  result = vkEndCommandBuffer(commandBuffer);
  if (result != VK_SUCCESS) {
    die("failed to record command buffer! %d", result);
  }
}

/* entrypoint */
int main() {
  /* init some libs */
  if (!glfwInit()) {
    return -1;
  }

  /* don't create opengl context */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  /* make a window tho */
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* vulkan instance */
  VkInstance instance;

  VkApplicationInfo appInfo = {0};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = TITLE;
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  uint32_t extCount = 0;
  const char **exts = requiredExtensions(&extCount);

  VkInstanceCreateInfo instanceCreateInfo = {0};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledExtensionCount = extCount;
  instanceCreateInfo.ppEnabledExtensionNames = exts;
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  VkResult result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
  if (vkCreateInstance(&instanceCreateInfo, NULL, &instance) != VK_SUCCESS) {
    die("failed to create instance! %d", result);
  }

  /* create a surface so vulkan can draw on glfw */
  VkSurfaceKHR surface;
  result = glfwCreateWindowSurface(instance, window, NULL, &surface);
  if (result != VK_SUCCESS) {
    die("failed to create surface! %d", result);
  }

  /* find physical device */
  VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance);

  /* find required queue families */
  struct QueueIndices queueIndices =
      findRequiredQueueFamilies(physicalDevice, surface);

  /* create logical device */
  float queuePriority = 1.0f;
  int queueCreateInfoCount = 0;
  VkDeviceQueueCreateInfo *queueCreateInfos = createQueueCreateInfos(
      queueIndices, &queuePriority, &queueCreateInfoCount);

  VkPhysicalDeviceFeatures deviceFeatures = {0};

  const int deviceExtLen = 1;
  const char *deviceExt[deviceExtLen] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  VkDeviceCreateInfo deviceCreateInfo = {0};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
  deviceCreateInfo.queueCreateInfoCount = queueCreateInfoCount;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.ppEnabledExtensionNames = deviceExt;
  deviceCreateInfo.enabledExtensionCount = deviceExtLen;
  deviceCreateInfo.enabledLayerCount = 0;

  VkDevice device;
  result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);
  if (result != VK_SUCCESS) {
    die("failed to create logical device! %d", result);
  }

  /* get handles for required queue families */
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  vkGetDeviceQueue(device, *queueIndices.graphicsFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(device, *queueIndices.presentFamily,
                   0 /* NOTE: i do not know why this has to be zero */,
                   &presentQueue);

  /* create the swapchain */
  struct SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice, surface);
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapChainCreateInfo = {0};
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = surface;

  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = extent;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {*queueIndices.graphicsFamily,
                                   *queueIndices.presentFamily};
  if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;  // Optional
    swapChainCreateInfo.pQueueFamilyIndices = NULL; // Optional
  }

  swapChainCreateInfo.preTransform =
      swapChainSupport.capabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  swapChainCreateInfo.presentMode = presentMode;
  swapChainCreateInfo.clipped = VK_TRUE;

  swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swapChain;
  result = vkCreateSwapchainKHR(device, &swapChainCreateInfo, NULL, &swapChain);
  if (result != VK_SUCCESS) {
    die("failed to create swap chain! %d", result);
  }

  /* aquire images from swapchain */
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
  VkImage *swapChainImages = calloc(imageCount, sizeof(VkImage));
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

  /* create image views from images */
  VkImageView *imageViews = calloc(imageCount, sizeof(VkImageView));
  for (size_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];

    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = surfaceFormat.format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(device, &createInfo, NULL, &imageViews[i]);
    if (result != VK_SUCCESS) {
      die("failed to create image views! %d", result);
    }
  }

  /* create render pass */
  VkAttachmentDescription colorAttachment = {0};
  colorAttachment.format = surfaceFormat.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {0};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency = {0};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkRenderPass renderPass;
  result = vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass);
  if (result != VK_SUCCESS) {
    die("failed to create render pass! %d", result);
  }

  /* create pipeline layout */
  VkShaderModule vertShaderModule =
      createShaderModule(readFileContents("bin/vert.spv"), device);
  VkShaderModule fragShaderModule =
      createShaderModule(readFileContents("bin/frag.spv"), device);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  const uint32_t dynamicStatesLen = 2;
  VkDynamicState dynamicStates[dynamicStatesLen] = {VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState = {0};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = dynamicStatesLen;
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = NULL;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = NULL;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)extent.width;
  viewport.height = (float)extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {0};
  scissor.offset = (VkOffset2D){0, 0};
  scissor.extent = extent;

  VkPipelineViewportStateCreateInfo viewportState = {0};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {0};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {0};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = NULL;               // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending = {0};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;         // Optional
  pipelineLayoutInfo.pSetLayouts = NULL;         // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

  VkPipelineLayout pipelineLayout;
  result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL,
                                  &pipelineLayout);
  if (result != VK_SUCCESS) {
    die("failed to create pipeline layout! %d", result);
  }

  /* create graphics pipeline */
  VkGraphicsPipelineCreateInfo pipelineInfo = {0};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = NULL;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1;              // Optional

  VkPipeline graphicsPipeline;
  result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     NULL, &graphicsPipeline);
  if (result != VK_SUCCESS) {
    die("failed to create graphics pipeline! %d", result);
  }

  /* create framebuffers */
  VkFramebuffer *swapChainFramebuffers =
      calloc(imageCount, sizeof(VkFramebuffer));
  for (int i = 0; i < imageCount; i++) {
    VkImageView attachments[] = {imageViews[i]};

    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    result = vkCreateFramebuffer(device, &framebufferInfo, NULL,
                                 &swapChainFramebuffers[i]);
    if (result != VK_SUCCESS) {
      die("failed to create framebuffer! %d", result);
    }
  }

  /* create the command pool */
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = *queueIndices.graphicsFamily;

  VkCommandPool commandPool;
  result = vkCreateCommandPool(device, &poolInfo, NULL, &commandPool);
  if (result != VK_SUCCESS) {
    die("failed to create command pool! %d", result);
  }

  /* create a command buffer */
  VkCommandBufferAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
  if (result != VK_SUCCESS) {
    die("failed to allocate command buffers! %d", result);
  }

  /* create sync objects */
  VkSemaphoreCreateInfo semaphoreInfo = {0};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {0};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphore imageAvailableSemaphore;
  result =
      vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore);
  if (result != VK_SUCCESS) {
    die("failed to create imageAvailableSemaphore! %d", result);
  }

  VkSemaphore renderFinishedSemaphore;
  result =
      vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphore);
  if (result != VK_SUCCESS) {
    die("failed to create renderFinishedSemaphore! %d", result);
  }

  VkFence inFlightFence;
  result = vkCreateFence(device, &fenceInfo, NULL, &inFlightFence);
  if (result != VK_SUCCESS) {
    die("failed to create inFlightFence! %d", result);
  }

  /* main loop */
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    /* wait for previous frame */
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    /* get the next image */
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    /* record commands */
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex, renderPass,
                        swapChainFramebuffers, extent, graphicsPipeline);

    /* submit command buffer */
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    if (result != VK_SUCCESS) {
      die("failed to submit draw command buffer! %d", result);
    }

    /* give the image back for presentation */
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(presentQueue, &presentInfo);
  }

  /* cleanup */
  vkDestroyFence(device, inFlightFence, NULL);
  vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
  vkDestroySemaphore(device, imageAvailableSemaphore, NULL);
  vkDestroyCommandPool(device, commandPool, NULL);
  for (int i = 0; i < imageCount; i++) {
    vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
  }
  vkDestroyPipeline(device, graphicsPipeline, NULL);
  vkDestroyPipelineLayout(device, pipelineLayout, NULL);
  vkDestroyRenderPass(device, renderPass, NULL);
  vkDestroyShaderModule(device, fragShaderModule, NULL);
  vkDestroyShaderModule(device, vertShaderModule, NULL);
  for (int i = 0; i < imageCount; i++) {
    vkDestroyImageView(device, imageViews[i], NULL);
  }
  vkDestroySwapchainKHR(device, swapChain, NULL);
  vkDestroyDevice(device, NULL);
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
