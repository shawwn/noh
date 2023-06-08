#parse("C File Header.h")

#parse("Headers.h")
#[[#include]]# "#evaluate($dirName)_common.h"

#[[#include]]# "${NAME}.h"

#undef ${SINGLETON_NAME}
#undef p${SINGLETON_NAME}
#parse("hr.h")

#parse("Globals.h")
${SINGLETON_CLASS}  *p${SINGLETON_NAME}(${SINGLETON_CLASS}::GetInstance());

SINGLETON_INIT(${SINGLETON_CLASS})
#parse("hr.h")


/*====================
  ${SINGLETON_CLASS}::~${SINGLETON_CLASS}
  ====================*/
${SINGLETON_CLASS}::~${SINGLETON_CLASS}()
{
}


/*====================
  ${SINGLETON_CLASS}::${SINGLETON_CLASS}
  ====================*/
${SINGLETON_CLASS}::${SINGLETON_CLASS}()
{
}