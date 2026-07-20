
# x86-64 目标代码生成与优化 设计文档

## 一、整体管线

```
源文件(.rs) → 词法分析 → Token流 → 语法分析 → AST
                                              ↓
                                      语义分析 (类型检查)
                                              ↓
                                      中间代码生成 (四元式 IR)
                                              ↓
                                    ┌─ IR 优化 ─────────────────┐
                                    │  · 常量折叠               │
                                    │  · 死代码消除             │
                                    │  · 复制传播               │
                                    │  · 窥孔优化               │
                                    └───────────────────────────┘
                                              ↓
                            目标代码生成 (x86-64 AT&T 汇编)
                                              ↓
                                      汇编 → 链接 → 可执行文件
```

## 二、IR 指令集回顾

当前四元式指令共 27 种，按语义分组：

| 分组 | 指令 | 格式 | 说明 |
|------|------|------|------|
| 数据移动 | `ASSIGN` | `(ASSIGN, src, -, dest)` | 赋值 |
| 算术 | `ADD, SUB, MUL, DIV` | `(ADD, a, b, result)` | 双目运算 |
| 一元算术 | `NEG` | `(NEG, a, -, result)` | 取负 |
| 比较 | `EQ, NE, LT, LE, GT, GE` | `(EQ, a, b, result)` | 结果存入临时变量 |
| 控制流 | `LABEL` | `(LABEL, name, -, -)` | 标号 |
|  | `JUMP` | `(JUMP, label, -, -)` | 无条件跳转 |
|  | `JZ` | `(JZ, cond, label, -)` | cond==0 跳转 |
|  | `JNZ` | `(JNZ, cond, label, -)` | cond!=0 跳转 |
| 函数 | `FUNC_BEGIN` | `(FUNC_BEGIN, name, -, -)` | 函数入口 |
|  | `FUNC_END` | `(FUNC_END, name, -, -)` | 函数出口 |
|  | `PARAM` | `(PARAM, val, -, -)` | 传递实参 |
|  | `CALL` | `(CALL, fn, argc, result)` | 函数调用 |
|  | `RETURN` | `(RETURN, val, -, -)` | 函数返回 |
| 内存 | `REF` | `(REF, var, -, result)` | 取地址 |
|  | `DEREF` | `(DEREF, ptr, -, result)` | 解引用读 |
|  | `DEREF` | `(DEREF, val, -, ptr)` | 解引用写 |
|  | `INDEX_LOAD` | `(INDEX_LOAD, arr, idx, result)` | 数组读 |
|  | `INDEX_STORE` | `(INDEX_STORE, val, idx, arr)` | 数组写 |
|  | `ARRAY_LIT` | `(ARRAY_LIT, elem, idx, result)` | 数组字面量 |
| 循环 | `BREAK, CONTINUE` | 转为 `JUMP` | 编译时已处理 |
| 其他 | `NOP, TUPLE_GET` | — | 空操作/元组 |

## 三、x86-64 目标平台约定

### 3.1 调用约定 (System V AMD64 ABI)

| 项目 | 约定 |
|------|------|
| 参数传递 | 前 6 个整数：`%rdi, %rsi, %rdx, %rcx, %r8, %r9`；其余压栈 |
| 返回值 | `%rax`（整数） |
| 调用者保存 | `%rax, %rcx, %rdx, %rsi, %rdi, %r8~%r11` |
| 被调用者保存 | `%rbx, %rbp, %r12~%r15` |
| 栈对齐 | 16 字节对齐（call 前） |
| 红区 | 128 字节（`%rsp` 下方，叶函数可用） |

### 3.2 栈帧结构（活动记录）

程序运行时以栈结构存放各过程的**活动记录**（Activation Record，即栈帧）。每次过程调用在栈顶新增活动记录，过程返回则销毁栈帧。

#### 3.2.1 基础字段布局

每个栈帧固定排布如下（按照 SP 相对偏移从低到高）：

