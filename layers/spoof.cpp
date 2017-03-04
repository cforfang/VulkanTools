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

//#define SPOOF_DEBUG

#ifdef ANDROID

#elif defined(__linux__)

static inline char *local_getenv(const char *name) { return getenv(name); }

static inline void local_free_getenv(const char *val) {}

#elif defined(_WIN32)
static inline char *local_getenv(const char *name) {
    char *retVal;
    DWORD valSize;

    valSize = GetEnvironmentVariableA(name, NULL, 0);

    // valSize DOES include the null terminator, so for any set variable
    // will always be at least 1. If it's 0, the variable wasn't set.
    if (valSize == 0)
        return NULL;

    // TODO; FIXME This should be using any app defined memory allocation
    retVal = (char *)malloc(valSize);

    GetEnvironmentVariableA(name, retVal, valSize);

    return retVal;
}

static inline void local_free_getenv(const char *val) { free((void *)val); }
#endif

static std::unordered_map<dispatch_key, VkInstance> spoof_instance_map;
static std::mutex global_lock; // Protect map accesses and unique_id increments

//Spoof Layer data to store spoofed GPU information
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

//Spoof Layer EXT APIs
typedef VkResult(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice,
                                                                                        const VkPhysicalDeviceLimits *limits);
static PFN_vkGetOriginalPhysicalDeviceLimitsEXT pfn_get_original_device_limits_extension;

typedef VkResult(VKAPI_PTR *PFN_vkSetPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice,
                 const VkPhysicalDeviceLimits *newLimits);
static PFN_vkSetPhysicalDeviceLimitsEXT pfn_set_physical_device_limits_extension;

VKAPI_ATTR void VKAPI_CALL
spoof_GetOriginalPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits *orgLimits) {
    //unwrapping the physicalDevice in order to get the same physicalDevice address which loader wraps
    //We have to do this as this API is a layer extension and loader does not wrap like regular API in vulkan.h
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: %p\n", (void *)physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: unwrap %p\n", (void *)unwrapped_phys_dev);
#endif
    {
        std::lock_guard<std::mutex> lock(global_lock);

        VkPhysicalDeviceProperties pImplicitProperties;
        instance_dispatch_table(unwrapped_phys_dev)->GetPhysicalDeviceProperties(unwrapped_phys_dev, &pImplicitProperties);

        if (orgLimits)
            memcpy(orgLimits, &pImplicitProperties.limits, sizeof(VkPhysicalDeviceLimits));
    }
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: vkGetOriginalDeviceLimitsEXT SUCCESS\n");
#endif
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_SetPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits) {

    //unwrapping the physicalDevice in order to get the same physicalDevice address which loader wraps
    //We have to do this as this API is a layer extension and loader does not wrap like regular API in vulkan.h
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: %p\n", (void *)physicalDevice);
    printf("VK_LAYER_LUNARG_Spoof: ARDA2 In vkSetDeviceLimitsEXT() call w/ gpu: unwrap %p\n", (void *)unwrapped_phys_dev);
#endif
    {
        std::lock_guard<std::mutex> lock(global_lock);

        //search if we got the device limits for this device and stored in spoof layer
        auto spoof_data_it = spoof_dev_data_map.find(unwrapped_phys_dev);
        //if we do not have it call getDeviceProperties implicitly and store device properties in the spoof_layer
        if (spoof_data_it != spoof_dev_data_map.end()) {
            if (spoof_dev_data_map[unwrapped_phys_dev].props && newLimits)
                memcpy(&(spoof_dev_data_map[unwrapped_phys_dev].props->limits), newLimits, sizeof(VkPhysicalDeviceLimits));
        }
#ifdef SPOOF_DEBUG
        else { //spoof layer device limits exists for this device so set the new limits
            printf("VK_LAYER_LUNARG_Spoof: newLimits is NULL! \n");
        }
#endif
    }
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: vkSetDeviceLimitsEXT returning SUCCESS\n");
#endif
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkCreateInstance() call w/ inst: %p\n", (void *)pInstance);
#endif
    std::lock_guard<std::mutex> lock(global_lock);

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
    VkPhysicalDevice* physicalDevices;
    result = instance_dispatch_table(*pInstance)->EnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, NULL);
    if (result != VK_SUCCESS)
        return result;

    physicalDevices = (VkPhysicalDevice*)malloc(sizeof(physicalDevices[0]) * physicalDeviceCount);
    result = instance_dispatch_table(*pInstance)->EnumeratePhysicalDevices(*pInstance, &physicalDeviceCount, physicalDevices);
    if (result != VK_SUCCESS)
        return result;

    //first of all get original physical device limits
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
                if (i == 0) {
                    return VK_ERROR_OUT_OF_HOST_MEMORY;
                }
                else{ // free others
                    for(uint8_t j = 0; j < i; j++) {
                        if (spoof_dev_data_map[physicalDevices[j]].props) {
                            free(spoof_dev_data_map[physicalDevices[j]].props);
                            spoof_dev_data_map[physicalDevices[j]].props = nullptr;
                        }
                    }
                }
            }
        }
    } 
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkCreateInstance() call w/ inst: %p\n", *pInstance);
#endif
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_EnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkEnumeratePhysicalDevices() call w/ inst: %p\n", (void *)instance);
#endif
    VkResult result = instance_dispatch_table(instance)->EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkEnumeratePhysicalDevices() call w/ count %u\n", *pPhysicalDeviceCount);
