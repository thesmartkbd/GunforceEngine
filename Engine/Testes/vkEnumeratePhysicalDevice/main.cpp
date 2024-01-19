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
|* Create Time:    2024/1/19 13:08                                                  *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <string.h>

static std::unordered_map<VkPhysicalDeviceType, const char *> _VKPhysicalDeviceType = {
        { VK_PHYSICAL_DEVICE_TYPE_OTHER, "VK_PHYSICAL_DEVICE_TYPE_OTHER" },
        { VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU" },
        { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU" },
        { VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU" },
        { VK_PHYSICAL_DEVICE_TYPE_CPU, "VK_PHYSICAL_DEVICE_TYPE_CPU" },
        { VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM, "VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM" },
};

VkInstance create_instance()
{
    VkInstance inst;
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = "vkEnumeratePhysicalDevice";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "vkEnumeratePhysicalDevice";
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &inst);

    return inst;
}

int find_display_gpu(int gpu_number, uint32_t gpu_count, VkPhysicalDevice *physical_devices)
{
    uint32_t display_count = 0;
    VkResult result;
    int gpu_return = gpu_number;
    if (gpu_number >= 0) {
        result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[gpu_number], &display_count, NULL);
        assert(!result);
    } else {
        for (uint32_t i = 0; i < gpu_count; i++) {
            result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[i], &display_count, NULL);
            assert(!result);
            if (display_count) {
                gpu_return = i;
                break;
            }
        }
    }
    if (display_count > 0)
        return gpu_return;
    else
        return -1;
}

void print_gpu_info(VkPhysicalDevice gpu)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(gpu, &properties);
    printf("  %s\n", properties.deviceName);
    printf("    - api version: %u.%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    printf("    - device id: %u\n", properties.deviceID);
    printf("    - device type: %s\n", _VKPhysicalDeviceType[properties.deviceType]);
    printf("\n");
}

int main(int argc, char **argv)
{
    VkInstance inst;
    VkPhysicalDevice gpu;
    uint32_t gpu_count;
    int gpu_number;

    inst = create_instance();
    gpu_number = -1;

    vkEnumeratePhysicalDevices(inst, &gpu_count, VK_NULL_HANDLE);
    VkPhysicalDevice physical_devices[gpu_count];
    vkEnumeratePhysicalDevices(inst, &gpu_count, physical_devices);

    printf("GPU(%u): \n", gpu_count);
    for (int i = 0; i < gpu_count; i++)
        print_gpu_info(physical_devices[i]);

    gpu_number = find_display_gpu(gpu_number, gpu_count, physical_devices);
    if (gpu_number == -1) {
        uint32_t count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
        memset(count_device_type, 0, sizeof(count_device_type));

        VkPhysicalDeviceProperties physicalDeviceProperties;
        for (uint32_t i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
            assert(physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU);
            count_device_type[physicalDeviceProperties.deviceType]++;
        }

        VkPhysicalDeviceType search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_CPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
        }

        for (uint32_t i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
            if (physicalDeviceProperties.deviceType == search_for_device_type) {
                gpu_number = i;
                break;
            }
        }
    }

    assert(gpu_number >= 0);
    gpu = physical_devices[gpu_number];
    printf("Selected GPU:\n");
    print_gpu_info(gpu);

    return 0;
}