| 偏移 | 字段 | 说明 |
|------|------|------|
| SP+0 | **动态链** | 调用者的 SP（老 SP），指向调用者的栈帧基址 |
| SP+1 | **返回地址** | `call` 指令自动压入，指向调用者中 `call` 的下一条指令 |
| SP+2 | **静态链** | 指向**静态直接外层**过程的活动记录基址 |
| SP+3 | 参数个数 | 本次调用的实参数量 |
| SP+4 | 形参/实参区 | 过程参数存放区 |
| ... | 局部变量 | 本过程声明的局部变量 |
| ... | 临时变量 | 表达式求值过程中产生的临时值 |
| ... | 内情向量 | 数组等需要运行时信息的变量描述符 |
| TOP   | — | 栈当前顶端位置 |

对应 x86-64 具体实现：

```
        ┌────────────────────┐  高地址（栈底方向）
        │    调用者栈帧       │
        ├────────────────────┤
        │   实参 N            │  ← 右→左压栈
        │   ...   (outgoing)  │
        │   实参 7            │
        ├────────────────────┤
        │   返回地址          │  ← call 指令自动压入
   %rbp ├────────────────────┤  ← 帧基址（动态链 = 老 %rbp）
        │   静态链            │  ← 指向静态外层帧 %rbp
        │   callee-save regs  │  (%rbx, %r12~%r15)
        ├────────────────────┤
        │   局部数组/元组     │
        │   ...               │
        │   局部变量 spill 区  │  （寄存器溢出槽）
        ├────────────────────┤
        │   outgoing args     │  （调用子过程用的实参构造区）
   %rsp └────────────────────┘  低地址（栈顶）
```

#### 3.2.2 动态链（控制链 / Control Link）

- **定义**：存储当前过程**直接调用者**的活动记录起始地址（`%rbp` 值）
- **性质**：完全跟随**运行时调用顺序**生成，递归调用会形成多层动态链
- **作用**：过程返回时通过动态链恢复上层 `%rsp` 和 `%rbp`，完成栈帧回收——这是 `leave` 指令的核心依据：

```asm
; x86-64 返回序列
leave           ; ≡ movq %rbp, %rsp; popq %rbp
ret             ; 弹出返回地址并跳转
```

- **形成方式**：每次 `call` 前，调用者将自己的 `%rbp` 作为被调者帧的动态链

#### 3.2.3 静态链（存取链 / Access Link）

- **定义**：由源代码**静态嵌套层级**决定，与运行时调用顺序无关
- **作用**：用于访问**外层非局部变量**（嵌套过程中引用外层声明的变量）
- **层级规则**：
  - 主程序（最外层）为 0 层
  - 内层过程 = 直接外层过程的层级 + 1
- **赋值规则**：
  - 若调用**静态直接内层**过程：新栈帧静态链 = 调用者的 SP（`%rbp`）
  - 若**递归调用自身**：新栈帧静态链 = 复用调用者的静态链（始终保持指向固定的静态外层）
  - 若**跨层调用**（调用兄弟过程或外层过程）：通过调用者的静态链查找目标静态层

示例：
```rust
fn outer() {           // 层 0
    let x = 1;
    fn inner() {       // 层 1
        let y = x;     // 通过静态链访问 outer 的 x
    }
    inner();
}
```

`inner` 的栈帧中，静态链指向 `outer` 的 `%rbp`，访问 `x` 时沿静态链 → `outer` 帧 → `x` 的偏移量。

#### 3.2.4 Display 表（优化结构）

逐层遍历静态链效率低（每多一层嵌套就多一次间接寻址）。**Display 表**用空间换时间：

- 在活动记录中增设**嵌套层次显示表**——数组下标对应源码层级，直接存储对应层级栈帧地址
- 新建过程时：截取调用者 Display 表中 `[0..自身层级-1]` 的数据，追加自身 `%rbp` 作为 `[自身层级]`，生成专属 Display 表
- 访问 `k` 层变量：直接 `Display[k] + offset`，O(1) 而非 O(n) 逐层遍历

```
Display 表示例（3层嵌套）：
  Display[0] → 主程序帧基址
  Display[1] → 第1层过程帧基址
  Display[2] → 当前（第2层）过程帧基址  ← 自身
```

