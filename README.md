# stateMachineC

## 介绍

基于C语言实现的状态机框架，基于此可以快捷的构建一个实用的状态机

## 目录结构

- `src/` —— 库本体，只需要把这两个文件加入你的工程：
  - `stateMachine.h` 状态机框架头文件
  - `stateMachine.c` 状态机框架实现
- `cfg/userSMCfg.h.sample` —— 用户配置样例，照抄到你自己的项目里并改名 `userSMCfg.h`
  （轮询周期、状态机层/状态层 buffer 配置、静态内存池大小，以及可选的 `SM_MEM` 存放区）；
- `example/` —— 参考示例，不参与库编译，也不需要加入你的工程：
  - `smDemo.h` / `smDemo.c` demo 状态机
  - `main.c` demo 的运行入口

## 安装教程

- 👉添加本仓库到您的项目中

```Bash
git submodule add git@gitee.com:DyyYq/stateMachineC.git submodule/stateMachineC
```

## 配置项（userSMCfg.h）

库本身不带默认配置，可调项都在你项目里的 `userSMCfg.h` 中定义（照着 `cfg/userSMCfg.h.sample` 抄）：

- `SM_CYCLE_TIME_MS` —— 一拍多少毫秒，必须与你调用 `fsm_run()` 的实际周期一致（`roundCounter`、`CNTSOFms` 都按它换算）；
- `SM_BUFFER_NO/FULL/PART/TINY` —— 状态机层的数据 buffer，四选一；
- `ST_BUFFER_NO/FULL/PART/TINY` —— 状态层的数据 buffer，四选一（RAM 占用 = 状态数 × sizeof(stBuffer_t)）；
- `DMEM_BUFFER_SIZE` —— 静态内存池大小（字节），`fsm_init` / `fsm_eventSignUp` 都从这块池里切内存；
- `SM_MEM`（可选）—— 指定状态机对象、状态单元、事件节点和内存池的存放区，例如 `#define SM_MEM xdata`；不定义则由编译器按内存模型处理。

启用 `SM_MEM` 时，应用侧只有两件事要做：

1. 声明对象时写上 `SM_MEM`：`stateMachine_t SM_MEM myFSM;`
   漏写不是语法错误，但会立刻报一串 `C150 'edata' pointer converted to 'xdata' pointer`，照着告警补上即可；
2. 动作/事件回调的参数统一用 `smUnitPtr_t`，不要写 `smUnit_t*`（后者是 edata 指针，与库里带 `SM_MEM` 的回调原型对不上，会报 `C98/C91`）。

另外两点（都是踩过的坑）：

- 对象落在 xdata 时**不要写初始化器**（`= {0}` 这类）：C251 不为 xdata 生成启动初始化代码，初始化器会被静默丢弃；`fsm_init` 本来就会把全部字段赋一遍，直接 `stateMachine_t SM_MEM myFSM;` 就行。
- 同理，库内那个统计池已用字节数的计数器刻意留在 `edata`（不挂 `SM_MEM`）：放到 xdata 时它的 `= 0` 同样会被丢弃，初值是上电随机值，`DynMemGet` 会直接判“池已满”，`fsm_init` 随后 `while(1)` 死等 —— 现象就是刷完黑屏、毫无反应。
- 不要试图把存储类折进 typedef（`typedef struct X SM_MEM T;`）来省掉 `SM_MEM`：C251 对这种写法处理不可靠，实测实例可能静默落回 edata 且不报任何告警。

## 使用说明

- 👉先在项目里照 `cfg/userSMCfg.h.sample` 建 `userSMCfg.h`，并把 `src/` 加入头文件搜索路径
- 👉构建你的状态机（假设你的状态机名称为smDemo）
  - 添加 *smDemo.h* 文档
  - 添加 *smDemo.c* 文档