#endif
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
spoof_CreateDevice(VkPhysicalDevice physicalDevice,
                   const VkDeviceCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkDevice *pDevice) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: At start of vkCreateDevice() call w/ gpu: %p\n", (void *)physicalDevice);
#endif

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

    pfn_get_original_device_limits_extension =
                    (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)fpGetDeviceProcAddr(*pDevice, "vkGetOriginalPhysicalDeviceLimitsEXT");
    pfn_set_physical_device_limits_extension =
                                    (PFN_vkSetPhysicalDeviceLimitsEXT)fpGetDeviceProcAddr(*pDevice, "vkSetPhysicalDeviceLimitsEXT");
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Completed vkCreateDevice() call w/ pDevice, Device %p: %p\n", (void *)pDevice, (void *)*pDevice);
#endif
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
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceFeatures() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceFeatures() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                        VkFormat format,
                                        VkFormatProperties *pFormatProperties) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceImageFormatProperties( VkPhysicalDevice physicalDevice,
                                              VkFormat format,
                                              VkImageType type,
                                              VkImageTiling tiling,
                                              VkImageUsageFlags usage,
                                              VkImageCreateFlags flags,
                                              VkImageFormatProperties* pImageFormatProperties) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceImageFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceImageFormatProperties(physicalDevice,
                                                                                    format,
                                                                                    type,
                                                                                    tiling,
                                                                                    usage,
                                                                                    flags,
                                                                                    pImageFormatProperties);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceImageFormatProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceProperties( VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties*  pProperties) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: ARDA start vkGetPhysicalDeviceProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
    {
        std::lock_guard<std::mutex> lock(global_lock);

        //search if we got the device limits for this device and stored in spoof layer
        auto spoof_data_it = spoof_dev_data_map.find(physicalDevice);
        if (spoof_data_it != spoof_dev_data_map.end()) {
            //spoof layer device limits exists for this device so overwrite with desired limits
            if(spoof_dev_data_map[physicalDevice].props)
                memcpy(pProperties, spoof_dev_data_map[physicalDevice].props, sizeof(VkPhysicalDeviceProperties));
        } else {
            instance_dispatch_table(physicalDevice)->GetPhysicalDeviceProperties(physicalDevice, pProperties);
        }
    }
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: ARDA Compl vkGetPhysicalDeviceProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                             uint32_t* pQueueFamilyPropertyCount,
                                             VkQueueFamilyProperties* pQueueFamilyProperties) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: start vkGetPhysicalDeviceQueueFamilyProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                                                    pQueueFamilyPropertyCount,
                                                                                    pQueueFamilyProperties);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Compl vkGetPhysicalDeviceQueueFamilyProperties() call w/ gpu: %p\n", (void *)physicalDevice);
#endif
}

VKAPI_ATTR void VKAPI_CALL
spoof_GetPhysicalDeviceMemoryProperties( VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: At start of wrapped vkGetPhysicalDeviceMemoryProperties() call w/ gpu: %p\n",
                                                                                                           (void *)physicalDevice);
#endif
    instance_dispatch_table(physicalDevice)->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
#ifdef SPOOF_DEBUG
    printf("VK_LAYER_LUNARG_Spoof: Completed wrapped vkGetPhysicalDeviceMemoryProperties() call w/ gpu: %p\n",
                                                                                                           (void *)physicalDevice);
#endif
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

    return instance_dispatch_table(physicalDevice)->EnumerateDeviceExtensionProperties(physicalDevice, 
                                                                                                  pLayerName, pCount, pProperties);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
spoof_GetDeviceProcAddr(VkDevice device, const char *pName) {
    if (!strcmp("vkGetDeviceProcAddr", pName))
        return (PFN_vkVoidFunction)spoof_GetDeviceProcAddr;
    if (!strcmp("vkDestroyDevice", pName))
        return (PFN_vkVoidFunction)spoof_DestroyDevice;
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
    if (!strcmp("vkSetPhysicalDeviceLimitsEXT", pName))
        return (PFN_vkVoidFunction)spoof_SetPhysicalDeviceLimitsEXT;
    if (!strcmp("vkGetOriginalPhysicalDeviceLimitsEXT", pName))
        return (PFN_vkVoidFunction)spoof_GetOriginalPhysicalDeviceLimitsEXT;

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
