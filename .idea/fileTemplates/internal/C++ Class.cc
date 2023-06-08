#parse("C File Header.h")

#parse("Headers.h")
#[[#include]]# "#evaluate($dirName)_common.h"

#[[#include]]# "${HEADER_FILENAME}"
#parse("hr.h")

#parse("Declarations.h")
#parse("hr.h")

#parse("Globals.h")
#parse("hr.h")
${NAMESPACES_OPEN_CPP}
/*====================
  ${NAME}::~${NAME}
  ====================*/
${NAME}::~${NAME}()
{
}


/*====================
  ${NAME}::${NAME}
  ====================*/
${NAME}::${NAME}()
{
}
${NAMESPACES_CLOSE_CPP}