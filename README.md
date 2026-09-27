# NetworkSync Demo 当前实现说明

## 1. 项目定位

NetworkSync 是一个基于 Unreal Engine 5.7 Third Person 模板的局域网多人网络同步 Demo。

- 网络拓扑：Listen Server
- 测试方式：PIE 多窗口，1 个 Listen Server 加 1 个或多个 Client
- 目标人数：2~4 人
- 核心闭环：角色移动、水平发射火球、服务端命中判定、生命值复制、头顶血条同步
- 技术原则：C++ 负责网络和玩法骨架，蓝图负责 Mesh、动画、输入资产、粒子和 UI 表现

当前未接入 OnlineSubsystem、Steam、EOS、Dedicated Server、Replication Graph 或自定义传输层。

## 2. 当前目录结构

Demo C++ 类位于：

```text
Source/NetworkSync/
├── NetDemoCharacter.h/.cpp
├── NetDemoGameMode.h/.cpp
├── NetDemoPlayerController.h/.cpp
├── NetDemoRespawnPoint.h/.cpp
├── NetDemoProjectile.h/.cpp
└── UI/
    ├── NetDemoHealthBar.h/.cpp
    └── NetDemoSkillMessageWidget.h/.cpp
```

对应蓝图和地图位于：

```text
Content/Network/Core/BP_NetDemoCharacter.uasset
Content/Network/Core/BP_NetDemoGameMode.uasset
Content/Network/Core/BP_NetDemoPlayerController.uasset
Content/Network/Core/BP_NetDemoProjectile.uasset
Content/Network/UI/BP_NetDemoHealthBar.uasset
Content/Network/Maps/Lv_01.umap
```

`Config/DefaultEngine.ini` 的全局 GameMode 已指向：

```text
/Game/Network/Core/BP_NetDemoGameMode.BP_NetDemoGameMode_C
```

## 3. C++ 类职责

### ANetDemoCharacter

文件：`Source/NetworkSync/Core/NetDemoCharacter.h/.cpp`

职责：

- 继承现有 `ANetworkSyncCharacter`，复用 Third Person 模板的相机、移动和 Enhanced Input 逻辑
- 开启 Actor 复制和移动复制
- 绑定火球输入
- 接收客户端发射请求并在服务器生成火球
- 管理服务器权威生命值和死亡状态
- 创建并刷新角色头顶血条 WidgetComponent

网络属性：

```cpp
UPROPERTY(ReplicatedUsing=OnRep_CurrentHP)
float CurrentHP;

UPROPERTY(ReplicatedUsing=OnRep_IsDead)
bool bIsDead;
```

`CurrentHP` 和 `bIsDead` 只由服务器修改，客户端通过 `OnRep_CurrentHP` 和 `OnRep_IsDead` 做 UI 表现刷新。

### ANetDemoProjectile

文件：`Source/NetworkSync/Core/NetDemoProjectile.h/.cpp`

职责：

- 服务器生成复制的火球 Actor
- 使用 `UProjectileMovementComponent` 进行移动
- 开启 `bReplicates` 和 `SetReplicateMovement(true)`
- 只在服务器碰撞回调中执行伤害
- 通过 `UGameplayStatics::ApplyDamage` 将伤害传给目标
- 提供球体碰撞、Static Mesh、点光源和 Niagara 组件给蓝图配置

### ANetDemoPlayerController

文件：`Source/NetworkSync/Core/NetDemoPlayerController.h/.cpp`

继承 `ANetworkSyncPlayerController`，复用模板的 Enhanced Input Mapping Context 和本地输入初始化逻辑；同时绑定本地 `L` 键循环切换 UE 内置网络模拟档位：

```text
关闭 -> 100ms + 5% 丢包 -> 150ms + 5% 丢包 -> 200ms + 5% 丢包 -> 关闭
```

### ANetDemoRespawnPoint

文件：`Source/NetworkSync/Core/NetDemoRespawnPoint.h/.cpp`

可放置的重生点 Actor。服务器 GameMode 遍历地图中的重生点，并选择距离死亡位置最近的点。

### UNetDemoHealthBar

文件：`Source/NetworkSync/UI/NetDemoHealthBar.h/.cpp`

职责：

- 作为头顶血条 Widget 的 C++ 基类
- 暴露 `SetHealthPercent` 和 `SetDeadState` 蓝图事件
- 绑定所属 Pawn
- 根据所属 Pawn 的 `IsLocallyControlled()` 切换血条颜色
- 通过蓝图公开的 `SelfColor`、`OtherColor` 设置本地和远程角色颜色

## 4. 角色移动同步

移动没有手写位置同步 RPC，使用 UE `ACharacter` 和 `CharacterMovementComponent` 的内置网络机制。

```text
本地输入
  -> Enhanced Input
  -> AddMovementInput
  -> CharacterMovementComponent
  -> 客户端预测
  -> 内置 ServerMove
  -> 服务器权威模拟和校正
  -> 其他客户端接收复制的移动结果
```

