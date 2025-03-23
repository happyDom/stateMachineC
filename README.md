# stateMachineC

## 介绍

基于C语言实现的状态机框架，基于此可以快捷的构建一个实用的状态机

## 软件架构

1. 状态机框架头文件<font color=Red>stateMachine.h</font>
2. 状态机框架实现<font color=Red>stateMachine.c</font>
3. 动态内存管理头文件<font color=Red>stateMachineMemmory.h</font>
4. 动态内存管理实现<font color=Red>stateMachineMemmory.c</font>
5. *demo*状态机头文件<font color=Red>smDemo.h</font>
6. *demo*状态机实现<font color=Red>smDemo.c</font>
7. *main*文件<font color=Red>main.c</font>

## 安装教程

- 👉添加本仓库到您的项目中

```Bash
git submodule add git@gitee.com:DyyYq/stateMachineC.git submodule/stateMachineC
```

## 使用说明

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
    stateMachine_t demoSM;
    ```

  - 实现状态机的进入事件，逗留事件和退出事件；你可以为每个状态分别实现不同的进入事件，逗留事件和退出事件，也可以共用它们

    ```C
    void actionEntry(smUnit_t *pSt)
    {
        if(pSt->pSm->roundCounter == 0){
            //状态机首次运行时
            printf("the current state is: %c\n", keys[pSt->stateID]);
        }else{
            //状态机切换进入某一状态时
            printf(" --> %c\n", keys[pSt->stateID]);
        }
    }

    void actionDo(smUnit_t *pSt)
    {
        //状态机的do事件
        printf("----roundCounter of %c is %d\n", keys[pSt->stateID], pSt->roundCounter);
    }

    void actionExit(smUnit_t *pSt)
    {
        //状态机的退出事件
        printf("state exchanged: %c", keys[pSt->stateID]);
    }
    ```

  - 实现状态机跳转事件，用于在每一次轮询状态机时观察是否发生了对应的事件，如果发生了（返回go），则状态跳转到对应的目标状态
  
    ```C
    smEventResult_t pressA(smUnit_t *pSt) {return 'a' == inputKey;};
    smEventResult_t pressB(smUnit_t *pSt) {return 'b' == inputKey;};
    smEventResult_t pressC(smUnit_t *pSt) {return 'c' == inputKey;};
    smEventResult_t pressD(smUnit_t *pSt) {return 'd' == inputKey;};
    ```

  - 实现 *smDemoBuild* 方法，初始化状态机，为每个状态注册对应的事件处理函数，以及为每个状态跳转路径注册对应的事件函数；以下的demo中实现了如下的一个状态机跳转关系：
    ![Snipaste_2025-03-22_22-07-39](https://s2.loli.net/2025/03/22/HFuwUeEj8OZ6cqW.png)

    ```C
    void smDemoBuild()
    {
        fsm_init(&demoSM, stateID_count, 0);
        
        // 注册状态动作
        // 为每一个状态指定进入事件，逗留事件和退出事件
        // 如果你的某些状态不需要全部的事件，则将不需要对应事件的位置使用NULL做入参即可
        demoSM.actionSignUp(&demoSM, a, actionEntry, NULL, actionExit);
        demoSM.actionSignUp(&demoSM, b, NULL, actionDo, actionExit);
        demoSM.actionSignUp(&demoSM, c, actionEntry, actionDo, NULL);
        demoSM.actionSignUp(&demoSM, d, NULL, actionDo, NULL);
        
        // 注册状态事件
        // 为每个状态跳转路径，指定对应的事件
        demoSM.eventSingUp(&demoSM, a, b, pressB);  //a->b
        demoSM.eventSingUp(&demoSM, a, c, pressC);  //a->c

        demoSM.eventSingUp(&demoSM, b, d, pressD);  //b->d

        demoSM.eventSingUp(&demoSM, c, a, pressA);  //c->a

        demoSM.eventSingUp(&demoSM, d, a, pressA);  //d->a
        demoSM.eventSingUp(&demoSM, d, b, pressB);  //d->b

        //状态机配置完成后，打印状态机使用的内存块的数量
        printf("the blockSize needed is: %d\n", dyMM_blocksNumOfUsed());
    }
    ```

  - 实现 *smDemoRun* 方法，用于运行 *demoSM* 状态机

    ```C
    void smDemoRun(void)
    {
        demoSM.run(&demoSM);
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
  - 内存块实际需求量  
    程序运行后，打印显示了状态机构建完成后实际需要的内存块数量，您可以根据这个值调整 *stateMachineMemmory.c* 中的 *DMEM_BLOCK_NUM* 的值，以避免不必要的内存浪费。
    ![Snipaste_2025-03-22_22-27-50](https://s2.loli.net/2025/03/22/S9LA27uXDzr6Bth.png)
  - 状态机启动后进入默认状态，显示状态的轮询次数，并等待用户输入值
    ![Snipaste_2025-03-22_22-31-52](https://s2.loli.net/2025/03/22/qHsCZ3RiQ5Izfxc.png)
  - 当用户输入后，状态机会检测用户输入是否满足状态跳转条件，如果检测到某键被按下（事件发生），则状态机会跳转到新的状态中，此时会执行前一状态的退出事件（如果有），然后执行新状态的进入事件（如果有）
    ![Snipaste_2025-03-22_22-35-49](https://s2.loli.net/2025/03/22/hpQmNdkxgjGT83u.png)
  - 当用户输入后，如果输入的值不满足当前状态的跳转事件，则状态会会留在当前状态，并执行逗留事件（如果有）
    ![Snipaste_2025-03-22_22-40-01](https://s2.loli.net/2025/03/22/aEi8nDecbTsoI4w.png)
  - 如果状态跳转事件满足，则状态机会在不同的状态之间按注册的跳转路径进行跳转
    ![Snipaste_2025-03-22_22-41-58](https://s2.loli.net/2025/03/22/x86Lv5EgXSjWJPs.png)

    以上完整说明并演示了状态机框架的使用过程和使用效果。在实际的工程项目中，您可以根据您的具体需求定义并实现您的个性化状态机。

- 两个未演示的状态机层面的方法  
  以上未演示到的，状态机框架中支持的两个方法是 *actionOnChangeBeforeEnter* 和 *actionAfterDo*，现说明如下
  - *actionOnChangeBeforeEnter* 方法（参数是新的状态的指针）
    该方法在状态跳转发生时，新的状态 *enter* 事件前执行。您可以在此方法中（您需要实现该方法并注册到状态机中）做一些状态切换时要处理的事务。例如计数器复位，再如标志位清除等。
  - *actionAfterDo* 方法（参数是当前状态的指针）
    该方法在状态 *Do* 事件后执行。您可以在此方法中（您需要实现该方法并注册到状态机中）做一些共性的善后事务。例如计数器更新，再如执行器动作等。

    以上两个事件并非必要，其实其所运行的时机完全可以在 *Enter* 事件和 *Do* 事件中完成。

## 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request
