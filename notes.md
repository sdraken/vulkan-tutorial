# General ideas
Program starts with initiating all required Vulkan (and GLFW) objects in the **initVulkan** function, we then enter the **mainloop** function to render frames until the window is closed. After that we need to deallocate the resources we've used in the **cleanup** function.

Vulkan objects can be automatically deallocated using **RAII**, smart pointers, or by implementing automatic resource management with a class's constructor/deconstructor. (Look into **pAllocator** parameter found in almost every function related to creating/destroying Vulkan object)

Vulkan technically doesn't need a window if we're only interested in off-screen rendering. Since we want a window we have to either create one ourselves using system calls or by using a library. This tutorial uses GLFW which is a cross platform library for creating and managing windows (as well as inputs). SDL2 is another option

# VkInstance
## Basics
The **VkInstance** object can be seen as the connection between the application and the Vulkan library. We need to specify details about our application and how it'll be using Vulkan.

Vulkan uses structure to pass information instead of function parameters. These structs almost always have a **pNext** entry. This void pointer can point to another structure for more information. For example **pNext** can point to extension information.

**VkInstance** needs a **VkInstanceCreateInfo** structure to be filled out, specifying optional application information (through a pointer to **VkApplicationInfo**) and required information about what extensions/layers are needed.

Once we have enough information we can create a **VkInstance** object with the **vkCreateInstance** function call. The object creation function parameters in Vulkan are usually:

- Pointer to struct with creation info
- Pointer to custom allocator callbacks (**pAllocator** discussed previously)
- Pointer to the variable that stores the handle to the new object

## Extension support
**vkCreateInstance** can throw errors **VK_ERROR_EXTENSION_NOT_PRESENT**if the extensions we requested weren't available. Throwing an error and terminating makes sense if required extensions are missing, but what about optional functionality? This is why it usually makes sense to check for extension support before creating our **VkInstance** object. 

The **vkEnumerateInstanceExtensionProperties** function is used to retrieve the number of supported extensions and a corresponding array of **VkExtensionProperties**. This can be used to only add optional extensions to the list of required extensions if they are actually available.

## Cleanup
The VkInstance needs to be deallocated- This is simply done with the **vkDestroyInstance** function (note that its parameters include **pAllocator** discussed previously)


# Validation layers
## Basics
The Vulkan API has almost no error checking by default. This comes from the design goal of minimal driver overhead. Validation layers is how Vulkan incorporates error checking. Validation layers are optional components that hook into Vulkan function calls to apply additional operations. For example:

- Checking the values of parameters against the specification to detect misuse
- Tracking creation and destruction of objects to find resource leaks
- Checking thread safety by tracking the threads that calls originate from
- Logging every call and its parameters to the standard output
- Tracing Vulkan calls for profiling and replaying

Validation layers can be enabled for debug builds and then disabled for release builds, completely removing the overhead once the error checking isn't needed anymore.

There are two types of validation layers, instance and device specific. instance layers check calls related to global Vulkan objects (like instances), device specific layers only check calls related to a specific GPU

## Layer support
Similarly to how supported extensions can be retrieved with **vkEnumerateInstanceExtensionProperties**, **vkEnumerateInstanceLayerProperties** does the same but for layers. 

## Message callback
By default the validation layers will print debug messages to standard output. If we want to handle this ourselves we need to provide a pointer to a callback function in our program (similar to **pAllocator** but for error messages instead of memory allocation). This will also allow us to decide what kind of errors we care about.

the callback is set up by creating a debug messenger and giving it a pointer to the callback. The **VK_EXT_debug_utils** extension gives us the debug messenger object **VkDebugUtilsMessengerEXT**. We create and later destroy **VkDebugUtilsMessengerEXT** with functions **vkCreateDebugUtilsMessengerEXT** and **DestroyDebugUtilsMessengerEXT**. These function aren't automatically loaded since they're extension functions belonging to **VK_EXT_debug_utils**. To call these functions we use proxy functions and call the **vkGetInstanceProcAddr** function to get pointers to the real **DestroyDebugUtilsMessengerEXT**/**vkCreateDebugUtilsMessengerEXT** functions.

The creation of the **VkDebugUtilsMessengerEXT** object needs an already created **VkInstance**, and the **VkDebugUtilsMessengerEXT** object needs to be deallocated before **VkInstance** is. So our debug messenger can't actually debug any issues in the **vkCreateInstance** and **vkDestroyInstance** calls. To fix this we can create a seperate **VkDebugUtilsMessengerEXT** object specifically for those two function calls.

# Physical devices
The next step after the Vulkan library has been initalized through a **VkInstance**, is to look for and select graphics cards (we can select multiple).  

The graphics card that we’ll end up selecting will be stored in a **VkPhysicalDevice** handle. The grafics cards are queried through the **vkEnumeratePhysicalDevices** function that enumerates the physical devices accessible to a Vulkan instance.

If we have a **VkPhysicalDevice** handle corresponding to a available graphics card, we can use different functions to query different information about the grahics card.

- **vkGetPhysicalDeviceProperties** Get basic device properties like the name, type and supported Vulkan version
- **vkGetPhysicalDeviceFeatures** Get the support for optional features like texture compression, 64 bit floats and
multi viewport rendering (useful for VR)
- **vkGetPhysicalDeviceQueueFamilyProperties** Get what queuefamilies the graphics card supports

# Logical devices
After we have picked a physical device we need to setup a logical device to interface with it. We need to specify some properties for our logical device and what device features and QueueFamilies we're gonna use.