关键代码：

```cpp
// NetDemoCharacter.cpp
SetReplicates(true);
SetReplicateMovement(true);
```

输入和方向转换复用：

```text
Source/NetworkSync/NetworkSyncCharacter.cpp
```

客户端只给本地控制的 Pawn 注入输入 Mapping Context，远程角色只接收复制结果。

## 5. 火球发射与同步

客户端按下火球输入后，调用：

```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerRequestFire(const FVector& AimDirection);
```

流程：

```text
客户端按键
  -> 只取控制器 Yaw，构造水平 AimDirection
  -> ServerRequestFire
  -> 服务器校验方向和冷却
  -> 服务器从 ProjectileClass SpawnActor
  -> 火球 Actor 复制到所有客户端
  -> 服务器移动和碰撞判定
  -> 命中后服务器 ApplyDamage 并销毁火球
```

火球出生位置优先取角色 Mesh 的 `FireSocket`，再沿水平发射方向增加 `FireSocketForwardOffset`。如果 Mesh 没有该 Socket，则回退到角色位置上方。

后续技能动画可以直接驱动 Mesh 上的 `FireSocket`，服务器生成点会使用服务器角色 Mesh 的当前 Socket 位置。

火球技能的冷却由角色属性 `FireCooldown` 配置，默认值为 6 秒，可在 `BP_NetDemoCharacter` 的 Class Defaults 中调整。冷却计时只在服务器上使用 `LastFireTime` 判定，客户端不能绕过。冷却期间重复请求会被服务器拒绝，并通过 Client RPC 只通知发起请求的客户端，显示剩余冷却时间。

## 6. 伤害和生命值同步

火球命中时只在服务器执行：

```cpp
UGameplayStatics::ApplyDamage(
    OtherActor,
    Damage,
    GetInstigatorController(),
    this,
    nullptr
);
```

目标角色通过覆盖 `TakeDamage` 接收伤害：

```text
服务器 TakeDamage
  -> 校验目标未死亡且伤害大于 0
  -> 修改 CurrentHP
  -> HP 归零时设置 bIsDead=true
  -> 服务器本地刷新血条
  -> CurrentHP/bIsDead 复制到所有客户端
  -> 客户端 OnRep 刷新血条
```

客户端不能直接修改 `CurrentHP`，`OnRep` 只用于表现，不重复执行伤害或死亡逻辑。

## 7. 头顶血条蓝图配置

### BP_NetDemoHealthBar

父类：NetDemoHealthBar

Class Defaults 中公开两个颜色：

- `SelfColor`：本地控制角色颜色，例如绿色
- `OtherColor`：其他玩家角色颜色，例如红色

颜色选择逻辑由 C++ 执行：

```text
所属 Pawn IsLocallyControlled() == true  -> SelfColor
所属 Pawn IsLocallyControlled() == false -> OtherColor
```

`SetHealthPercent` 蓝图事件连接到：

```text
HealthProgressBar -> Set Percent
```

`SetDeadState` 根据需要隐藏血条、改变颜色或显示死亡状态。

### BP_NetDemoCharacter

角色蓝图使用 C++ 创建的 `HealthBar` WidgetComponent，并设置：

```text
Widget Class = BP_NetDemoHealthBar
```

### BP_NetDemoProjectile

配置：

- `ProjectileMesh`：球体 Static Mesh
- `NiagaraEffect`：火球 Niagara System
- `PointLight`：可选点光源并设置为可见
- `Damage`：火球伤害值
- `LifeSeconds`：火球最大存在时间

### BP_NetDemoCharacter 输入和技能配置

- `FireAction`：绑定火球输入 Action
- `ProjectileClass`：设置为 `BP_NetDemoProjectile`
- `FireSocketName`：默认 `FireSocket`
- 在角色 Skeleton/Mesh 上创建 `FireSocket`，放在法杖顶部

## 8. PIE 验收流程

1. PIE 设置 `Number of Players = 2`，使用 Listen Server + Client New Window。
2. 两端都应看到服务器生成的角色。
3. 移动和跳跃由 `CharacterMovementComponent` 同步。
4. 客户端 A 发射火球，服务器生成火球并复制给双方。
5. 火球保持水平飞行，从Fire Socket 附近生成。
6. 火球命中客户端 B 后，服务器日志输出伤害和剩余 HP。
7. 双方都能看到 B 的血条下降。
8. 当前控制角色血条使用 `SelfColor（默认绿色）`，其他角色血条使用 `OtherColor（默认红色）`。

## 9. 晚加入测试

晚加入测试用于确认新客户端能够从服务器获得已经存在角色的当前状态，而不是只验证初始生成。

### 测试步骤

1. PIE 设置 `Number of Players = 2`，使用 Listen Server + Client New Window。
2. 启动测试，确认 Server 和 Client 都能看到彼此的角色。
3. 让 Client 角色被火球命中，使其 `CurrentHP` 降低；也可以继续攻击直到 `bIsDead = true`，等待它完成一次死亡和重生。
4. 保持 Server 和第一个 Client 继续运行，不停止当前 PIE 会话。
5. 在 PIE 设置中启动第二个 Client，或使用同一 PIE 会话的新增客户端窗口。
6. 观察新客户端进入场景后的已有角色状态。