- 👉在 *smDemo.h*中定义并管理你的状态机
  - 引入 *stateMachine.h* 文件，并定义enum类型以管理您的各个状态值

    ```C
    #include "stateMachine.h"

    //定义状态机的各个状态值
    typedef enum
    {
        //请将你的状态依次写到这里,请保证状态值从0开始，并且是连续的
        a = 0,
        b,
        c,
        d,
        e,
        f,
        g,
        
        // 这是状态结束标记，这个值表示了您在这之前定义的状态的数量
        stateID_end
    } demoState_t;
    ```

  - 声明状态机创建事务和运行事务的方法

    ```C
    //定义两个方法，用于执行状态的创建和运行事务
    void smDemoBuild(void);
    void smDemoRun(void);
    ```

  - 根据你的需要，声明一些其它的变量

    ```C
    //定义一个char变量，用于接收用户输入，并在状态机事件中观察处理此输入
    extern char inputKey;
    ```

- 👉在 *smDemo.C* 中实现你的状态机功能
  - 定义状态机变量，以及一些其它的必要的变量（根据你的需求来喽）
  
    ```C
    #include <stdio.h>
    #include "smDemo.h"

    char keys[7] = {'a','b','c','d','e','f','g'};

    char inputKey;
    stateMachine_t SM_MEM demoSM;
    ```

  - 实现状态机的进入事件，逗留事件和退出事件；你可以为每个状态分别实现不同的进入事件，逗留事件和退出事件，也可以共用它们

    ```C
    void actionEntry(smUnitPtr_t pSt)
    {
        if(pSt->pSm->roundCounter == 0){
            //状态机首次运行时
            printf("the current state is: %c\n", keys[pSt->stateID]);
        }else{
            //状态机切换进入某一状态时
            printf(" --> %c\n", keys[pSt->stateID]);
        }
    }

    void actionDo(smUnitPtr_t pSt)
    {
        //状态机的do事件
        printf("----roundCounter of %c is %lu\n", keys[pSt->stateID], (unsigned long)pSt->roundCounter);
    }

    void actionExit(smUnitPtr_t pSt)
    {
        //状态机的退出事件
        printf("state exchanged: %c", keys[pSt->stateID]);
    }
    ```

  - 实现状态机跳转事件，用于在每一次轮询状态机时观察是否发生了对应的事件，如果发生了（返回go），则状态跳转到对应的目标状态
  
    ```C
    smEventResult_t pressA(smUnitPtr_t pSt) {return 'a' == inputKey;};
    smEventResult_t pressB(smUnitPtr_t pSt) {return 'b' == inputKey;};
    smEventResult_t pressC(smUnitPtr_t pSt) {return 'c' == inputKey;};
    smEventResult_t pressD(smUnitPtr_t pSt) {return 'd' == inputKey;};
    ```

  - 实现 *smDemoBuild* 方法，初始化状态机，为每个状态注册对应的事件处理函数，以及为每个状态跳转路径注册对应的事件函数；以下的demo中实现了如下的一个状态机跳转关系：
    ![Snipaste_2025-03-22_22-07-39](https://s2.loli.net/2025/03/22/HFuwUeEj8OZ6cqW.png)

    ```C
    void smDemoBuild()
    {
        fsm_init(&demoSM, stateID_count, 0, 0, 0);   // 5 个参数：状态数、默认状态、报警回调、do 后处理回调
        
        // 注册状态动作
        // 为每一个状态指定进入事件，逗留事件和退出事件
        // 如果你的某些状态不需要全部的事件，则将不需要对应事件的位置传 0 即可
        fsm_actionSignUp(&demoSM, a, actionEntry, 0, actionExit);
        fsm_actionSignUp(&demoSM, b, 0, actionDo, actionExit);
        fsm_actionSignUp(&demoSM, c, actionEntry, actionDo, 0);
        fsm_actionSignUp(&demoSM, d, 0, actionDo, 0);
        
        // 注册状态事件
        // 为每个状态跳转路径，指定对应的事件
        fsm_eventSignUp(&demoSM, a, b, pressB);  //a->b
        fsm_eventSignUp(&demoSM, a, c, pressC);  //a->c

        fsm_eventSignUp(&demoSM, b, d, pressD);  //b->d

        fsm_eventSignUp(&demoSM, c, a, pressA);  //c->a

        fsm_eventSignUp(&demoSM, d, a, pressA);  //d->a
        fsm_eventSignUp(&demoSM, d, b, pressB);  //d->b


        // 状态机配置完成后，DMEM_BUFFER_SIZE 的取值可以按 map 里 DMEMORY 段的实际占用再往回收
        // （调试时可临时把 stateMachine.c 里的 bufferUsed 改成非 static 打印出来）
    }
    ```

  - 实现 *smDemoRun* 方法，用于运行 *demoSM* 状态机

    ```C
    void smDemoRun(void)
    {
        fsm_run(&demoSM);
    }
    ```

    至此，你的状态机已经构建完成。

- 👉在 *main.c* 中运行你的状态机

    以下代码，引入了您的状态机资源，并完成了状态机的 *Build* 动作。主函数读取用户的输入，并根据用户的输入做出对应的状态跳转动作。

    ```C
    #include <stdio.h>
    #include "smDemo.h"

    int main(int argc, char const *argv[])
    {
        smDemoBuild();
        smDemoRun();    //运行一次状态机，进入初始状态

        inputKey = '0';
        while (1)
        {
            printf("please input a key: ");
            inputKey = getchar();
            if('\n' != inputKey){
                printf("%c is pressed\n", inputKey);
                getchar();   //读取空字符
            }
            smDemoRun();
        }
        
        return 0;
    }
    ```

- 👉效果说明
  - 内存池实际需求量  
    `fsm_init` 完成后，报告里 `DMEMORY` 段的占用就是实际切走的字节数；把 `userSMCfg.h` 里的 `DMEM_BUFFER_SIZE` 调到这个值（留一点余量）即可，避免不必要的内存浪费。
  - 状态机启动后进入默认状态，显示状态的轮询次数，并等待用户输入值
    ![Snipaste_2025-03-22_22-31-52](https://s2.loli.net/2025/03/22/qHsCZ3RiQ5Izfxc.png)
  - 当用户输入后，状态机会检测用户输入是否满足状态跳转条件，如果检测到某键被按下（事件发生），则状态机会跳转到新的状态中，此时会执行前一状态的退出事件（如果有），然后执行新状态的进入事件（如果有）
    ![Snipaste_2025-03-22_22-35-49](https://s2.loli.net/2025/03/22/hpQmNdkxgjGT83u.png)
  - 当用户输入后，如果输入的值不满足当前状态的跳转事件，则状态会会留在当前状态，并执行逗留事件（如果有）
    ![Snipaste_2025-03-22_22-40-01](https://s2.loli.net/2025/03/22/aEi8nDecbTsoI4w.png)
  - 如果状态跳转事件满足，则状态机会在不同的状态之间按注册的跳转路径进行跳转
    ![Snipaste_2025-03-22_22-41-58](https://s2.loli.net/2025/03/22/x86Lv5EgXSjWJPs.png)

    以上完整说明并演示了状态机框架的使用过程和使用效果。在实际的工程项目中，您可以根据您的具体需求定义并实现您的个性化状态机。

- `fsm_init` 的五个参数

  `fsm_init(&myFSM, 状态数, 默认状态, 报警回调, do 后处理回调)`

  - **状态数**：你 enum 里的 `stateID_count`，即状态个数，状态值必须从 0 开始且连续；
  - **默认状态**：上电后进入的状态 ID；
  - **报警回调**（可传 0）：初始化或申请内存失败时先调用它，然后 `while(1)` 停在那里 —— 很适合在这里点灯或响蜂鸣器，方便现场定位；
  - **do 后处理回调**（可传 0）：每个状态的 *Do* 动作之后执行，适合放共性的善后事务（计数更新、执行器刷新等）。

  这两项都不是必需的，传 0 即可；它们的执行时机也完全可以在 *Enter*/*Do* 动作里完成。
## 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request
