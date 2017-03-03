/*
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
 * Copyright (C) 2015 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Arda Coskunses <arda@lunarg.com>
 */
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unordered_map>
#include "loader.h"
#include "vk_dispatch_table_helper.h"
#include "vulkan/vk_lunarg_spoof-layer.h"
#include "vulkan/vk_spoof_layer.h"
#include "vulkan/vk_layer.h"
#include "vk_layer_table.h"
#include "mutex"

#include <fstream>
#include <iostream>
#include <sstream>

#include <json/json.h>

static std::unordered_map<dispatch_key, VkInstance> spoof_instance_map;
static std::mutex global_lock; // Protect map accesses and unique_id increments

/*
struct AppGpu {
uint32_t id;
VkPhysicalDevice obj;

VkPhysicalDeviceProperties props;

uint32_t queue_count;
VkQueueFamilyProperties *queue_props;
VkDeviceQueueCreateInfo *queue_reqs;

VkPhysicalDeviceMemoryProperties memory_props;
VkPhysicalDeviceFeatures features;
VkPhysicalDevice limits;

uint32_t device_extension_count;
VkExtensionProperties *device_extensions;

struct AppDev dev;
};

typedef struct VkPhysicalDeviceProperties {
    uint32_t                            apiVersion;
    uint32_t                            driverVersion;
    uint32_t                            vendorID;
    uint32_t                            deviceID;
    VkPhysicalDeviceType                deviceType;
    char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
    VkPhysicalDeviceLimits              limits;
    VkPhysicalDeviceSparseProperties    sparseProperties;
} VkPhysicalDeviceProperties;

The VkPhysicalDeviceLimits are properties of the physical device.These are available in the limits member of the 
VkPhysicalDeviceProperties structure which is returned from vkGetPhysicalDeviceProperties.

typedef struct VkPhysicalDeviceLimits {
uint32_t              maxImageDimension1D;
uint32_t              maxImageDimension2D;
uint32_t              maxImageDimension3D;
uint32_t              maxImageDimensionCube;
uint32_t              maxImageArrayLayers;
uint32_t              maxTexelBufferElements;
uint32_t              maxUniformBufferRange;
uint32_t              maxStorageBufferRange;
uint32_t              maxPushConstantsSize;
uint32_t              maxMemoryAllocationCount;
uint32_t              maxSamplerAllocationCount;
VkDeviceSize          bufferImageGranularity;
VkDeviceSize          sparseAddressSpaceSize;
uint32_t              maxBoundDescriptorSets;
uint32_t              maxPerStageDescriptorSamplers;
uint32_t              maxPerStageDescriptorUniformBuffers;
uint32_t              maxPerStageDescriptorStorageBuffers;
uint32_t              maxPerStageDescriptorSampledImages;
uint32_t              maxPerStageDescriptorStorageImages;
uint32_t              maxPerStageDescriptorInputAttachments;
uint32_t              maxPerStageResources;
uint32_t              maxDescriptorSetSamplers;
uint32_t              maxDescriptorSetUniformBuffers;
uint32_t              maxDescriptorSetUniformBuffersDynamic;
uint32_t              maxDescriptorSetStorageBuffers;
uint32_t              maxDescriptorSetStorageBuffersDynamic;
uint32_t              maxDescriptorSetSampledImages;
uint32_t              maxDescriptorSetStorageImages;
uint32_t              maxDescriptorSetInputAttachments;
uint32_t              maxVertexInputAttributes;
uint32_t              maxVertexInputBindings;
uint32_t              maxVertexInputAttributeOffset;
uint32_t              maxVertexInputBindingStride;
uint32_t              maxVertexOutputComponents;
uint32_t              maxTessellationGenerationLevel;
uint32_t              maxTessellationPatchSize;
uint32_t              maxTessellationControlPerVertexInputComponents;
uint32_t              maxTessellationControlPerVertexOutputComponents;
uint32_t              maxTessellationControlPerPatchOutputComponents;
uint32_t              maxTessellationControlTotalOutputComponents;
uint32_t              maxTessellationEvaluationInputComponents;
uint32_t              maxTessellationEvaluationOutputComponents;
uint32_t              maxGeometryShaderInvocations;
uint32_t              maxGeometryInputComponents;
uint32_t              maxGeometryOutputComponents;
uint32_t              maxGeometryOutputVertices;
uint32_t              maxGeometryTotalOutputComponents;
uint32_t              maxFragmentInputComponents;
uint32_t              maxFragmentOutputAttachments;
uint32_t              maxFragmentDualSrcAttachments;
uint32_t              maxFragmentCombinedOutputResources;
uint32_t              maxComputeSharedMemorySize;
uint32_t              maxComputeWorkGroupCount[3];
uint32_t              maxComputeWorkGroupInvocations;
uint32_t              maxComputeWorkGroupSize[3];
uint32_t              subPixelPrecisionBits;
uint32_t              subTexelPrecisionBits;
uint32_t              mipmapPrecisionBits;
uint32_t              maxDrawIndexedIndexValue;
uint32_t              maxDrawIndirectCount;
float                 maxSamplerLodBias;
float                 maxSamplerAnisotropy;
uint32_t              maxViewports;
uint32_t              maxViewportDimensions[2];
float                 viewportBoundsRange[2];
uint32_t              viewportSubPixelBits;
size_t                minMemoryMapAlignment;
VkDeviceSize          minTexelBufferOffsetAlignment;
VkDeviceSize          minUniformBufferOffsetAlignment;
VkDeviceSize          minStorageBufferOffsetAlignment;
int32_t               minTexelOffset;
uint32_t              maxTexelOffset;
int32_t               minTexelGatherOffset;
uint32_t              maxTexelGatherOffset;
float                 minInterpolationOffset;
float                 maxInterpolationOffset;
uint32_t              subPixelInterpolationOffsetBits;
uint32_t              maxFramebufferWidth;
uint32_t              maxFramebufferHeight;
uint32_t              maxFramebufferLayers;
VkSampleCountFlags    framebufferColorSampleCounts;
VkSampleCountFlags    framebufferDepthSampleCounts;
VkSampleCountFlags    framebufferStencilSampleCounts;
VkSampleCountFlags    framebufferNoAttachmentsSampleCounts;
uint32_t              maxColorAttachments;
VkSampleCountFlags    sampledImageColorSampleCounts;
VkSampleCountFlags    sampledImageIntegerSampleCounts;
VkSampleCountFlags    sampledImageDepthSampleCounts;
VkSampleCountFlags    sampledImageStencilSampleCounts;
VkSampleCountFlags    storageImageSampleCounts;
uint32_t              maxSampleMaskWords;
VkBool32              timestampComputeAndGraphics;
float                 timestampPeriod;
uint32_t              maxClipDistances;
uint32_t              maxCullDistances;
uint32_t              maxCombinedClipAndCullDistances;
uint32_t              discreteQueuePriorities;
float                 pointSizeRange[2];
float                 lineWidthRange[2];
float                 pointSizeGranularity;
float                 lineWidthGranularity;
VkBool32              strictLines;
VkBool32              standardSampleLocations;
VkDeviceSize          optimalBufferCopyOffsetAlignment;
VkDeviceSize          optimalBufferCopyRowPitchAlignment;
VkDeviceSize          nonCoherentAtomSize;
} VkPhysicalDeviceLimits;
*/

