#parse("C File Header.h")
#parse("Include Guard.h")

#parse("Headers.h")
#parse("hr.h")
${NAMESPACES_OPEN}
#parse("Declarations.h")
#parse("hr.h")

#parse("Definitions.h")
#parse("hr.h")

#parse("hr.h")
// ${NAME}
#parse("hr.h")
class ${NAME}
{
public
    ~${NAME}();:
    ${NAME}();
};
#parse("hr.h")
${NAMESPACES_CLOSE}
#parse("Include Guard Close.h")
