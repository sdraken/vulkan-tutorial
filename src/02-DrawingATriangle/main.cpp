#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>

//window dimensions
const uint32_t WIDTH = 800;     
const uint32_t HEIGHT = 600;

//standard validation layers is bundled into layer in SDK known as VK_LAYER_KHRONOS_validation
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

//enable validation Layers if we're debugging
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

//proxy function to extension function CreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//proxy function to extension function DestroyDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

//struct to keep track of indices of queue families (std::optional is used since all values of uint32_t can be an index)
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication {
    public:

        //Called from main, starts everything
        void run(){
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        GLFWwindow* window;
        
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;

        //initiates GLFW and creates a window
        void initWindow(){
            glfwInit();
            
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);                   //tells GLFW not to make OpenGL context (default)
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);                     //disable resizable windows

            window = glfwCreateWindow(WIDTH,HEIGHT,"Vulkan",nullptr,nullptr);    //creates window
        }

        //initiates all Vulkan objects
        void initVulkan(){
            createInstance();
            setupDebugMessenger();
            pickPhysicalDevice();
            createLogicalDevice();
        }

        //loop while window remains open
        void mainLoop(){
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }

        //deallocate used resources
        void cleanup(){
            vkDestroyDevice(device, nullptr);

            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            }

            vkDestroyInstance(instance, nullptr);

            glfwDestroyWindow(window);

            glfwTerminate();
        }

        //create VkInstance
        void createInstance(){
            if (enableValidationLayers && !checkValidationLayerSupport()){
                throw std::runtime_error("validation layers requested, but not available!");
            }

            //optional information but useful for optimiziation
            VkApplicationInfo appInfo{
                VK_STRUCTURE_TYPE_APPLICATION_INFO,
                nullptr,
                "Hello Triangle",
                VK_MAKE_VERSION(1, 0, 0),
                "No Engine",
                VK_MAKE_VERSION(1, 0, 0),
                VK_API_VERSION_1_0
                };


            //get required Vulkan extensions by GLFW
            auto extensions = getRequiredExtensions();

            //default values of 3 createInfo parameters
            uint32_t layerCount = 0;
            const char* const* layerNames = nullptr;
            const void* pNext = nullptr;

            

            //change createInfo values if we're debugging
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};               //declare outside of if statement to prevent out of scope
            if(enableValidationLayers){
                layerCount = static_cast<uint32_t>(validationLayers.size());
                layerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
            }

            //required information about what extensions and validation layers to be used
            VkInstanceCreateInfo createInfo{
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                pNext,
                0,
                &appInfo,
                layerCount,
                layerNames,
                static_cast<uint32_t>(extensions.size()),
                extensions.data()
                };


            //create VkInstance object
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }

        //setup what kind of error messages we care about
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }

        //create VkDebugUtilsMessengerEXT
        void setupDebugMessenger() {
            if (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }
        
        //go through all graphics cards and pick first suitable
        void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

        //create VkDevice and its 
        void createLogicalDevice() {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;

            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{};                              //empty right now but we can add device features if needed

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = 0;

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        }

        //does the device have required queue families?
        bool isDeviceSuitable(VkPhysicalDevice device) {
            QueueFamilyIndices indices = findQueueFamilies(device);

            return indices.isComplete();
        }

        //return QueueFamilyIndices struct that tells what queue families a VkPhysicalDevice supports and what indices they have
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        //get required GLFW extensions and extensions needed for VkDebugUtilsMessengerEXT if we're debugging
        std::vector<const char*> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        //check if all validation layers in validationLayers is available
        bool checkValidationLayerSupport(){
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        //callback function to validation layer error messages
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }
};


int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}