对于当前 Rust-like 语言（嵌套深度 ≤ 2，函数定义仅在顶层），Display 表可简化为最多 2 个槽位。若语言后续支持更深嵌套，Display 表优势会更明显。

### 3.3 栈帧完整生命周期

#### 调用阶段（Call）

```
1. 调用者计算实参，依次压栈（或移入 %rdi/%rsi/%rdx/%rcx/%r8/%r9）
2. call 指令：
   a. 将返回地址（下一条指令地址）压栈
   b. 跳转到被调函数入口
3. 被调函数序言：
   pushq %rbp              ; 保存调用者 rbp（= 动态链）
   movq  %rsp, %rbp        ; 建立新帧基址
   pushq static_link       ; 存入静态链（按 3.2.3 规则计算）
   pushq callee-save regs  ; 保存 %rbx, %r12~%r15
   subq  $N, %rsp          ; 分配局部变量 / 临时变量空间（抬高 TOP）
```

#### 返回阶段（Return）

```
1. 被调函数结语：
   movq  val, %rax         ; 返回值放入 %rax
   ; 恢复 callee-save 寄存器
   leave                   ; ≡ movq %rbp, %rsp; popq %rbp（恢复调用者帧）
   ret                     ; 弹出返回地址并跳转
2. 调用者清理实参栈空间（若使用栈传参）
```

关键：返回阶段**仅借助动态链**恢复上层栈基，丢弃当前栈帧，栈回到调用前状态。静态链不参与返回——它只用于变量寻址。

## 四、寄存器分配与管理算法

寄存器分配是目标代码生成的**前置核心**——所有运算必须在寄存器中完成，分配质量直接决定生成代码的性能。

### 4.1 两大记录数组（全程维护寄存器状态）

```
RVALUE[R]  —— 寄存器描述：记录寄存器 R 当前存放了哪些变量（可能多个）
AVALUE[X]  —— 变量地址描述：记录变量 X 当前在哪个寄存器 / 内存中
```

每条四元式生成指令后同步更新这两个数组，保证状态一致性。

### 4.2 待用信息与活跃变量分析（寄存器分配前置）

从基本块**末尾反向**遍历四元式，为每个变量标记：

| 信息 | 含义 |
|------|------|
| **待用信息** | 变量下一次被使用的位置（行号）；若当前行之后不再读取 → 标记为"非待用" |
| **活跃信息** | 块结束后该变量是否还会被使用（跨块存活） |

**作用**：
- 变量**不活跃**且**无后续待用**时，可立即释放占用的寄存器
- GETREG 选择被抢占寄存器时，优先抢"最晚才用到"的变量

### 4.3 GETREG 寄存器选取核心算法

为四元式 `A := B op C` 分配存放结果 A 的寄存器 R，按三层优先级选取：

```
          ┌──── 需要寄存器 R 存放 A:=B op C ────┐
          │                                       │
          ▼                                       否
    B 独占某寄存器 Ri？ ───是──▶ 取 R = Ri
    (B只在Ri中，且运算后B不再使用)
          │
          │否
          ▼
    有空闲寄存器 Rj？ ───是──▶ 取 R = Rj
          │
          │否
          ▼
    抢占已占用寄存器：
    遍历已占用寄存器，选择「未来最晚使用」或「值已在内存有副本」的变量对应的寄存器
          │
          ▼
    抢占前处理：被抢占变量仅存寄存器（无内存副本）
    → 生成 ST R, mem[X]  写回内存
    → 清空 RVALUE[R]、AVALUE[X]
```

### 4.4 寄存器复用规则（简单拷贝 A:=B）

若四元式为 `A := B`（`ASSIGN`），且 B 已在寄存器 Ri 中：

```
不生成任何 LD/ST 指令
仅更新：
  RVALUE[Ri] += {A}
  AVALUE[A]  = Ri
```

A 和 B **共享同一寄存器**，零开销。

### 4.5 寄存器回收规则

运算结束后，检查 B、C：
- 若 B 在当前行后**不再待用**且**块出口不活跃** → 从 `RVALUE`、`AVALUE` 中清除 B 的记录
- 若 C 同理 → 清除 C 的记录