### 预期结果

- 新客户端能看到服务器已经生成的所有角色及其当前位置。
- 已受伤角色的头顶血条直接显示当前 HP，而不是回到满血。
- 如果加入时角色正处于死亡状态，新客户端收到 `bIsDead=true`，显示死亡血条状态并播放/保持倒地表现。
- 如果加入时角色已经重生，新客户端收到当前的满血和存活状态。
- 新客户端自己的 Pawn 由服务器生成并正常获得本地控制权。

### 复制依据

`ANetDemoCharacter` 使用服务器权威属性复制：

```cpp
UPROPERTY(ReplicatedUsing=OnRep_CurrentHP)
float CurrentHP;

UPROPERTY(ReplicatedUsing=OnRep_IsDead)
bool bIsDead;
```

服务器在 `GetLifetimeReplicatedProps` 中注册这两个属性。客户端收到初始复制或后续更新时，分别通过 `OnRep_CurrentHP` 和 `OnRep_IsDead` 刷新血条及死亡表现。因此晚加入客户端不依赖之前错过的伤害 RPC 或死亡 Multicast。

具体时序是：服务器为新连接创建并 Possess 玩家 Pawn；该 Pawn 作为运行时复制 Actor 出现在新客户端的初始复制包中，同时发送当前位置、`CurrentHP` 和 `bIsDead`。客户端构造 Pawn 后执行初始属性接收，`OnRep_CurrentHP` 刷新血条数值，`OnRep_IsDead` 根据当前死亡状态播放倒地表现或恢复移动。角色位置则由 `SetReplicateMovement(true)` 的移动复制更新。因此同步的是服务器当前状态，而不是重新播放此前发生过的伤害、死亡或移动事件。

### 瞬时 Actor 边界

火球是短生命周期的瞬时 Actor。晚加入客户端看不到已经发射、已经命中或已经销毁的火球，这是预期行为；火球不会被保存成历史事件，也不会为晚加入客户端补播过去的飞行和命中特效。晚加入验证应关注角色的持久状态：位置、`CurrentHP`、`bIsDead`、血条和当前死亡/存活表现。

`IsNetStartupActor()` 只用于判断关卡启动时就存在的网络 Actor。本 Demo 的玩家角色和火球由服务器运行时生成，晚加入同步依赖普通 Actor 初始复制和属性复制，不依赖 Net Startup Actor。

### 测试记录

本节需要在编辑器 PIE 实测后补充：

- 测试日期和 UE 版本
- Server/Client 窗口配置
- 受伤 HP 数值和晚加入时角色是否死亡
- 新客户端观察结果
- Server、Client 窗口截图

## 10. 已实现与未实现

已实现：

- Listen Server 多人角色生成
- UE 内置角色移动同步
- Server RPC 火球请求
- 服务器生成和复制火球
- 水平火球发射
- Socket 发射点
- 服务端碰撞与伤害判定
- `CurrentHP` 和 `bIsDead` 属性复制
- OnRep 血条刷新
- 本地/远程角色血条颜色区分
- 死亡倒地动画和死亡状态复制
- 服务器延迟 2 秒重生和最近重生点选择
- L 键循环网络模拟（关闭、100ms、150ms、200ms；各档 5% 丢包）
- 服务端权威火球技能冷却（默认 6 秒，可由蓝图配置）
- 冷却提示使用本地 Widget，提示显示时重复按键不会刷新；Widget 的 `DisplayDuration` 可在蓝图中配置
- 火球命中特效和音效的 NetMulticast 播放
- 火球本体组件使用 Cascade `UParticleSystemComponent`；命中特效同时支持 Cascade `UParticleSystem` 和 Niagara `UNiagaraSystem`
- 命中特效通过发射者角色的持久网络通道多播，避免短生命周期火球过早销毁导致客户端收不到 RPC

尚未实现：

- 屏幕 HUD（NetMode、Ping、本地玩家名、自身 HP）
- 正式胜负、计分和回合逻辑
- 技能动画复制

## 11. 网络边界声明

本项目定位为网络同步基础演示，采用 UE 原生通用复制系统（未启用 Iris 高级特性）。
已实现的机制：属性复制（Replicated / RepNotify）、Server/Client RPC、服务端权威伤害判定、命中特效 NetMulticast、OnRep 客户端表现。
未实现的机制：自定义传输协议、快照插值、客户端预测手写、断线重连、帧同步、增量热更、OnlineSubsystem 接入。
设计决策：技能伤害在服务端计算以防止客户端作弊；移动同步依赖引擎内置
CharacterMovementComponent 的客户端预测与服务端校正；持久状态通过属性复制，命中特效通过服务端触发的 NetMulticast 表现，与逻辑解耦。

