#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define IS_NULL(p) (NULL == p)

typedef enum
{   
    //请将你的状态依次写到这里,请保证状态值从0开始且是连续的
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    
    stateID_end,    //
} stateMachine_stateID_t;

typedef enum
{
    aWait=0,
    go=1,
} stateMachine_eventResult_t;

typedef void (* stateAction)(void *);
typedef stateMachine_eventResult_t (*stateEvent)(void *);

typedef struct
{
    stateAction pEnterAction;
    stateAction pDoAction;
    stateAction pExistAction;
} stateMachine_actionMap_t;

typedef struct stateMachine_event_s
{
    stateEvent pEventForGoing;
    stateMachine_stateID_t nextState;           //目标状态
    struct stateMachine_event_s *nextEvent;     //下一个事件
} stateMachine_event_t;                         //这是一个单向链表,用于登记多个事件

typedef struct
{
    stateMachine_stateID_t stateID_l;   //状态机的前一个状态
    stateMachine_stateID_t stateID;     //当前状态循环的状态
    stateMachine_actionMap_t actions;   //在本状态时需要执行的动作
    stateMachine_event_t *events;       //在本状态时，需要进行关注的事件，这是一个数组地址
    unsigned int roundCounter;          //这个计数器显示了在本状态期间，状态机轮询的次数
} stateMachine_t;

static stateMachine_t *pStateMachine;    //存放状态单元的数组空间的地址，数组地址单元数量不小于状态数量

//初始化状态表
void fsm_init();
//向指定的状态注册事件
void fsm_eventSingUp(stateMachine_stateID_t state, stateEvent pEvent, stateMachine_stateID_t nextState);
void fsm_actionSignUp(stateMachine_stateID_t state, stateAction pEnter, stateAction pDo, stateAction pExist);
//运行一次状态机
void fsm_run();

#endif /* C0FD9D79_317D_44BD_BF7F_E51B5C4F850C */
