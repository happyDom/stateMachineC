#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * 你需要创建并完成一个 userSMCfg.h 文档，在该文档中根据需要，应完成以下内容的定义

 * 1、 状态机运行周期
 * 由于状态机的运行周期，决定了状态机中 roundCounter 计数器的单位时间
 * 例如：如果状态机的运行周期为 1ms，则 roundCounter 的单位时间为 1ms
 * 如果状态机的运行周期为 10ms，则 roundCounter 的单位时间为 10ms
 * 你需要根据你的项目实际情况，定义下面的宏变量
 #define SM_CYCLE_TIME_MS         5    //状态机运行周期，单位为毫秒

 * 2、配置状态机层和状态层的数据buffer，如果你需要在不同的状态之间传递数据，这是个不错的选择
 *
 * SM_BUFFER_NO		//状态机层面不定义buffer
 * SM_BUFFER_FULL	//状态机层面定义全量buffer
 * SM_BUFFER_PART	//状态机层面定义部分buffer
 * SM_BUFFER_TINY	//状态机层面定义最小buffer
#define SM_BUFFER_NO

 * ST_BUFFER_NO		//状态层面不定义buffer
 * ST_BUFFER_FULL	//状态层面定义全量buffer
 * ST_BUFFER_PART	//状态层面定义部分buffer
 * ST_BUFFER_TINY	//状态层面定义最小buffer
#define ST_BUFFER_NO

 * 3、定义变量 DMEM_BUFFER_SIZE 用于管理状态机使用的内存，在项目定形后，通过观察bufferUsed的值，适当的减小该变量的值
#define DMEM_BUFFER_SIZE          512

 * 提示： 声明状态机对象时，可以直接将对象初始化为0，例如：
stateMachine_t myFSM = {0};
*/
#include "userSMCfg.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

#define IS_NULL(p) (NULL == (p))
#define IS_pSafe(p) (NULL != (p))

#ifndef DMEM_BUFFER_SIZE
#error "please #define DMEM_BUFFER_SIZE //in userSMCfg.h"
#elif DMEM_BUFFER_SIZE <= 0
#error "DMEM_BUFFER_SIZE must be greater than 0!"
#endif

#if ((defined(SM_BUFFER_FULL) ? 1 : 0) + (defined(SM_BUFFER_PART) ? 1 : 0) + (defined(SM_BUFFER_TINY) ? 1 : 0) > 1)
#error "Only one of SM_BUFFER_FULL / SM_BUFFER_PART / SM_BUFFER_TINY can be defined"
#endif

#ifndef SM_CYCLE_TIME_MS
#error "please #define SM_CYCLE_TIME_MS //in userSMCfg.h"
#elif SM_CYCLE_TIME_MS <= 0
#error "SM_CYCLE_TIME_MS must be greater than 0!"
#endif

#if ((defined(ST_BUFFER_FULL) ? 1 : 0) + (defined(ST_BUFFER_PART) ? 1 : 0) + (defined(ST_BUFFER_TINY) ? 1 : 0) > 1)
#error "Only one of ST_BUFFER_FULL / ST_BUFFER_PART / ST_BUFFER_TINY can be defined"
#endif

#ifndef CNTSOFms
#define CNTSOFms(ms) ((uint32_t)((ms) / (SM_CYCLE_TIME_MS)))  // 计算多少个周期数为 ms 毫秒， T 为周期时间，单位为毫秒
#endif

#ifndef CNTSOFs
#define CNTSOFs(s) ((uint32_t)((s) * 1000.0f / (SM_CYCLE_TIME_MS)))  // 计算多少个周期数为 s 秒， T 为周期时间，单位为毫秒
#endif

typedef enum {
    aWait = 0,
    go = 1,
} smEventResult_t;

#if defined(SM_BUFFER_FULL)
typedef struct {
    bool b;
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    float f32;

    void* ptr;
} smBuffer_t;
#elif defined(SM_BUFFER_PART)
typedef struct {
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
    } d8;
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;

        bool bAry[2];
        int8_t i8Ary[2];
        uint8_t u8Ary[2];
    } d16;

    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;
        int32_t i32;
        uint32_t u32;
        float f32;

        bool bAry[4];
        int8_t i8Ary[4];
        uint8_t u8Ary[4];
        int16_t i16Ary[2];
        uint16_t u16Ary[2];
    } d32;

    void* ptr;
} smBuffer_t;
#elif defined(SM_BUFFER_TINY)
typedef struct {
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;

        bool bAry[2];
        int8_t i8Ary[2];
        uint8_t u8Ary[2];
    } d16;
} smBuffer_t;
#endif

