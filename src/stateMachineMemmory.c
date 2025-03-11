#include "stateMachineMemmory.h"
#include <stdio.h>

/**
 * 这里会预告在stack上申请一块指定大小的内存空间，用于后续应用层的动态申请，而不占用Heap空间，你可以根据实际情况合适调整 stack和heap的大小
 */

#define DMEM_BLOCK_SIZE         4      	//内存块大小(x字节)，这取决于实际申请内存时的最小公约数值
#define DMEM_BLOCK_NUM          128     //内存块个数，可以根据自己的实际状态数量和事件数量，调整到合适的大小
#define DMEM_TOTAL_SIZE         (DMEM_BLOCK_SIZE * DMEM_BLOCK_NUM)    //内存总大小

typedef enum
{
    DMEM_FREE   = 0,
    DMEM_USED   = 1,
}DMEM_USED_ITEM;

typedef struct
{
    DMEM_USED_ITEM   used;       //使用状态
    uint16_t         blk_s;      //起始块序号
    uint16_t         blk_num;    //块个数
}DMEM_APPLY;
 
typedef struct
{
    DMEM_USED_ITEM  tb_blk[DMEM_BLOCK_NUM];
    DMEM            tb_user[DMEM_BLOCK_NUM];        //用户申请内存信息
    DMEM_APPLY      tb_apply[DMEM_BLOCK_NUM];       //系统分配内存信息
    uint16_t        apply_num;                      //内存申请表占用数目
    uint16_t        blk_num;                        //内存块占用数目
}DMEM_STATE;
 
static uint8_t DMEMORY[DMEM_TOTAL_SIZE];
static DMEM_STATE DMEMS;
#ifdef dyMM__DEBUG
static uint16_t __reservedBlock_num_min = DMEM_BLOCK_NUM;          //剩余内存快数量的最小值
#endif

DMEM *DynMemGet(uint32_t size)
{
    uint16_t loop = 0;
    uint16_t find = 0;
    uint16_t blk_num_want = 0;
    uint16_t size16 = (uint16_t)size;
    DMEM *user = NULL;
    DMEM_APPLY *apply = NULL;
    
    //申请内存大小不能为0
    if(size16 == 0)               {   return NULL;    }
    //申请内存不可超过总内存大小
    if(size16 > DMEM_TOTAL_SIZE)  {   return NULL;    }
    //申请内存不可超过剩余内存大小
    if(size16 > (DMEM_BLOCK_NUM - DMEMS.blk_num) * DMEM_BLOCK_SIZE)   {   return NULL;    }
    //申请表必须有空余
    if(DMEMS.apply_num >= DMEM_BLOCK_NUM)   {   return NULL;    }
    
    //计算所需连续块的个数
    blk_num_want = (size16 + DMEM_BLOCK_SIZE - 1) / DMEM_BLOCK_SIZE;
    
    //寻找申请表
    for(loop = 0; loop < DMEM_BLOCK_NUM; loop++)
    {
        if(DMEMS.tb_apply[loop].used == DMEM_FREE)
        {
            apply = &DMEMS.tb_apply[loop];                  //申请表已找到
            user = &DMEMS.tb_user[loop];                    //用户表对应找到
            user->tb = loop;                                //申请表编号记录
            user->size = blk_num_want * DMEM_BLOCK_SIZE;    //分配大小计算
            break;
        }
    }
    
    //没有找到可用申请表，理论上是不会出现此现象的，申请表剩余已在上面校验
    if(loop == DMEM_BLOCK_NUM)  {   return NULL;    }
    
    //寻找连续内存块
    for(loop = 0; loop < DMEM_BLOCK_NUM; loop++)
    {
        if(DMEMS.tb_blk[loop] == DMEM_FREE)
        {//找到第一个空闲内存块
            for(find = 1; (find < blk_num_want) && (loop + find < DMEM_BLOCK_NUM); find ++)
            {//找到下一个空闲内存块
                if(DMEMS.tb_blk[loop + find] != DMEM_FREE)
                {//发现已使用内存块
                    break;
                }
            }
            if(find >= blk_num_want)
            {//寻找到的空闲内存块数目已经够用
                user->addr = DMEMORY + loop * DMEM_BLOCK_SIZE;  //计算申请到的内存的地址
                apply->blk_s = loop;                            //记录申请到的内存块首序号
                apply->blk_num = blk_num_want;                  //记录申请到的内存块数目
                for(find = 0 ; find < apply->blk_num; find++)
                {
                    DMEMS.tb_blk[loop + find] = DMEM_USED;
                }
                apply->used = DMEM_USED;                        //标记申请表已使用
                DMEMS.apply_num += 1;
                DMEMS.blk_num += blk_num_want;

                #ifdef dyMM__DEBUG // 如果申请成功，计算剩余的内存块数量
                if(DMEM_BLOCK_NUM < __reservedBlock_num_min + apply->blk_s + apply->blk_num){
                    __reservedBlock_num_min = DMEM_BLOCK_NUM - apply->blk_s - apply->blk_num;
                }
                #endif
                
                return user;
            }
            else
            {//寻找到的空闲内存块不够用，从下一个开始找
                loop += find;
            }
        }
    }

    //搜索整个内存块，未找到大小适合的空间
    return NULL;
}

void DynMemFree(DMEM *user)
{
    uint16_t loop = 0;
    //若参数为空，直接返回
    if(NULL == user)    {   return; }
    
    //释放内存空间
    for(loop = DMEMS.tb_apply[user->tb].blk_s; loop < DMEMS.tb_apply[user->tb].blk_s + DMEMS.tb_apply[user->tb].blk_num; loop++)
    {
        DMEMS.tb_blk[loop] = DMEM_FREE;
        DMEMS.blk_num -= 1;
    }
    //释放申请表
    DMEMS.tb_apply[user->tb].used = DMEM_FREE;
    DMEMS.apply_num -= 1;
}

uint16_t getReservedBlock_num_min(void){
    #ifdef dyMM__DEBUG
    return __reservedBlock_num_min;
    #else
    return 0;
    #endif
}