### 4.6 极简单寄存器退化模式

若只有 **1 个可用寄存器**（或作为调试/基线模式）：

```
每条运算：
  LD  R, B      ; 加载 B
  op  R, C      ; 运算
  ST  R, T      ; 结果写回临时变量
```

无复用逻辑，代码冗余多，但实现简单、**结果正确**。可作为"先跑通再优化"的基线方案，后续逐步启用 GETREG 完整分配。

---

## 五、指令选择与目标代码生成算法（Quadruple → x86-64）

基于 GETREG 寄存器分配，逐类四元式翻译为汇编指令。

**基础原则**：目标机器模型假定拥有多通用寄存器，支持 LD/ST/ADD/SUB/MUL/DIV、条件跳转、变址与间接寻址，所有运算操作数必须先加载到寄存器中。

### 5.1 标准双目运算 A := B op C

完整 4 步生成流程：

```
1. 调用 GETREG，获取存放结果 A 的寄存器 R
2. 查询 AVALUE，获取 B、C 当前存储位置（寄存器 B' / 内存 B'，寄存器 C' / 内存 C'）
3. 加载并执行运算：
   · 若 B' == R（B 已在 R 中）：直接  op  R, C'
   · 若 B' != R：                   LD   R, B'; op  R, C'
4. 维护状态表：
   · RVALUE[R] = {A}
   · AVALUE[A] = {R}
5. 释放无用寄存器：B、C 若后续不再使用，从 RVALUE/AVALUE 中清除
```

x86-64 对应指令：

| 四元式 | 汇编模板 |
|--------|---------|
| `(ASSIGN, src, -, dest)` | `movl src, %e; movl %e, dest`（若 src==dest 则消除） |
| `(ADD, a, b, R)` | `movl a, R; addl b, R` |
| `(SUB, a, b, R)` | `movl a, R; subl b, R` |
| `(MUL, a, b, R)` | `movl a, %eax; imull b`（MUL 需 %eax 为隐式被乘数） |
| `(DIV, a, b, R)` | `movl a, %eax; cdq; idivl b; movl %eax, R` |

### 5.2 单目运算 A := op B

逻辑同 5.1，仅省略第二个操作数 C：

```
1. GETREG 获取 R
2. 查 AVALUE 获取 B'
3. 若 B' != R → LD R, B'
4. 执行 op R（如 NEG → negl R）
5. 更新 RVALUE/AVALUE；释放 B（若非待用）
```

### 5.3 比较运算

```
(EQ, a, b, result)      →  cmpl  b, a; sete  %al; movzbl %al, result
(NE, a, b, result)      →  cmpl  b, a; setne %al; movzbl %al, result
(LT, a, b, result)      →  cmpl  b, a; setl  %al; movzbl %al, result
(LE, a, b, result)      →  cmpl  b, a; setle %al; movzbl %al, result
(GT, a, b, result)      →  cmpl  b, a; setg  %al; movzbl %al, result
(GE, a, b, result)      →  cmpl  b, a; setge %al; movzbl %al, result
```

### 5.4 分支跳转

用 `if B rop C goto L` 语义翻译：

```
1. 若 B、C 不全在寄存器中 → 先 LD 加载
2. CMP  B, C          ; 比较，设置标志位（EFLAGS）
3. 根据 rop 生成条件跳转：
   (<)  → JL  L
   (<=) → JLE L
   (==) → JE  L
   (!=) → JNE L
   (>)  → JG  L
   (>=) → JGE L
```

对应四元式：

```
(LABEL, L0, -, -)      →  L0:
(JUMP, L0, -, -)       →  jmp   L0
(JZ, cond, L0, -)      →  cmpl  $0, cond; je  L0
(JNZ, cond, L0, -)     →  cmpl  $0, cond; jne L0
```

### 5.5 数组寻址 A := B[C] / B[C] := A

通过**变址寻址**生成地址访问：

