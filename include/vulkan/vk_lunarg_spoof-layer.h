//
// File: vk_lunarg_spoof-layer.h
//
/*
** Copyright (c) 2015 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
**
** Authors:
**   Arda Coskunses <arda@lunarg.com>
*/

#ifndef __VK_SPOOF_H__
#define __VK_SPOOF_H__

#include "vulkan/vulkan.h"
//#include "vulkan/vk_ext_debug_report.h"
//TODO Do we need this for extension API
//#define VK_DEBUG_MARKER_EXTENSION_NUMBER 6
//#define VK_DEBUG_MARKER_EXTENSION_REVISION 1
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*
***************************************************************************************************
*   Spoof Vulkan Extension API
***************************************************************************************************
*/

#define SPOOF_EXTENSION_NAME "VK_LUNARG_SPOOF"

// ------------------------------------------------------------------------------------------------
// Enumerations
//TODO do we need this for extesnion API
//#define VK_DEBUG_MARKER_ENUM_EXTEND(type, id)    ((type)(VK_DEBUG_MARKER_EXTENSION_NUMBER * -1000 + (id)))

// ------------------------------------------------------------------------------------------------
// API functions

typedef VkResult(VKAPI_PTR *PFN_vkLayerSpoofEXT)(VkPhysicalDevice physicalDevice);
typedef VkResult(VKAPI_PTR *PFN_vkSetPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits);

#ifdef VK_PROTOTYPES

// Spoof Layer extension entrypoints
VKAPI_ATTR VkResult VKAPI_CALL vkLayerSpoofEXT(VkPhysicalDevice physicalDevice);
VKAPI_ATTR VkResult VKAPI_CALL vkSetPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits);

#endif // VK_PROTOTYPES

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __VK_SPOOF_H__