struct spoof_data{
    VkPhysicalDeviceProperties *props;

    VkQueueFamilyProperties *queue_props;
    VkDeviceQueueCreateInfo *queue_reqs;

    VkPhysicalDeviceMemoryProperties memory_props;
    VkPhysicalDeviceFeatures features;

    uint32_t device_extension_count;
    VkExtensionProperties *device_extensions;
};

static std::unordered_map<VkPhysicalDevice, struct spoof_data> spoof_dev_data_map;

typedef VkResult(VKAPI_PTR *PFN_vkLayerSpoofEXT)(VkPhysicalDevice physicalDevice);
static PFN_vkLayerSpoofEXT pfn_layer_extension;

typedef VkResult(VKAPI_PTR *PFN_vkSetPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice,
                 const VkPhysicalDeviceLimits *newLimits);
static PFN_vkSetPhysicalDeviceLimitsEXT pfn_set_physical_device_limits_extension;

VKAPI_ATTR VkResult VKAPI_CALL
spoof_LayerSpoofEXT(VkPhysicalDevice physicalDevice) {
    printf("VK_LAYER_LUNARG_Spoof: In vkLayerSpoofEXT() call w/ gpu: %p\n", (void *)physicalDevice);
    if (pfn_layer_extension) {
        /*this should not happen keeping this code for reference*/
        printf("VK_LAYER_LUNARG_Spoof: In vkLayerSpoofEXT() call down chain\n");
        return pfn_layer_extension(physicalDevice);
    }
    printf("VK_LAYER_LUNARG_Spoof: vkLayerSpoofEXT returning SUCCESS\n");
    return VK_SUCCESS;
}

