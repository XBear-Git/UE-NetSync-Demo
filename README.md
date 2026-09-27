# NetworkSync Demo

一款基于 **Unreal Engine 5.7** 的局域网多人网络同步 Demo，采用 **Listen Server** 拓扑，支持 2~4 人同屏联机。项目专注于 UE 原生网络机制的实践，覆盖多人游戏的核心闭环：移动同步、技能发射、伤害判定、生命值复制、死亡重生、头顶血条。

---

## 已实现功能与技术点

- **角色移动同步** – 使用 `CharacterMovementComponent` 内置客户端预测与服务端校正，无需手写同步逻辑。
- **服务端权威技能系统** – 客户端通过 `Server RPC` 请求发射火球，服务端负责生成、碰撞、伤害判定与销毁，杜绝客户端作弊。
- **属性复制** – `CurrentHP` 与 `bIsDead` 通过 `ReplicatedUsing=OnRep` 同步至所有客户端，`OnRep` 仅用于表现刷新，不重复执行逻辑。
- **头顶血条** – 根据 `IsLocallyControlled()` 动态切换颜色（本地绿 / 敌方红），颜色值可在蓝图配置，不通过网络传输。
- **死亡与重生** – 服务端控制死亡状态，2 秒后自动复活至最近重生点，保留原 Pawn 以避免网络身份丢失。
- **技能冷却** – 服务端使用非复制 `LastFireTime` 校验，拒绝过快请求；通过 `Client RPC` 通知请求者剩余冷却时间，本地 Widget 显示倒计时。
- **命中特效多播** – 服务端通过 `NetMulticast` 在所有客户端播放 Cascade/Niagara 特效与音效，利用发射者角色的持久网络通道避免短生命周期 Actor 导致 RPC 丢失。
- **网络模拟** – 按 `L` 键循环切换引擎内置延迟/丢包模拟（关闭 → 100ms+5% → 150ms+5% → 200ms+5%），用于弱网表现演示。
- **晚加入支持** – 新客户端自动同步已有角色的位置、血量、死亡状态（瞬时火球除外），依赖属性复制而非历史事件重播。

---

## 暂未实现

- 屏幕 HUD（NetMode、Ping、本地玩家名、自身 HP 数值）
- 胜负、计分、回合逻辑
- 技能动画复制

---

## 技术边界

- 使用 UE 原生通用复制系统（**未启用 Iris**）
- **未实现**：自定义传输协议、快照插值、客户端预测手写、断线重连、帧同步、OnlineSubsystem 接入
- 设计决策：所有伤害/死亡/冷却逻辑仅在服务端执行；移动同步依赖引擎内置 CMC；持久状态通过属性复制，瞬时事件通过 NetMulticast 表现

---

## 主要文件位置

- **C++ 源码**：`Source/NetworkSync/Core/`（Character、GameMode、PlayerController、Projectile、RespawnPoint）及 `Source/NetworkSync/UI/`（HealthBar、SkillMessageWidget）
- **蓝图与地图**：`Content/Network/Core/`、`Content/Network/UI/`、`Content/Network/Maps/Lv_01.umap`

---

> 详细实现说明见 `Docs/network_demo_implementation.md`。