```
A := B[C]（INDEX_LOAD）：
  LD   Ri, C           ; 索引加载到寄存器
  LD   R,  B(Ri)       ; 变址寻址：base + index×scale
  ; 更新 RVALUE[R] = {A}, AVALUE[A] = {R}

B[C] := A（INDEX_STORE）：
  LD   Ri, C           ; 加载索引
  LD   Rj, A           ; 加载待存值
  ST   Rj, B(Ri)       ; 变址存储
```

x86-64 SIB 寻址：`movl arr_base(,%idx,4), %r`

### 5.6 过程调用四元式翻译

#### 实参序列（PARAM）

多条 `(PARAM, Ti, -, -)` 循环生成：把实参值写入栈顶 TOP 区域（或移入参数寄存器），收集参数列表。

#### 调用指令（CALL）

`(CALL, P, n, result)`：

```
1. 保存当前 %rsp → 老 SP 位置（动态链）
2. 写入参数个数 n
3. 前 6 个实参 → %rdi, %rsi, %rdx, %rcx, %r8, %r9
4. 第 7+ 实参 → pushq（逆序压栈）
5. call  P              ; JSR：保存返回地址 + 跳转
6. movl  %rax, result   ; 保存返回值
```

#### 被调过程入口

```
(FUNC_BEGIN, fn, -, -)：
  .globl fn
  fn:
   pushq %rbp
   movq  %rsp, %rbp
   更新 %rsp = TOP + 1
   分配局部空间 → subq $N, %rsp（抬高 TOP）
```

#### 过程返回

```
(RETURN, val, -, -)
  →  movl  val, %rax

(FUNC_END, fn, -, -)
  →  TOP  = %rsp - 1     ; 丢弃局部空间
     %rsp = 动态链老 %rsp ; 恢复调用者栈帧
     取出返回地址 → ret
```

即 x86 的 `leave; ret`。

### 5.7 基本块收尾处理

基本块内所有四元式翻译完毕后，**块出口仍活跃**的变量必须生成 `ST` 指令从寄存器写回内存：

```
for each 在块出口仍活跃的变量 X：
  if AVALUE[X] 指向某寄存器 R：
     生成  ST  R, mem[X]    ; 写回内存
     AVALUE[X] = {mem}      ; 更新地址描述
```

否则寄存器中的值跨块时可能被覆盖，后续块无法获取正确值。

---

## 六、编译优化

优化在 IR 层面进行，不改变程序语义。整体分三大类：**局部优化**（基本块内）、**循环优化**（收益最高）、**全局优化**（跨基本块），另有**配套分析算法**支撑所有优化执行。

### 6.1 局部优化（基本块内）

**基本块**是一段只有单一入口、单一出口、无跳转中断的连续四元式代码。局部优化全部局限在块内执行。

#### 6.1.1 删除公共子表达式（Common Subexpression Elimination）

同一计算表达式重复出现：只计算第一次，后续直接复用临时变量。

```
优化前：                      优化后：
(MUL, 4, i, T1)              (MUL, 4, i, T1)
...                          ...
(MUL, 4, i, T2)              (ASSIGN, T1, -, T2)   ← 复用 T1，省去乘法
```

#### 6.1.2 复写传播（Copy Propagation）

消除单纯的变量拷贝语句，所有用到拷贝变量的地方全部替换为原始变量。

```
优化前：                      优化后：
(ASSIGN, a, -, T6)           (ADD, a, 1, T8)       ← T6 被替换为 a
(ASSIGN, T6, -, T7)          (ADD, a, 1, T8)       ← T7 也替换为 a
(ADD, T7, 1, T8)
```

#### 6.1.3 删除无用赋值 / 死代码消除（Dead Code Elimination）

变量被赋值后，全程再无任何读取——该赋值语句直接删除，不影响程序结果。

```
优化前：                      优化后：
(ASSIGN, 1, -, t0)           (ASSIGN, 3, -, t1)     ← t0 被消除
(ASSIGN, 3, -, t1)
(RETURN, t1, -, -)           (RETURN, t1, -, -)
```

**算法**：反向扫描，维护活跃变量集合；`result` 不在活跃集中的指令可删除。