//bool ReadDriverJson(std::string cur_driver_json, bool &found_lib) {
bool ReadDriverJson(void) {
    bool found_json = false;
    std::ifstream *stream = NULL;
    Json::Value root = Json::nullValue;
    Json::Value inst_exts = Json::nullValue;
    Json::Value dev_exts = Json::nullValue;
    Json::Reader reader;
    char full_json_path[100]="/home/arda-sky/workspace/vulkanreport.json";
    //char generic_string[MAX_STRING_LENGTH];
    uint32_t j = 0;

    volatile bool a = false;
    while(a)
    {
        int k; k++;
    }
    
    printf("ARDA JSON File \n");
    
    stream = new std::ifstream(full_json_path, std::ifstream::in);
    
    if (nullptr == stream || stream->fail()) {
        printf("Error reading JSON file\n");
        printf("GAGA: %s\n", full_json_path);

    }
    
    printf("ARDA JSON File 2\n");

    if (!reader.parse(*stream, root, false) || root.isNull()) {
        printf("Error reading JSON file\n");
        printf("ARDA %s \n", reader.getFormattedErrorMessages());
    }

    printf("ARDA JSON File 3\n");
    
    if (!root["devicefeatures"]["alphaToOne"].isNull()) {
        std::string bak = root["devicefeatures"]["alphaToOne"].asString();
        printf("ARDA LETs see %s \n", bak.c_str());
        printf("ARDA LETs see %s \n", root["devicelimits"]["bufferImageGranularity"].asString().c_str());
        std::string bIG = root["devicelimits"]["bufferImageGranularity"].asString();
        printf("ARDA LETs see %s \n", bIG.c_str());
        //printf("ARDA LETs see %d \n", bIG. );
        //printf("LETs see %d \n", root["devicefeatures"]["alphaToOne"].asUInt());
    } else {
        printf("ARDA MISSING!\n");
    }
    
    printf("ARDA JSON File 4\n");
    
/*
    if (root["ICD"].isNull()) {
        PrintBeginTableRow();
        PrintTableElement("");
        PrintTableElement("ICD Section");
        PrintTableElement("MISSING!");
        PrintEndTableRow();
        goto out;
    }

    found_json = true;

    PrintBeginTableRow();
    PrintTableElement("");
    PrintTableElement("API Version");
    if (!root["ICD"]["api_version"].isNull()) {
        PrintTableElement(root["ICD"]["api_version"].asString());
    } else {
        PrintTableElement("MISSING!");
    }
    PrintEndTableRow();

    PrintBeginTableRow();
    PrintTableElement("");
    PrintTableElement("Library Path");
    if (!root["ICD"]["library_path"].isNull()) {
        std::string driver_name = root["ICD"]["library_path"].asString();
        PrintTableElement(driver_name);
        PrintEndTableRow();
    }*/
    return true;
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetOriginalPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits *orgLimits) {
    //unwrapping the physicalDevice in order to get the same physicalDevice address which loader wraps
    //this part will be carried into loader in the future
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: %p\n", (void *)physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: unwrap %p\n", (void *)unwrapped_phys_dev);
    {
        std::lock_guard<std::mutex> lock(global_lock);

        VkPhysicalDeviceProperties pImplicitProperties;
        instance_dispatch_table(unwrapped_phys_dev)->GetPhysicalDeviceProperties(unwrapped_phys_dev, &pImplicitProperties);

        if (orgLimits)
            memcpy(orgLimits, &pImplicitProperties.limits, sizeof(VkPhysicalDeviceLimits));
    }
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_SetPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits) {

    //unwrapping the physicalDevice in order to get the same physicalDevice address which loader wraps
    //this part will be carried into loader in the future
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: %p\n", (void *)physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: unwrap %p\n", (void *)unwrapped_phys_dev);
    {
        std::lock_guard<std::mutex> lock(global_lock);

        //search if we got the device limits for this device and stored in spoof layer
        auto spoof_data_it = spoof_dev_data_map.find(unwrapped_phys_dev);
        //if we do not have it call getDeviceProperties implicitly and store device properties in the spoof_layer
        if (spoof_data_it == spoof_dev_data_map.end()) {
            //Even this is set Device Lmits function we read Device Properties all toeht for consistency.
            VkPhysicalDeviceProperties pImplicitProperties;
            instance_dispatch_table(unwrapped_phys_dev)->GetPhysicalDeviceProperties(unwrapped_phys_dev, &pImplicitProperties);
            //spoof_dev_data_map.insert()
            spoof_dev_data_map[unwrapped_phys_dev].props =
                (VkPhysicalDeviceProperties*)malloc(sizeof(VkPhysicalDeviceProperties));
            //deep copy the original limits
            if (spoof_dev_data_map[unwrapped_phys_dev].props)
                memcpy(spoof_dev_data_map[unwrapped_phys_dev].props, &pImplicitProperties, sizeof(VkPhysicalDeviceProperties));

            //now set new limits
            if (newLimits) {
                memcpy(&(spoof_dev_data_map[unwrapped_phys_dev].props->limits), newLimits, sizeof(VkPhysicalDeviceLimits));
            }
            else {
                printf("VK_LAYER_LUNARG_Spoof: newLimits is NULL! \n");
            }
        }
        else { //spoof layer device limits exists for this device so set the new limits
            if (spoof_dev_data_map[unwrapped_phys_dev].props && newLimits)
                memcpy(&(spoof_dev_data_map[unwrapped_phys_dev].props->limits), newLimits, sizeof(VkPhysicalDeviceLimits));
        }
    }

    /*
    if (pfn_set_physical_device_limits_extension) {
        //this should not happen keeping this code for reference
        printf("VK_LAYER_LUNARG_Spoof: In vkSetDeviceLimitsEXT() call down chain\n");
        return pfn_set_physical_device_limits_extension(unwrapped_phys_dev, newLimits);
    }*/
    printf("VK_LAYER_LUNARG_Spoof: vkSetDeviceLimitsEXT returning SUCCESS\n");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkCreateInstance() call w/ inst: %p\n", (void *)pInstance);

    std::lock_guard<std::mutex> lock(global_lock);

    ReadDriverJson();
    
    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS)
        return result;

    spoof_instance_map[get_dispatch_key(*pInstance)] = *pInstance;
    initInstanceTable(*pInstance, fpGetInstanceProcAddr);

    uint32_t physicalDeviceCount = 0;
    VkResult err;
    VkPhysicalDevice* physicalDevices;
    err = instance_dispatch_table(*pInstance)->EnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, NULL);
    if (err)
       printf("VK_LAYER_LUNARG_Spoof: ERRRR\n");

    physicalDevices = (VkPhysicalDevice*)malloc(sizeof(physicalDevices[0]) * physicalDeviceCount);
    err = instance_dispatch_table(*pInstance)->EnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, physicalDevices);
    if (err)
        printf("VK_LAYER_LUNARG_Spoof: ERRRR2\n");

    for (uint8_t i = 0; i < physicalDeviceCount; i++) {
        //search if we got the device limits for this device and stored in spoof layer
        auto spoof_data_it = spoof_dev_data_map.find(physicalDevices[i]);
        //if we do not have it store device properties in the spoof_layer
        if (spoof_data_it == spoof_dev_data_map.end()) {
            spoof_dev_data_map[physicalDevices[i]].props = (VkPhysicalDeviceProperties*)malloc(sizeof(VkPhysicalDeviceProperties));
            if (spoof_dev_data_map[physicalDevices[i]].props) {
                instance_dispatch_table(*pInstance)->GetPhysicalDeviceProperties(physicalDevices[i],
                                                                                      spoof_dev_data_map[physicalDevices[i]].props);
            }
            else {
                printf(" Out of Memory \n");
            }
        }
    }    
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkCreateInstance() call w/ inst: %p\n", *pInstance);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices) {
    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkEnumeratePhysicalDevices() call w/ inst: %p\n", (void *)instance);
    VkResult result = instance_dispatch_table(instance)->EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkEnumeratePhysicalDevices() call w/ count %u\n", *pPhysicalDeviceCount);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_CreateDevice(VkPhysicalDevice physicalDevice,
                   const VkDeviceCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkDevice *pDevice) {
    printf("VK_LAYER_LUNARG_Spoof: At start of vkCreateDevice() call w/ gpu: %p\n", (void *)physicalDevice);

    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    VkInstance instance = spoof_instance_map[get_dispatch_key(physicalDevice)];
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    /* Advance the link info for the next element on the chain */
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    initDeviceTable(*pDevice, fpGetDeviceProcAddr);

    pfn_layer_extension = (PFN_vkLayerSpoofEXT)fpGetDeviceProcAddr(*pDevice, "vkLayerSpoofEXT");
    pfn_set_physical_device_limits_extension =
                                    (PFN_vkSetPhysicalDeviceLimitsEXT)fpGetDeviceProcAddr(*pDevice, "vkSetPhysicalDeviceLimitsEXT");
    printf("VK_LAYER_LUNARG_Spoof: Completed vkCreateDevice() call w/ pDevice, Device %p: %p\n", (void *)pDevice, (void *)*pDevice);
    return result;
}

