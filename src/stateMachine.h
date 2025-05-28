#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#include <stdint.h>

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

#define IS_NULL(p) (NULL == p)
#define IS_pSafe(p) (NULL != p)

typedef enum{
	aWait=0,
	go=1,
}smEventResult_t;

typedef union {
	bool b;
	unsigned char bt[8];
	signed char i8;
    unsigned char ui8;
	short s16;
	unsigned short us16;
    int i32;
    unsigned int ui32;
    long long l64;
    unsigned long long ul64;
    float f32;
    double d64;
	void *ptr;
} bufferUnion;

struct stateMachine_event_s;
typedef struct stateMachineUnit_s smUnit_t;
typedef struct stateMachine_s stateMachine_t;

struct stateMachine_actionMap_s
{
	void (*pEnterAction)(smUnit_t *);
	void (*pDoAction)(smUnit_t *);
	void (*pExistAction)(smUnit_t *);
};

struct stateMachine_event_s
{
	smEventResult_t (*pEventForGoing)(smUnit_t *);
	uint8_t nextState;							//目标状态
	struct stateMachine_event_s *nextEvent;			//下一个事件
};												//这是一个单向链表,用于登记多个事件

struct stateMachineUnit_s
{
	bool latched;							//状态锁，为真时，状态机进行该状态的轮询时，不会检测该状态注册的事件
	uint8_t stateID_l;						//状态机的前一个状态
	uint8_t stateID;						//当前状态循环的状态
	struct stateMachine_actionMap_s actions;		//在本状态时需要执行的动作
	struct stateMachine_event_s *events;			//在本状态时，需要进行关注的事件，这是一个数组地址
	uint32_t roundCounter;					//这个计数器显示了在本状态期间，状态机轮询的次数，如果 1ms 轮询一次，支持最大 49.7 天时间的计数
	bufferUnion buffer;						//一个通用的buffer，用于存放与实际实用场景相关的数据
	stateMachine_t *pSm;					//状态机的指针，这使得状态单元可以使用状态机中的信息
};

struct stateMachine_s
{
	bool latched;					//状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
	uint32_t *enterCounterOf;		//一个数组，用于记录状态机中每一个状态出现的次数，在对应状态退出时进行计数
	smUnit_t *pSMChain;	//存放状态单元的数组空间的地址
	void (*actionOnChangeBeforeEnter)(smUnit_t *pstNew); //状态切换前要做的动作
	void (*actionAfterDo)(smUnit_t *); //在每个do事件后执行的动作
	uint8_t stateID;				//标记当前状态机的状态
	uint8_t stateID_default;		//状态机的默认状态
	uint8_t stateIDs_Count;			//状态机的总状态数
	uint32_t roundCounter;			//记录状态机的轮询次数
	bufferUnion buffer;				//一个通用的buffer，用于存放与实际实用场景相关的数据

	//复位状态机：将状态机的运行状态复位到默认状态
	void (*reset)(stateMachine_t *pSm);
	//向指定的状态注册事件
	void (*eventSingUp)(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, smEventResult_t (*pEventForGoing)(smUnit_t *));
	void (*actionSignUp)(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(smUnit_t *), void (*pDo)(smUnit_t *), void (*pExist)(smUnit_t *));

	//运行一次指定的状态机
	void (*run)(stateMachine_t *pSm);
};

//初始化状态表
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default);

//提供一个接口，用于获取内存池中剩余内存块的最小值，为合理优化内存池大小做为参考
uint16_t dyMM_blocksNumOfUsed(void);

#endif
