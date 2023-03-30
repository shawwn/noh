#ifndef __BLOCK_T__
#define __BLOCK_T__

typedef struct block_s
{
    char name[5];
    size_t pos;     //position in buffer
    unsigned int length;
    byte *data;     //data pointer
}
block_t;

typedef struct blockList_s
{
    int num_blocks;
    int _num_allocated;

    block_t *blocks;
}
blockList_t;

#endif // __BLOCK_T__