/* hook DestroyDevice to remove tableMap entry */
VKAPI_ATTR void VKAPI_CALL
spoof_DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    device_dispatch_table(device)->DestroyDevice(device, pAllocator);
    destroy_device_dispatch_table(key);
}

/* hook DestroyInstance to remove tableInstanceMap entry */
VKAPI_ATTR void VKAPI_CALL
spoof_DestroyInstance(VkInstance instance,
                      const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_dispatch_table(instance)->DestroyInstance(instance, pAllocator);
    destroy_instance_dispatch_table(key);
    spoof_instance_map.erase(key);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                VkPhysicalDeviceFeatures* pFeatures) {
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceFeatures() call w/ gpu: %p\n", (void *)physicalDevice);
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceFeatures() call w/ gpu: %p\n", (void *)physicalDevice);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                        VkFormat format,
                                        VkFormatProperties *pFormatProperties) {
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceImageFormatProperties( VkPhysicalDevice physicalDevice,
                                              VkFormat format,
                                              VkImageType type,
                                              VkImageTiling tiling,
                                              VkImageUsageFlags usage,
                                              VkImageCreateFlags flags,
                                              VkImageFormatProperties* pImageFormatProperties) {
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceImageFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceImageFormatProperties(physicalDevice,
                                                                                    format,
                                                                                    type,
                                                                                    tiling,
                                                                                    usage,
                                                                                    flags,
                                                                                    pImageFormatProperties);
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceImageFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceProperties( VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties*  pProperties) {
    printf("VK_LAYER_LUNARG_Spoof: ARDA start vkGetPhysicalDeviceProperties() call w/ gpu: %p\n", (void *)physicalDevice);
    {
        std::lock_guard<std::mutex> lock(global_lock);

        //search if we got the device limits for this device and stored in spoof layer
        auto spoof_data_it = spoof_dev_data_map.find(physicalDevice);
        if (spoof_data_it != spoof_dev_data_map.end()) {
            //spoof layer device limits exists for this device so overwrite with desired limits
            if(spoof_dev_data_map[physicalDevice].props)
                memcpy( &(pProperties->limits), &(spoof_dev_data_map[physicalDevice].props->limits), 
                                                                                              sizeof(VkPhysicalDeviceLimits));
        } else {
            instance_dispatch_table(physicalDevice)->GetPhysicalDeviceProperties(physicalDevice, pProperties);
        }
    }
    printf("VK_LAYER_LUNARG_Spoof: ARDA Compl vkGetPhysicalDeviceProperties() call w/ gpu: %p\n", (void *)physicalDevice);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                             uint32_t* pQueueFamilyPropertyCount,
                                             VkQueueFamilyProperties* pQueueFamilyProperties) {
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceQueueFamilyProperties() call w/ gpu: %p\n", (void *)physicalDevice);
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                                                    pQueueFamilyPropertyCount,
                                                                                    pQueueFamilyProperties);
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceQueueFamilyProperties() call w/ gpu: %p\n", (void *)physicalDevice);
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkGetPhysicalDeviceMemoryProperties() call w/ gpu: %p\n",
                                                                                                           (void *)physicalDevice);
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkGetPhysicalDeviceMemoryProperties() call w/ gpu: %p\n",
                                                                                                           (void *)physicalDevice);
}