#### 6.1.4 合并已知量 / 常量折叠（Constant Folding）

编译阶段就能算出固定常数的表达式，直接求值替代，不等运行时再算。

```
优化前：                      优化后：
(ASSIGN, 2, -, T1)           (ASSIGN, 8, -, T2)     ← 一步算出
(MUL, 4, T1, T2)
```

**算法**：遍历四元式，若 `arg1` 和 `arg2` 均为整数常量，直接计算并替换为 `ASSIGN`。

#### 6.1.5 代数变换（Algebraic Simplification）

依靠数学恒等式简化运算：

| 模式 | 替换 |
|------|------|
| `(ADD, X, 0, T)` 或 `(SUB, X, 0, T)` | `(ASSIGN, X, -, T)` |
| `(MUL, X, 1, T)` | `(ASSIGN, X, -, T)` |
| `(MUL, X, 0, T)` | `(ASSIGN, 0, -, T)` |
| `(DIV, X, 1, T)` | `(ASSIGN, X, -, T)` |
| `(MUL, X, 2, T)` | `(ADD, X, X, T)` |
| 高开销运算 `pow(X,2)` | `(MUL, X, X, T)` |

#### 6.1.6 临时变量改名

只修改临时变量名，计算逻辑完全不变。用于规整代码、便于后续优化识别模式。例如将分散命名的 `T1, T5, T9` 统一为 `T1, T2, T3` 连续编号。

#### 6.1.7 交换无关语句位置

两条相邻语句不存在数据依赖（不读对方写入的变量），调换先后顺序不影响结果，目的是为后续其他优化创造相邻模式（如让公共子表达式相邻，便于 CSE 识别）。

#### 6.1.8 DAG 图优化（局部优化统一实现）

给基本块构建**有向无环依赖图**（DAG），一张图同时完成：

- 合并常量（常量折叠）
- 删除公共子表达式
- 删除无用赋值

是局部优化的**通用实现方案**。DAG 节点表示运算（操作数 + 操作符），边表示依赖关系；相同运算指向同一节点即自动消除重复。

### 6.2 循环优化（收益最高）

循环代码重复执行成千上万次，内部冗余运算每轮都重复。循环优化是**整体收益最高**的优化。

#### 6.2.1 代码外提（Loop-Invariant Code Motion）

**循环不变运算**：式子中所有参与计算的值都在循环外定义，循环全程不变。将这类语句从循环体挪到循环入口前置块，避免每轮重复计算。

```
优化前：                      优化后：
while i < n {                (MUL, i, 2, T1)       ← 外提到前置块
    T1 = i * 2;              while i < n {
    T2 = a + T1;                 T2 = a + T1;
    ...                          ...
}                            }
```

**外提的三个必要条件**（必须全部满足）：
1. 该语句所在块是循环所有出口的**必经节点**
2. 该变量在循环内部**不会被重新赋值**
3. 循环内所有读取该变量的地方，**只能来自这一次赋值**

补充：若循环结束后该变量不再使用，只需满足条件 ②③ 即可。

#### 6.2.2 强度削弱（Strength Reduction）

把循环内开销大的乘除法替换为开销小的加减法，最常用于数组下标线性计算。

```
优化前（每轮做乘法）：           优化后（每轮只做加法）：
for i in 0..n {                T2 = 10 * 0;          ← 初始值
    T2 = 10 * i;               for i in 0..n {
    arr[T2] = 0;                   arr[T2] = 0;
}                                  T2 = T2 + 10;     ← 每次只加 10
                               }
```

#### 6.2.3 删除归纳变量（Induction Variable Elimination）

**基本归纳变量**：循环里只有 `I = I ± 常数` 这种自增自减的变量。

**同族归纳变量**：与基本变量满足固定线性关系 `J = C1 × I ± C2`。

优化两步：
1. 对同族变量先做强度削弱（如上节）
2. 若基本归纳变量**只用于循环判断**且循环结束后不再使用，用同族变量替换判断条件并删除基础变量的自增代码

