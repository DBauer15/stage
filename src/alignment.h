#if defined(STAGE_API_USAGE_OPENGL)
#define DEVICE_ALIGNED alignas(16)
#elif  defined(STAGE_API_USAGE_METAL)
#define DEVICE_ALIGNED
#else
#define DEVICE_ALIGNED
#endif