#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#include "limits.h"
#define IS_NULL(p) (NULL == p)
#define IS_pSafe(p) (NULL != p)

typedef enum
{
    aWait=0,
    go=1,
} stateMachine_eventResult_t;

typedef void (* stateAction)(void *);
typedef stateMachine_eventResult_t (*eventFunc)(void *);

typedef struct
{
    stateAction pEnterAction;
    stateAction pDoAction;
    stateAction pExistAction;
} stateMachine_actionMap_t;

typedef struct stateMachine_event_s
{
    eventFunc pEventForGoing;
    unsigned int nextState;                     //目标状态
    struct stateMachine_event_s *nextEvent;     //下一个事件
} stateMachine_event_t;                         //这是一个单向链表,用于登记多个事件

typedef struct
{
    unsigned int stateID_l;   //状态机的前一个状态
    unsigned int stateID;     //当前状态循环的状态
    stateMachine_actionMap_t actions;   //在本状态时需要执行的动作
    stateMachine_event_t *events;       //在本状态时，需要进行关注的事件，这是一个数组地址
    unsigned int roundCounter;          //这个计数器显示了在本状态期间，状态机轮询的次数
} stateMachineUnit_t;

typedef struct stateMachine_t_s
{
    stateMachineUnit_t *pSMChain;    //存放状态单元的数组空间的地址
    __int16 stateID;                //标记当前状态机的状态
    __int16 stateID_default;        //状态机的默认状态
    __int16 stateIDs_Count;         //状态机的总状态数
}stateMachine_t;

//初始化状态表
void fsm_init(stateMachine_t *pSm, int stateIDs_count, int stateID_default);
//复位状态机：将状态机的运行状态复位到默认状态
void fsm_reset(stateMachine_t *pSm);
//向指定的状态注册事件
void fsm_eventSingUp(stateMachine_t *pSm, unsigned int stateID, unsigned int nextState, eventFunc pEvent);
void fsm_actionSignUp(stateMachine_t *pSm, unsigned int stateID, stateAction pEnter, stateAction pDo, stateAction pExist);

//运行一次指定的状态机
void fsm_run(stateMachine_t *pSm);

#endif /* C0FD9D79_317D_44BD_BF7F_E51B5C4F850C */
