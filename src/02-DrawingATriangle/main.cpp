#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

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
        }

        //loop while window remains open
        void mainLoop(){
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }

        //deallocate used resources
        void cleanup(){
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