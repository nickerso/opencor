#ifdef INTERFACE_DEFINITION
    #define PURE = 0
#else
    #define PURE
#endif

    virtual void handleArguments(const QStringList &pArguments) PURE;

#undef PURE
