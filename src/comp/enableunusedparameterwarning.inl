#if defined(Q_OS_WIN)
    // Do nothing...
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    #pragma GCC diagnostic error "-Wunused-parameter"
#else
    #error Unsupported platform
#endif