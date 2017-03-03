//
// File: vk_spoof_layer.h
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

#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vk_lunarg_spoof-layer.h"
#include "vk_layer.h"

typedef struct VkLayerSpoofDispatchTable_
{
    PFN_vkLayerSpoofEXT vkLayerSpoofEXT;
    PFN_vkSetPhysicalDeviceLimitsEXT vkSetPhysicalDeviceLimitsEXT;
} VkLayerSpoofDispatchTable;
