# CHANGELOG

- Wed Jan 10 21:27:39 +03 2024

  Okay so the steps are as below:

  - create a window
    - init glfw
    - check for vulkan support
    - disable opengl context creation
  - create a vulkan instance
  - create surface
  - get queue handles
    - create logical device
      - pick a physical device
    - find queue families
  - create swapchain
  - create images
  - create image views
  - create graphics pipeline
    - create shader modules
    - create a renderpass
    - create pipeline layout
  - create framebuffers
  - create the commandpool
  - create a commandbuffer
  - create sync objects
  - main loop
    - wait for previous frame
    - get the next image
    - record commands to a commandbuffer
    - submit the commandbuffer
    - release back the image for presentation
  - cleanup

  That is too many fucking steps.
  And I still leak memories left and right, UUGHHHH...



- Wed Jan 10 11:47:31 +03 2024

  We will use GLFW for window and event management.  I do not care
  other libraries, this one works an is great.  Quite simple too.

  > NOTICE: As of the 1.3.216 SDK release,  the Vulkan Portability
    Enumeration extension is being enforced by the Vulkan loader.
    Failure to 'opt in' for non-conformant Vulkan implementations
    will cause applications to fail to find the MoltenVK ICD.  See
    the macOS release notes for more information.
