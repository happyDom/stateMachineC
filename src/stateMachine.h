#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#include <stdint.h>

/* 请认真阅读以下关于 userSMCfg.h 文件的使用说明
 * 1、 状态机内存池管理
 * 用户需要定义下面的宏变量，来管理状态机使用的内存池，其值为内存池的byte数量， 此处建议设置一个比较大的数字，待项目定形后，再调整到合适的大小
#define DMEM_BUFFER_SIZE          1024

 * 2、 状态机和状态的buffer类型（由于C51架构单片机的内存架构和指针模型比较特殊，所以buffer内不再支持指针（因为难以预料用户要指向data/idata/xdata/pdata中的哪里））
 * 如果用户需要在状态机，或者各个状态中加添buffer，用于在各状态之间传递数据，则可能根据如下说明定义状态机/状态的buffer类型：
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

 * 3、 状态机存放位置
 * 在51类单片机中，由于其可用的data空间较小，所以状态机存放于xdata存储区
 * 💣 注意！注意！注意！💣：在你使用keil进行编译时，请务必将 Options->C51->Don’t use absolute register accesses 这一项打勾，这是为了避免将xData的低位地址误解析为寄存器地址，如下：
 * ✅ Don’t use absolute register accesses

 * 另外，建议你在定义状态机实例时，进行初始化操作，例如：
 stateMachine_t myFSM = {0};
 * 
*/
#include "userSMCfg.h"

typedef enum{
	false=0,
	true=!false
} _dyyBool;

#ifndef bool
#define bool _dyyBool
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define IS_NULL(p) (NULL == (p))
#define IS_pSafe(p) (NULL != (p))

typedef enum{
	aWait=0,
	go=1,
}smEventResult_t;

#if defined(SM_BUFFER_FULL) || defined(ST_BUFFER_FULL)
struct buffer_s {
	bool b;
	unsigned char ucAry[8];
	signed char sc;
    unsigned char uc;
	short s;
	unsigned short us;
    int i;
    unsigned int ui;
	long l;
	unsigned long ul;
    float f;
};
#elif defined(SM_BUFFER_PART) || defined(ST_BUFFER_PART)
struct buffer_s {
	union {
		bool b;
		signed char sc;
		unsigned char uc;
		short s;
		unsigned short us;
		unsigned char raw_16[2];
	}d16;
	
	union {
		int i;
    	unsigned int ui;
		long l;
		unsigned long ul;
		float f;
		unsigned char raw_32[4];
	}d32;
};
#elif defined(SM_BUFFER_TINY) || defined(ST_BUFFER_TINY)
struct buffer_s {
	union {
		bool b;
		signed char sc;
		unsigned char uc;
		short s;
		unsigned short us;
		unsigned char raw_16[2];
	}d16;
};
#endif

#if defined(SM_BUFFER_FULL) || defined(SM_BUFFER_PART) || defined(SM_BUFFER_TINY) || defined(ST_BUFFER_FULL) || defined(ST_BUFFER_PART) || defined(ST_BUFFER_TINY)
typedef struct buffer_s buffer_t;
#endif

struct stateMachine_event_s;
typedef struct stateMachineUnit_s smUnit_t;
typedef struct stateMachine_s xdata stateMachine_t;

typedef void (*smActionFunc_t)(smUnit_t xdata *);
typedef smEventResult_t (*smEventFunc_t)(smUnit_t xdata *);

struct stateMachine_actionMap_s
{
	smActionFunc_t pEnterAction;
	smActionFunc_t pDoAction;
	smActionFunc_t pExistAction;
};

struct stateMachine_event_s
{
	uint8_t nextState;
	smEventFunc_t pEventForGoing;
	struct stateMachine_event_s xdata *nextEvent;				//下一个事件
}; //这是一个单向链表,用于登记多个事件

struct stateMachineUnit_s
{
	bool latched;								//状态锁，为真时，状态机进行该状态的轮询时，不会检测该状态注册的事件
	uint8_t stateID_l;							//状态机的前一个状态
	uint8_t stateID;							//当前状态循环的状态
	struct stateMachine_actionMap_s actions;	//在本状态时需要执行的动作
	struct stateMachine_event_s *events;
	stateMachine_t xdata *pSm;
	uint16_t roundCounter;						// 如果1ms为周期计数，可记 65s

	//一个通用的buffer，用于存放与实际实用场景相关的数据
	#if defined(ST_BUFFER_FULL) || defined(ST_BUFFER_PART) || defined(ST_BUFFER_TINY)
	buffer_t buffer;
	#endif
};

struct stateMachine_s
{
	bool latched;								// 状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
	smActionFunc_t actionOnChangeBeforeEnter; 	// 状态切换前要做的动作, 参数是即将要切换的目标状态
	smActionFunc_t actionAfterDo;				// 在每个状态的do事件完成后，要执行的动作

	smUnit_t xdata *pSMChain;
	uint16_t roundCounter;
	uint8_t stateID;				//标记当前状态机的状态
	uint8_t stateID_default;		//状态机的默认状态
	uint8_t stateIDs_Count;			//状态机的总状态数

	// 定义一个buffer，用于存放与实际实用场景相关的数据
	#if defined(SM_BUFFER_FULL) || defined(SM_BUFFER_PART) || defined(SM_BUFFER_TINY)
	buffer_t buffer;
	#endif

	// 报警处理函数，如果状态机遇到异常，可以通过该函数进行报警
	void (*warningOn)(void);
};

// 如果在 C51 单片机上，内存模式使用 LargeMode： data in xData，则以上定义做如下调整（指针参数加xdata说明）
void fsm_init(stateMachine_t xdata *pSm, uint8_t stateIDs_count, uint8_t stateID_default, void (*warningFunc)(void));
void fsm_eventSignUp(stateMachine_t xdata *pSm, uint8_t stateID, uint8_t nextState, smEventFunc_t pEventForGoing);
void fsm_actionSignUp(stateMachine_t xdata *pSm, uint8_t stateID, smActionFunc_t pEnter, smActionFunc_t pDo, smActionFunc_t pExist);
void fsm_reset(stateMachine_t xdata *pSm);
void fsm_run(stateMachine_t xdata *pSm);
#endif