static const VkLayerProperties spoof_LayerProps = {
    "VK_LAYER_LUNARG_spoof",
    VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION), // specVersion
    1,              // implementationVersion
    "LunarG Spoof Layer",
};

static const VkExtensionProperties spoof_physicaldevice_extensions[] = {{
    "vkLayerSpoofEXT", 1,
}};

template<typename T>
VkResult EnumerateProperties(uint32_t src_count, const T *src_props, uint32_t *dst_count, T *dst_props) {
    if (!dst_props || !src_props) {
        *dst_count = src_count;
        return VK_SUCCESS;
    }

    uint32_t copy_count = (*dst_count < src_count) ? *dst_count : src_count;
    memcpy(dst_props, src_props, sizeof(T) * copy_count);
    *dst_count = copy_count;

    return (copy_count == src_count) ? VK_SUCCESS : VK_INCOMPLETE;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return EnumerateProperties(1, &spoof_LayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount, VkLayerProperties *pProperties) {
    return EnumerateProperties(1, &spoof_LayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, spoof_LayerProps.layerName))
        return EnumerateProperties<VkExtensionProperties>(0, NULL, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                         const char *pLayerName, uint32_t *pCount,
                                         VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, spoof_LayerProps.layerName)) {
        uint32_t count = sizeof(spoof_physicaldevice_extensions) /
            sizeof(spoof_physicaldevice_extensions[0]);
        return EnumerateProperties(count, spoof_physicaldevice_extensions, pCount, pProperties);
    }

    return instance_dispatch_table(physicalDevice)
        ->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
