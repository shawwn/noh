#parse("C File Header.h")
#parse("Include Guard.h")

#parse("Headers.h")
#parse("hr.h")

#parse("Declarations.h")
#parse("hr.h")

#parse("Definitions.h")
#[[#define]]# ${SINGLETON_NAME} (*${SINGLETON_CLASS}::GetInstance())
#parse("hr.h")

#parse("hr.h")
// ${SINGLETON_CLASS}
#parse("hr.h")
class ${SINGLETON_CLASS}
{
    SINGLETON_DEF(${SINGLETON_CLASS})

private:

public:
    ~${SINGLETON_CLASS}();
};
#parse("hr.h")

#parse("Include Guard Close.h")