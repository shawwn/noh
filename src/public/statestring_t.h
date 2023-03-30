#ifndef __STATESTRING_T__
#define __STATESTRING_T__

typedef struct stateString_s
{
	char	*string;
	int		modifyCount;
	size_t	length;			//so we don't need to keep recomputing it during sending
	size_t	memsize;		//memory size allocated
	int		lastModifiedFrame;
	int		flags;
}
stateString_t;

#endif // __STATESTRING_T__