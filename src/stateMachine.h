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

/* =====================================================================
 * 【踩坑警示：Keil C251 的 Data Overlay（数据覆盖）会踩掉本框架的局部变量】
 *
 * 一句话判据：状态机里某个计数器/指针“不增长、不更新”，而把某个回调
 *   （动作 / 事件 / 后处理）的调用注释掉就恢复正常 —— 十有八九是 overlay，
 *   不是业务逻辑错。此时先去看 map，不要先去改业务代码。
 *
 * 为什么会踩：
 *   本框架靠函数指针驱动状态，下面这些调用边链接器是看不见的：
 *       pSm->pSMChain[i].actions.pEnterAction / pDoAction / pExitAction
 *       pSm->actionAfterDo
 *       event->pEventForGoing
 *   链接器只按“直接调用”的调用树决定哪些函数的局部变量可共用同一块
 *   overlay 区（map 中的 _EDATA_GROUP_ 段）。看不见的边 = 它认为两者不会
 *   同时运行，于是把 fsm_run() 的局部变量（p / st / stNew）与“任一被注册
 *   进回调链的函数（含该函数调用的所有函数）”的局部变量、参数块(?BYTE)
 *   分到同一组地址。回调里写一次参数，就把 st 之类覆盖掉了。
 *
 * 踩中后的样子（从 C 代码逻辑上完全挑不出错）：
 *   - roundCounter 卡在某个值 → 依赖它的周期动作、周期刷新全部停摆；
 *   - 回调返回后 st / stNew “变味”，后续 st->xxx 的读写落在野地址；
 *   - 结构体某字段莫名变化、偶发死机、关掉某个调用就“好了”。
 *
 * 三步排查：
 *   1) 看 map：存在 _EDATA_GROUP_ 段 = overlay 开着；
 *   2) 在该段后面的 OVERLAY MAP OF MODULE 里，核对 fsm_run() 的 st / stNew
 *      的地址是否与被怀疑的函数（及其调用链）的局部变量同址 —— 同址即命中，
 *      不必再往别处查；
 *   3) 处理二选一：整体关闭数据覆盖（链接器选项里禁用，例如 L251 Misc
 *      controls 填 NOOVERLAY），或只让回调函数单独 NOOVERLAY、用
 *      OVERLAY(父 ! 子) 把链接器不知道的调用关系显式补上。
 *      关闭后 map 里每个函数应各自拥有独立的 ?ED?<函数>?<模块> 段。
 *
 * 取舍：局部变量区宽裕、或隐形边很多（函数指针 / 中断 / 汇编 / 跨模块
 *   回调）时，优先整体关闭；RAM 真紧张才逐函数声明，那样必须维护一份
 *   “谁通过指针调用谁”的清单，漏一条照样会踩。
 *
 * 易忽略：
 *   - 分配结果与优化等级、模块链接顺序相关，改几行代码症状就会变形，
 *     别用“改代码试探”来定位，直接看 map 最快；
 *   - 中断服务函数及其调用链同样是隐形边（中断随时打断主程序），不要与
 *     主程序共用局部变量区；
 *   - 只影响“局部变量与参数块”，全局/静态变量不受影响（本框架
 *     DMEM_BUFFER_SIZE 申请的那块静态池、状态机对象本身都是安全的）。
 * ===================================================================== */
/* 空指针统一写裸 0；本模块既不提供 NULL 的定义，也不使用 NULL：
 *   - 库头里 #define NULL 会把定义扩散到所有包含它的文件，还会与 <stddef.h>
 *     等标准头的同名定义互相干扰（谁先被包含就用谁的，属隐式依赖）；
 *   - 0 本身就是合法的空指针常量，比较、赋值都成立，去掉 NULL 不损失任何表达力；
 *   - 与工程内其它通用模块（flashCtrl、linearInterpolation）保持同一口径。
 * 注意：给更窄的指针置空时不要写 (void*)0 —— 那是 3 字节通用指针，赋给
 *       1~2 字节的指针（尤其 code 函数指针）会触发 C251 的 C151
 *       "pointer truncation" 告警；写裸 0 才干净。
 *       用 0 做"比较"则无论指针多宽都安全。
 * IS_NULL / IS_pSafe 只是"与 0 比较"的命名，宏体里没有任何 NULL 依赖，故保留。 */
#define IS_NULL(p) ((0) == (p))
#define IS_pSafe(p) ((0) != (p))

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
    uint32_t roundCounter;                    // 这个计数器显示了在本状态期间，状态机已经完成的轮询次数，如果 1ms 轮询一次，支持最大 49.7 天时间的计数
    stateMachine_t* pSm;                      // 状态机的指针，这使得状态单元可以使用状态机中的信息

// 一个通用的buffer，用于存放与实际实用场景相关的数据
#if defined(ST_BUFFER_FULL) || defined(ST_BUFFER_PART) || defined(ST_BUFFER_TINY)
    stBuffer_t buffer;
#endif
};

struct stateMachine_s {
    bool latched;                              // 状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
    smUnit_t* pSMChain;                        // 存放状态单元的数组空间的地址
    smActionFunc_t actionAfterDo;              // 在每个do事件后执行的动作
    uint8_t stateID;                           // 标记当前状态机的状态
    uint8_t stateID_default;                   // 状态机的默认状态
    uint8_t stateIDs_Count;                    // 状态机的总状态数
    uint32_t* enterCounterOf;                  // 一个数组，用于记录状态机中每一个状态出现的次数，在对应状态进入时进行计数
    uint32_t roundCounter;                     // 记录状态机的已经完成的轮询次数

// 定义一个buffer，用于存放与实际实用场景相关的数据
#if defined(SM_BUFFER_FULL) || defined(SM_BUFFER_PART) || defined(SM_BUFFER_TINY)
    smBuffer_t buffer;
#endif

    // 报警处理函数，如果状态机遇到异常，可以通过该函数进行报警
    void (*warningOn)(void);
};

// 初始化状态表
void fsm_init(stateMachine_t* pSm, uint8_t stateIDs_count, uint8_t stateID_default, void (*warningFunc)(void), smActionFunc_t actionAfterDo);
// 注册跳转事件/条件
void fsm_eventSignUp(stateMachine_t* pSm, uint8_t stateID, uint8_t nextState, smEventFunc_t pEventForGoing);
// 注册行为动作
void fsm_actionSignUp(stateMachine_t* pSm, uint8_t stateID, smActionFunc_t pEnter, smActionFunc_t pDo, smActionFunc_t pExit);
// 复位状态机：将状态机的运行状态复位到默认状态
void fsm_reset(stateMachine_t* pSm);
// 运行一次指定的状态机
void fsm_run(stateMachine_t* pSm);
#endif