#if defined(ST_BUFFER_FULL)
typedef struct {
    bool b;
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    float f32;

    void* ptr;
} stBuffer_t;
#elif defined(ST_BUFFER_PART)
typedef struct {
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
    } d8;
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;

        bool bAry[2];
        int8_t i8Ary[2];
        uint8_t u8Ary[2];
    } d16;

    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;
        int32_t i32;
        uint32_t u32;
        float f32;

        bool bAry[4];
        int8_t i8Ary[4];
        uint8_t u8Ary[4];
        int16_t i16Ary[2];
        uint16_t u16Ary[2];
    } d32;

    void* ptr;
} stBuffer_t;
#elif defined(ST_BUFFER_TINY)
typedef struct {
    union {
        bool b;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;

        bool bAry[2];
        int8_t i8Ary[2];
        uint8_t u8Ary[2];
    } d16;
} stBuffer_t;
#endif

struct stateMachine_event_s;
typedef struct stateMachineUnit_s smUnit_t;
typedef struct stateMachine_s stateMachine_t;

typedef void (*smActionFunc_t)(smUnit_t*);
typedef smEventResult_t (*smEventFunc_t)(smUnit_t*);

struct stateMachine_actionMap_s {
    smActionFunc_t pEnterAction;
    smActionFunc_t pDoAction;
    smActionFunc_t pExitAction;
};

struct stateMachine_event_s {
    smEventFunc_t pEventForGoing;
    uint8_t nextState;                       // 目标状态
    struct stateMachine_event_s* nextEvent;  // 下一个事件
};  // 这是一个单向链表,用于登记多个事件

struct stateMachineUnit_s {
    bool latched;                             // 状态锁，为真时，状态机进行该状态的轮询时，不会检测该状态注册的事件
    uint8_t stateID_l;                        // 状态机的前一个状态
    uint8_t stateID;                          // 当前状态循环的状态
    struct stateMachine_actionMap_s actions;  // 在本状态时需要执行的动作
    struct stateMachine_event_s* events;      // 在本状态时，需要进行关注的事件，这是事件链表的表头地址
    uint32_t roundCounter;                    // 这个计数器显示了在本状态期间，状态机轮询的次数，如果 1ms 轮询一次，支持最大 49.7 天时间的计数
    stateMachine_t* pSm;                      // 状态机的指针，这使得状态单元可以使用状态机中的信息

// 一个通用的buffer，用于存放与实际实用场景相关的数据
#if defined(ST_BUFFER_FULL) || defined(ST_BUFFER_PART) || defined(ST_BUFFER_TINY)
    stBuffer_t buffer;
#endif
};

struct stateMachine_s {
    bool latched;                              // 状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
    smUnit_t* pSMChain;                        // 存放状态单元的数组空间的地址
    smActionFunc_t actionOnChangeBeforeEnter;  // 状态切换前要做的动作，参数是即将要切换到的目标状态实例
    smActionFunc_t actionAfterDo;              // 在每个do事件后执行的动作
    uint8_t stateID;                           // 标记当前状态机的状态
    uint8_t stateID_default;                   // 状态机的默认状态
    uint8_t stateIDs_Count;                    // 状态机的总状态数
    uint32_t* enterCounterOf;                  // 一个数组，用于记录状态机中每一个状态出现的次数，在对应状态进入时进行计数
    uint32_t roundCounter;                     // 记录状态机的轮询次数

// 定义一个buffer，用于存放与实际实用场景相关的数据
#if defined(SM_BUFFER_FULL) || defined(SM_BUFFER_PART) || defined(SM_BUFFER_TINY)
    smBuffer_t buffer;
#endif

    // 报警处理函数，如果状态机遇到异常，可以通过该函数进行报警
    void (*warningOn)(void);
};

// 初始化状态表
void fsm_init(stateMachine_t* pSm, uint8_t stateIDs_count, uint8_t stateID_default, void (*warningFunc)(void));
// 注册跳转事件/条件
void fsm_eventSignUp(stateMachine_t* pSm, uint8_t stateID, uint8_t nextState, smEventFunc_t pEventForGoing);
// 注册行为动作
void fsm_actionSignUp(stateMachine_t* pSm, uint8_t stateID, smActionFunc_t pEnter, smActionFunc_t pDo, smActionFunc_t pExit);
// 复位状态机：将状态机的运行状态复位到默认状态
void fsm_reset(stateMachine_t* pSm);
// 运行一次指定的状态机
void fsm_run(stateMachine_t* pSm);
#endif
