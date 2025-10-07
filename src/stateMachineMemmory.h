#ifndef Ed45cea82_814e_cc34_5397_4dae3d57fe8b
#define Ed45cea82_814e_cc34_5397_4dae3d57fe8b
#define dyMM__DEBUG

#include "stdint.h"

//用户使用
typedef struct
{
    void    *addr;      //申请到的内存的起始地址
    uint16_t size;      //申请到的内存的大小，按照块大小分配，大于等于申请大小
    uint16_t  tb;       //申请表序号，申请内存时分配，释放内存时使用，用户不使用
}DMEM;
 
//若返回空，则申请失败
DMEM *DynMemGet(uint16_t size);
#endif //Ed45cea82_814e_cc34_5397_4dae3d57fe8b