spoof_GetDeviceProcAddr(VkDevice device, const char *pName) {
    if (!strcmp("vkGetDeviceProcAddr", pName))
        return (PFN_vkVoidFunction)spoof_GetDeviceProcAddr;
    if (!strcmp("vkDestroyDevice", pName))
        return (PFN_vkVoidFunction)spoof_DestroyDevice;
    //if (!strcmp("vkLayerSpoofEXT", pName))
    //    return (PFN_vkVoidFunction)spoof_LayerSpoofEXT;
    //if (!strcmp("vkSetPhysicalDeviceLimitsEXT", pName))
    //    return (PFN_vkVoidFunction)spoof_SetPhysicalDeviceLimitsEXT;
    if (device == NULL)
        return NULL;

    if (device_dispatch_table(device)->GetDeviceProcAddr == NULL)
        return NULL;
    return device_dispatch_table(device)->GetDeviceProcAddr(device, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
spoof_GetInstanceProcAddr(VkInstance instance, const char *pName) {
    if (!strcmp("vkEnumerateInstanceLayerProperties", pName))
        return (PFN_vkVoidFunction)spoof_EnumerateInstanceLayerProperties;
    if (!strcmp("vkEnumerateDeviceLayerProperties", pName))
        return (PFN_vkVoidFunction)spoof_EnumerateDeviceLayerProperties;
    if (!strcmp("vkEnumerateInstanceExtensionProperties", pName))
        return (PFN_vkVoidFunction)spoof_EnumerateInstanceExtensionProperties;
    if (!strcmp("vkEnumerateDeviceExtensionProperties", pName))
        return (PFN_vkVoidFunction)spoof_EnumerateDeviceExtensionProperties;
    if (!strcmp("vkGetInstanceProcAddr", pName))
        return (PFN_vkVoidFunction)spoof_GetInstanceProcAddr;
    if (!strcmp("vkGetPhysicalDeviceFeatures", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceFeatures;
    if (!strcmp("vkGetPhysicalDeviceFormatProperties", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceFormatProperties;
    if (!strcmp("vkGetPhysicalDeviceImageFormatProperties", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceImageFormatProperties;
    if (!strcmp("vkGetPhysicalDeviceProperties", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceProperties;
    if (!strcmp("vkGetPhysicalDeviceQueueFamilyProperties", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceQueueFamilyProperties;
    if (!strcmp("vkGetPhysicalDeviceMemoryProperties", pName))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceMemoryProperties;
    if (!strcmp("vkCreateInstance", pName))
        return (PFN_vkVoidFunction)spoof_CreateInstance;
    if (!strcmp("vkDestroyInstance", pName))
        return (PFN_vkVoidFunction)spoof_DestroyInstance;
    if (!strcmp("vkCreateDevice", pName))
        return (PFN_vkVoidFunction)spoof_CreateDevice;
    if (!strcmp("vkEnumeratePhysicalDevices", pName))
        return (PFN_vkVoidFunction)spoof_EnumeratePhysicalDevices;
    if (!strcmp("vkLayerSpoofEXT", pName))
        return (PFN_vkVoidFunction)spoof_LayerSpoofEXT;
    if (!strcmp("vkSetPhysicalDeviceLimitsEXT", pName))
        return (PFN_vkVoidFunction)spoof_SetPhysicalDeviceLimitsEXT;

    assert(instance);

    PFN_vkVoidFunction proc = spoof_GetDeviceProcAddr(VK_NULL_HANDLE, pName);
    if (proc)
        return proc;

    if (instance_dispatch_table(instance)->GetInstanceProcAddr == NULL)
        return NULL;
    return instance_dispatch_table(instance)->GetInstanceProcAddr(instance, pName);
}

// loader-layer interface v0, just wrappers since there is only a layer

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return spoof_EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount, VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return spoof_EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
    return spoof_EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                     const char *pLayerName, uint32_t *pCount,
                                     VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return spoof_EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return spoof_GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return spoof_GetInstanceProcAddr(instance, funcName);
}