```
优化前（I 只用来判断+驱动 J）：   优化后（I 被消除）：
for i in 0..n {                T2 = 0;
    T2 = 4 * i;                for T2 in 0..4*n step 4 {
    ...                            ...
}                            }
```

#### 6.2.4 循环展开（Loop Unrolling）

复制多份循环体，减少跳转和条件判断次数，降低分支开销。每次迭代处理多个元素。

```
优化前：                      优化后（展开 2×）：
for i in 0..n {              for i in 0..n step 2 {
    body(i);                      body(i);
}                                 body(i + 1);
                              }
                              // 处理剩余奇数元素（epilogue）
```

#### 6.2.5 循环合并（Loop Fusion）

两段独立、循环次数相同的相邻循环合并为一个，省去一套循环控制逻辑。

```
优化前：                      优化后：
for i in 0..n {              for i in 0..n {
    a[i] = b[i] + 1;             a[i] = b[i] + 1;
}                                 c[i] = d[i] * 2;
for i in 0..n {              }
    c[i] = d[i] * 2;
}
```

### 6.3 全局优化

作用范围覆盖整个程序所有基本块，跨块分析数据流与变量生命周期：

- **全局公共子表达式消除**（Global CSE）：跨基本块识别相同表达式
- **全局死代码消除**：跨块追踪变量活跃性
- **内联展开**（Inline Expansion）：将小函数体替换调用点，消除调用开销
- **过程间优化**（IPO）：跨函数边界的常量传播与别名分析

（本节保留分类定义，具体算法在实现阶段细化。）

### 6.4 窥孔优化（Peephole Optimization）

在指令序列上滑动窗口（3~5 条），匹配模式并直接替换。此优化在 IR 和目标指令两个层面均可用：

| 模式 | 替换 |
|------|------|
| `movl a,%e; movl %e,b` | `movl a,b`（%e 后续不被使用） |
| `pushq %r; popq %r` | 删除 |
| `cmpl $0,x; je L; jmp L` | `cmpl $0,x; je L` |
| `(ADD, X, 0, T)` | `(ASSIGN, X, -, T)` |
| `(MUL, X, 1, T)` | `(ASSIGN, X, -, T)` |
| `movl $0,%e; movl %e,x` | `movl $0,x` |
| `jmp L; L:` | 删除无用跳转 |

### 6.5 配套前置分析算法

所有优化都依赖以下分析。没有这些分析，优化无法正确执行。

#### 6.5.1 基本块划分

将一长串四元式按跳转边界切分为独立基本块（局部优化的前提）：

- **入口**：① 程序第一条指令；② LABEL 标记的指令；③ 跳转目标指令
- **出口**：① 下一条是 LABEL；② JUMP/JZ/JNZ；③ RETURN/CALL

#### 6.5.2 流图构建

以基本块为**节点**、跳转语句为**边**构建控制流图（CFG），用于：
- 识别循环（回边检测自然循环）
- 计算必经节点（DOM 树，支撑代码外提合法性判断）
- 数据流分析的基础框架

#### 6.5.3 活跃变量分析（Live Variable Analysis）

判断某变量在后续是否会被读取（反向数据流分析）：

- **活跃变量**：当前值在后续会被使用
- **死变量**：当前值后续不再使用 → 可消除其赋值

#### 6.5.4 到达-定值分析（Reaching Definitions）

判断一个变量的赋值能到达哪些后续使用点：

- 某一行的定值能否"到达"后面的某行 → 判断常量传播、循环不变量外提的合法性
- 并行多路径定值 → 需要 `φ` 函数或 `use-def` 链处理

#### 6.5.5 DAG 构造算法

搭建基本块内运算的有向无环依赖图：

1. 遍历块内四元式，为每条指令创建节点
2. 操作数和操作符相同 → 复用已有节点（自动消除公共子表达式）
3. 常量操作数 → 合并为常量节点（自动常量折叠）
4. 拓扑排序输出优化后的指令序列

### 6.6 优化 Pass 编排

按此顺序执行优化 Pass：

