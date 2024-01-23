/* -------------------------------------------------------------------------------- *\
|*                               Gunforce Engine                                    *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 thesmartkbd                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           main.cpp                                                         *|
|* Create Time:    2024/1/23 23:46                                                  *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "vulkan/vulkan.h"
// std
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <vector>

struct demo {
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};

void vk_initialize_instance(VkInstance *p_inst)
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = "VkChooseDevice";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "VkChooseDevice";
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = VK_NULL_HANDLE;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
    vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, p_inst);
}

void vk_initialize_physical_devices(VkInstance instance, std::vector<VkPhysicalDevice> &physical_devices)
{
    uint32_t gpu_count;
    vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE);
    physical_devices.resize(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, std::data(physical_devices));
}

void vk_choose_physical_device(std::vector<VkPhysicalDevice> &physical_devices, int *p_gpu_number)
{
    size_t gpu_count;
    gpu_count = std::size(physical_devices);

    uint32_t count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
    memset(count_device_type, 0, sizeof(count_device_type));

    for (int i = 0; i < std::size(physical_devices); i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

    }

    *p_gpu_number = 0;
}

int main()
{
    struct demo *demo;
    demo = (struct demo *) malloc(sizeof(struct demo));

    vk_initialize_instance(&demo->instance);

    std::vector<VkPhysicalDevice> physical_devices;
    vk_initialize_physical_devices(demo->instance, physical_devices);

    int gpu_number;
    vk_choose_physical_device(physical_devices, &gpu_number);
    demo->gpu = physical_devices[gpu_number];
    vkGetPhysicalDeviceProperties(demo->gpu, &demo->properties);
    vkGetPhysicalDeviceFeatures(demo->gpu, &demo->features);

    fprintf(stderr, "Selected GPU: %d, %s\n", gpu_number, demo->properties.deviceName);

    return 0;
}