```
IR 序列
  │
  ├── 分析:  基本块划分 + 流图构建 + 活跃变量分析
  │
  ├── Pass 1: 常量折叠 + 代数变换（消除无用运算）
  ├── Pass 2: 复写传播（消除拷贝语句）
  ├── Pass 3: 局部 CSE + DAG 优化（消除冗余计算）
  ├── Pass 4: 循环不变量外提（代码外提）
  ├── Pass 5: 强度削弱 + 删除归纳变量
  ├── Pass 6: 死代码消除（清理前序 pass 产生的垃圾）
  ├── Pass 7: 窥孔优化（模式匹配局部精修）
  │
  ├── Pass 8: 常量折叠（第二轮——前序优化可能暴露新常量）
  └── Pass 9: 死代码消除（第二轮收尾）
```

### 6.7 优化效果估计

以 `test_full.rs`（243 条四元式）为例，预期优化效果：

| Pass | 消除指令数 | 缩减比例 |
|------|-----------|---------|
| 常量折叠 + 代数变换 | ~10 | ~4% |
| 复写传播 | ~20 | ~8% |
| 局部 CSE | ~8 | ~3% |
| 死代码消除 | ~30 | ~12% |
| 窥孔优化 | ~15 | ~6% |
| **合计** | **~83** | **~34%** |

## 七、目标代码生成器架构

### 7.1 类结构

```cpp
class CodeGenerator {
    // 输入
    vector<Quadruple>& ir;

    // 输出
    stringstream asm_output;           // 汇编文本

    // 寄存器状态表（基于 RVALUE/AVALUE 模型）
    unordered_map<string, set<string>> RVALUE;  // 寄存器 → 变量集合
    unordered_map<string, string> AVALUE;       // 变量 → 位置（"R:0" / "mem:offset"）

    // 待用/活跃信息（寄存器分配前置分析结果）
    unordered_map<string, int> next_use;       // 变量 → 下一次使用行号
    unordered_map<string, bool> live_out;      // 变量 → 块出口是否活跃

    // 栈帧
    int stack_frame_size;
    unordered_map<string, int> stack_offsets;

    // 核心方法
    void genFunction(const string& name, const vector<Quadruple>& body);
    void genInstruction(const Quadruple& q);
    string GETREG(const Quadruple& q);          // 寄存器分配核心
    void updateState(const string& r, const string& var);
    void releaseReg(const string& var);
    void spillVar(const string& var);           // 溢出到内存
    void genBlockEpilogue(int block_id);        // 基本块收尾
};
```

### 7.2 生成流程

```
1. 遍历 IR，收集所有函数（以 FUNC_BEGIN/FUNC_END 为界）
2. 对每个函数：
   a. 统计局部变量，分配栈槽
   b. 生成函数序言（prologue）
   c. 遍历函数体四元式，逐条生成指令
   d. 生成函数结语（epilogue）
3. 输出完整汇编文件
```

### 7.3 新增 CLI 参数

```bash
./parser --input src.rs --asm-output output.s   # 生成汇编
./parser --input src.rs -O0 --asm-output out.s  # 无优化
./parser --input src.rs -O1 --asm-output out.s  # 一级优化（推荐）
./parser --input src.rs -O2 --asm-output out.s  # 二级优化（更激进）
```

### 7.4 新增 GUI 视图

在右侧面板增加 "Assembly" 视图按钮，展示生成的汇编代码，排版为等宽字体、带行号的可滚动区域。

## 八、参考文献

1. **System V Application Binary Interface: AMD64 Architecture Processor Supplement** — x86-64 调用约定标准
2. **Intel® 64 and IA-32 Architectures Software Developer's Manual** — x86 指令集参考
3. Aho, Lam, Sethi, Ullman. *Compilers: Principles, Techniques, and Tools* (2nd Ed.). — 第 8 章（代码生成）、第 9 章（优化）
4. Andrew Appel. *Modern Compiler Implementation in C*. — 寄存器分配与指令选择
5. Massalin, Henry. "Superoptimizer: A Look at the Smallest Program." ASPLOS 1987. — 窥孔优化参考
6. Poletto, Sarkar. "Linear Scan Register Allocation." TOPLAS 1999. — 线性扫描寄存器分配
