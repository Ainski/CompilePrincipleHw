# 报告图表 AI 生成 Prompt

本文件存储用于 AI 图片生成工具生成报告插图的 prompt。
每个条目以 YAML front matter 格式定义元信息，`### prompt` 以下为生成 prompt 正文。

---

## 跨平台构建流程图

<!-- figure
id: 1
title: 跨平台构建流程图
section: 5.2 Linux/Windows 交叉编译配置
output: figures/cross_platform_build.png
size: 1792x1024
-->

### prompt

Generate a clean, professional technical flowchart (横向流程图) in white background with dark text, suitable for inclusion in a LaTeX academic report. Use a modern flat style with rounded rectangles for process steps, parallelograms for input/output, and diamond for decision. Color scheme: soft blue (#4A90D9) for process boxes, light green (#5CB85C) for outputs, light orange (#F0AD4E) for tools/dependencies, gray borders. No gradients, no shadows, no 3D effects.

The diagram illustrates a cross-platform build pipeline that runs on Linux and produces executables for both Linux and Windows:

1. **Input (left side, parallelogram):** Source files — `lexer.l`, `main.cpp`, `TokenStream.cpp`, `Type.cpp`

2. **Flex code generation (process box):** Flex 2.6.4 processes `lexer.l` → generates `lex.yy.cpp`

3. **CMake configuration (process box):** CMake reads `CMakeLists.txt`, configures two parallel build paths

4. **Dependency management (box above CMake):** vcpkg installs GLFW3 in two versions:
   - `glfw3:x64-linux` for native build
   - `glfw3:x64-mingw-dynamic` for cross-compilation

5. **Upper path — Linux native build (horizontal):**
   - Tool: GCC / G++ (C++17)
   - Build directory: `build/`
   - Output (green parallelogram): `build/bin/parser` (Linux ELF executable)

6. **Lower path — Windows cross-compilation (horizontal):**
   - Tool: MinGW-w64 toolchain (`x86_64-w64-mingw32-g++`)
   - Build directory: `build-win/`
   - Additional step: copy `glfw3.dll` from vcpkg to `build-win/bin/`
   - Output (green parallelogram): `build-win/bin/parser.exe` (Windows PE executable)

7. **Final step (right side):** Linux executable runs the parser with `--input` flag to produce lexer TSV and parse tree output.

Use arrows to connect the flow. Label each arrow with the tool or command involved (e.g., "flex", "cmake ..", "make -j4", "cmake .. -DCMAKE_TOOLCHAIN_FILE=..."). Add small tool icons or labels near each process box. The overall layout should be wide (landscape-oriented), flowing left to right, with the two parallel build paths clearly separated.

---

## GUI 模块架构图

<!-- figure
id: 2
title: GUI 模块架构图
section: 4.4 GUI架构
output: figures/gui_architecture.png
size: 1024x1536
-->

### prompt

Generate a clean, professional layered architecture diagram (分层架构图) with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for application layer, light green (#5CB85C) for core logic layer, light orange (#F0AD4E) for data/state layer, light purple (#9B59B6) for rendering/backend layer. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict box/node visual rules (you MUST follow these):
1. ALL boxes must use **rounded rectangles** with a **small uniform corner radius** (approximately 6px). Differentiate box types ONLY by color, not by shape.
2. Each box may optionally contain an internal **horizontal divider line** (thin 1px, solid, same color as the box border) separating a header from the body. If present, header text is **bold**, body is regular weight. Apply consistently to ALL multi-line boxes.
3. All box borders must be **1.5px solid gray (#999999)**, consistent across every box.

IMPORTANT — Strict arrow rules (you MUST follow these):
4. ALL arrows must be **strictly single-directional** with **NO bidirectional arrows**. The arrowhead (triangular tip) must appear ONLY at the destination end. The source end and the middle of the arrow line must be plain lines with NO arrowhead, NO triangle, and NO decoration — just a clean straight or curved line. There must NEVER be two arrowheads on a single arrow line.
5. Every arrow must connect **exactly two boxes**. The arrow line must start from one box's edge and end at another box's edge. No arrows with one end dangling in empty space. No arrows that start or end at nothing.
6. Each arrow can have **at most ONE text label** inserted at ONE point in the middle of the line. This label should briefly describe the relationship (e.g., "calls", "reads", "triggers", "polls events"). Do NOT place multiple separate text labels along a single arrow. Do NOT insert text at more than one point on the same arrow.
7. **Solid arrows** represent "calls/invokes" control flow. **Dashed arrows** represent "reads/writes" data flow. Both types must follow the same single-directional rule (one arrowhead at destination end only).
8. When an arrow goes FROM a component inside one layer TO a component inside another layer, it must clearly exit the source box's edge and enter the target box's edge — do not cut through other boxes.

The diagram shows a 4-layer architecture of the GUI module for a Rust-like language parser, with the following layers from top to bottom:

**Layer 1 — Application Entry (blue, top, rounded rectangle with divider):**
- Header: "Application Entry"
- Body:
  - `main.cpp`: parses CLI arguments, detects `--gui` flag
  - `runGui(input_file)`: initializes window and enters main loop

**Layer 2 — Core Rendering Functions (green, rounded rectangle with divider):**
- Header: "Core Rendering Functions"
- Body:
  - `render_token_list()`: scrollable 6-column table via ImGui::BeginTable
  - `render_parse_tree()`: recursive tree rendering via ImGui::TreeNodeEx
  - `render_tree_node(node, ptr)`: leaf vs internal node helper
  - Main menu bar and About popup

**Layer 3 — Analysis Engine (orange, rounded rectangle with divider):**
- Header: "Analysis Engine"
- Body:
  - `perform_lex(filepath)`: calls Flex lexer, extracts tokens
  - `perform_parse()`: creates TokenStream, runs Parser
  - `read_file_content(filepath)`: reads source file
  - `show_file_dialog()`: native file open dialog

**Layer 4 — Global State (centered, rounded rectangle with dashed border, no divider):**
- Label: `AppState g_state`
- Fields listed inside:
  - `source_code` (string), `loaded_file_path` (string)
  - `tokens` (vector<Token>), `parse_tree` (unique_ptr<Node>)
  - `lex_success / parse_success` (bool), `lex_error / parse_error` (string)

**Layer 5 — Backend (purple, bottom, rounded rectangle with divider):**
- Header: "Rendering Backend"
- Body:
  - ImGui (core + Stdlib helper)
  - ImGui GLFW backend + ImGui OpenGL3 backend
  - GLFW3 (window, input) + OpenGL 3.0 (rendering)

**Data flow (dashed arrows, orange color):**
- Arrow FROM "Parse" button (in Layer 2) TO `perform_lex()` (Layer 3), labeled "triggers"
- Arrow FROM `perform_lex()` (Layer 3) TO `g_state.tokens` (Layer 4), labeled "fills"
- Arrow FROM `perform_parse()` (Layer 3) TO `g_state.parse_tree` (Layer 4), labeled "fills"
- Arrow FROM `g_state.tokens` (Layer 4) TO `render_token_list()` (Layer 2), labeled "reads"
- Arrow FROM `g_state.parse_tree` (Layer 4) TO `render_parse_tree()` (Layer 2), labeled "reads"

**Call flow (solid arrows, dark gray):**
- Arrow FROM `main()` (Layer 1) TO `runGui()` (Layer 1), labeled "calls"
- Arrow FROM `runGui()` (Layer 1) TO main loop components (Layer 2), labeled "invokes each frame"
- Arrow FROM rendering functions (Layer 2) TO ImGui (Layer 5), labeled "draws via"
- Arrow FROM ImGui (Layer 5) TO OpenGL (Layer 5), labeled "renders with"
- Arrow FROM GLFW (Layer 5) TO `runGui()` (Layer 1), labeled "polls events"

Layout: vertical stack of layers, with `AppState` shown as a central shared data store that layers 2 and 3 both reference. Keep the diagram portrait-oriented and well-spaced. Every arrow connects two concrete elements and is labeled.

---

## GUI 界面布局设计图

<!-- figure
id: 3
title: GUI 界面布局设计图
section: 4.2 界面布局设计
output: figures/gui_layout.png
size: 1536x1024
-->

### prompt

Generate a clean, professional UI wireframe/layout diagram (界面布局线框图) with white background, suitable for a LaTeX academic report. Use thin dark gray (#333333) borders for all regions, light gray (#F5F5F5) fills, with dark text labels. Style should be a flat wireframe/mockup — no real content, just labeled regions with icons and annotations. No gradients, no shadows, no 3D effects.

The diagram represents a desktop application window (1400×900 px) titled "Rust Language Parser - GUI", divided into the following regions:

**Top bar — Menu Bar (full width, ~30px height):**
- Left: "File" menu (with "Exit" item) | "Help" menu (with "About" item)
- Style: standard desktop menu bar

**Left panel — "Code Input" (40% width, full height minus menu and status bar):**
- Top section: a horizontal row with:
  - Label "File Path:" followed by a text input field (stretching)
  - Three buttons side by side: "Browse...", "Load", and "Save to Temp File"
- Middle section: Label "Source Code:" above a code editor area with:
  - Left gutter showing line numbers (1, 2, 3, ... in gray)
  - Right side: multiline text editing area
  - Show a few placeholder lines of code in monospace font, e.g.:
    ```
    fn main() -> i32 {
        let mut x: i32 = 10;
        return x;
    }
    ```
- Bottom section: a prominent green "Parse" button (150×35px)

**Right top panel — "Token Stream" (60% width, 50% of usable height):**
- Header text: "Total: XX tokens"
- A scrollable table with 6 columns and headers:
  - "Pos" | "Type" | "Category" | "Value" | "Line" | "Col"
- Show 3-4 example rows with placeholder data:
  - e.g., "1 | keyword | Fn | fn | 1 | 1"
  - e.g., "2 | identifier | Identifier | main | 1 | 4"
- Alternating row background colors (white / light gray)

**Right bottom panel — "Parse Tree" (60% width, 50% of usable height):**
- A scrollable tree view showing hierarchical parse tree nodes
- Show example tree structure with expand/collapse arrows:
  ```
  ▼ <Program>
    ▼ <Function>
      Fn: fn
      Identifier: main
      ...
    ▼ <Function>
      Fn: fn
      Identifier: add
      ...
  ```
- Leaf nodes shown with bullet points, internal nodes with expand arrows

**Bottom left — Status Bar (40% width, ~40px height):**
- Left-aligned status text
- Show two states with color coding:
  - Success (green text): "[OK] Lexical analysis succeeded (42 tokens)"
  - Error (red text): "[ERR] Cannot open file"

Add dimension annotations on the sides showing "40%" and "60%" width split, and "50% / 50%" height split on the right side. Use dashed lines to show the internal layout divisions.

---

## 递归下降分析流程图

<!-- figure
id: 4
title: 递归下降分析流程图
section: 5.2 递归下降分析法原理
output: figures/recursive_descent_flow.png
size: 1536x1024
-->

### prompt

Generate a clean, professional technical flowchart with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for process boxes, light green (#5CB85C) for output boxes, light orange (#F0AD4E) for input/tool boxes, light purple (#9B59B6) for the decision diamond, gray borders. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict box/node visual rules (you MUST follow these):
1. ALL boxes (input, process, output, tool) must use **rounded rectangles** with a **small uniform corner radius** (approximately 6px). Do NOT use parallelograms or other polygons — use rounded rectangles for everything, differentiated only by color.
2. Each box may optionally contain an internal **horizontal divider line** separating a header/title row from the body content. The divider line must be **thin (1px), solid, same color as the box border**.
3. If a box has a divider line, the header section above it should contain the box title in **bold text**, and the body below should list the details in regular-weight text. This pattern must be applied consistently to ALL boxes that contain multiple lines of information.
4. Boxes without dividers (simple single-concept nodes) should have centered text without any internal line.
5. All box borders must be **1.5px solid gray (#999999)**, consistent across every box regardless of color.

IMPORTANT — Strict arrow rules (you MUST follow these):
6. Every arrow must be **single-directional**: the arrowhead (triangular tip) appears ONLY at the destination end. The source end must have NO arrowhead, no decoration, just a plain line.
7. Every arrow must connect **two concrete visual elements** — it must originate FROM a specific shape (a rounded rectangle, a diamond) and point TO a specific shape. No arrows floating in empty space with one end dangling.
8. Every arrow must have a **short label** on or near it explaining its semantic meaning (e.g., "calls", "returns", "dispatches", "reads", "consumes", "emits"). No arrow should be unlabeled.
9. Decision branch arrows must be clearly separated and not overlap.

The diagram shows the overall workflow of a recursive descent parser for a Rust-like language, flowing top to bottom:

1. **Input (top, orange rounded rectangle with divider):**
   - Header: "TokenStream"
   - Body: "A stream of tokens produced by the lexer"
   - `peek()` for lookahead, `advance()` to consume

2. **Entry point (blue rounded rectangle with divider):**
   - Header: "`parseProgram()`"
   - Body: "Reads tokens in a loop, calling `parseFunction()` for each function definition until the stream ends"

3. **Function parsing (blue rounded rectangle with divider):**
   - Header: "`parseFunction()`"
   - Body: "Calls `parseFuncHeader()` then `parseBlock()`"
   - Arrow FROM this box TO parseFuncHeader and parseBlock boxes, labeled "calls"

4. **Statement dispatch (purple diamond, centered):**
   - Text inside diamond: "`parseStmt()` inspects current token"
   - Arrow FROM step 3 box TO this diamond, labeled "calls"
   - Branch arrows FROM the diamond TO each sub-parser box below, each labeled with the matching token:
     - Arrow labeled "`let`" → `parseLetStmt()` box
     - Arrow labeled "`return`" → `parseReturnStmt()` box
     - Arrow labeled "`if`" → `parseIfStmt()` box
     - Arrow labeled "`while`" → `parseWhileStmt()` box
     - Arrow labeled "`for`" → `parseForStmt()` box
     - Arrow labeled "`loop`" → `parseLoopStmt()` box
     - Arrow labeled "`break`" → `BreakStmt` box
     - Arrow labeled "`continue`" → `ContinueStmt` box
     - Arrow labeled "otherwise" → `parseExprOrAssignStmt()` box

5. **Expression parsing (blue rounded rectangle with divider):**
   - Header: "`parseExpr()`"
   - Body: "Layered precedence chain: parseExpr → parseCmpExpr → parseAddExpr → parseTerm → parseUnary → parseAtom"
   - Small note: "(see expression parsing call chain diagram)"

6. **Output (bottom, green rounded rectangle with divider):**
   - Header: "Parse Tree"
   - Body: "Node objects: `<Label>` for non-terminals, `Token: value` for leaf nodes"
   - Arrow FROM step 5 box TO this box, labeled "produces"

7. **Side element (left column, small blue boxes):**
   - A small box labeled "TokenStream API" with two items:
     - `ts.peek()` — lookahead (arrow FROM this item TO the main flow, labeled "reads")
     - `ts.advance()` — consume (arrow FROM this item TO the main flow, labeled "consumes")

Use downward-flowing arrows between steps. Keep the diagram portrait-oriented, well-spaced, with clear visual hierarchy. All arrows labeled. All boxes rounded rectangles with consistent styling.

---

## 语法树结构示意图

<!-- figure
id: 5
title: 语法树结构示意图
section: 5.3 Parser类实现
output: figures/parse_tree_structure.png
size: 1536x1024
-->

### prompt

Generate a clean, professional tree diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for non-terminal nodes (rounded rectangles), light orange (#F0AD4E) for leaf/terminal nodes (rounded rectangles with thinner border), gray connecting lines. No gradients, no shadows, no 3D effects.

The diagram shows a concrete parse tree for the following Rust-like code snippet:

```
if x > y {
    x = x + 1;
} else if x < y {
    x = x - 1;
} else {
    x = 0;
}
```

The tree structure is (top to bottom):

**Root:** `<IfStmt>`
  - `If: if` (leaf)
  - `<CmpExpr>`
    - `<Identifier>` → `Identifier: x` (leaf)
    - `Greater: >` (leaf)
    - `<Identifier>` → `Identifier: y` (leaf)
  - `<Block>`
    - `LBrace: {` (leaf)
    - `<AssignStmt>`
      - `<Identifier>` → `Identifier: x` (leaf)
      - `Assign: =` (leaf)
      - `<AddExpr>`
        - `<Identifier>` → `Identifier: x` (leaf)
        - `Plus: +` (leaf)
        - `<Literal>` → `IntegerConstant: 1` (leaf)
      - `Semicolon: ;` (leaf)
    - `RBrace: }` (leaf)
  - `<ElseClause>`
    - `Else: else` (leaf)
    - `<IfStmt>` (second if, show with "..." for brevity, indicating recursive nesting)
      - `If: if` (leaf)
      - `<CmpExpr>` (shown with ...)
      - `<Block>` (shown with ...)
      - `<ElseClause>` (shown with ...)
        - `Else: else` (leaf)
        - `<Block>` (shown with ...)

Use straight or slightly curved lines connecting parent to child nodes. Non-terminal nodes use `<label>` notation in blue boxes. Terminal/leaf nodes use `Category: value` notation in orange boxes. Show the tree growing top-down with clear indentation at each level. Keep the diagram portrait-oriented.

---

## 表达式运算符优先级层次图

<!-- figure
id: 6
title: 表达式运算符优先级层次图
section: 5.4 表达式解析与运算符优先级
output: figures/operator_precedence.png
size: 1024x1024
-->

### prompt

Generate a clean, professional layered hierarchy diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style with rounded rectangles. No gradients, no shadows, no 3D effects.

The diagram shows the 6-level operator precedence hierarchy used in a recursive descent parser for a Rust-like language, arranged as horizontal bands stacked vertically. Each band represents one precedence level, from lowest (top) to highest (bottom):

**Level 1 — Range (top band, light orange #F0AD4E):**
- Function: `parseExpr()`
- Operator: `..`
- Example: `1..10`, `start..end`
- Width: full band, thinner (represents lowest precedence, widest scope)

**Level 2 — Comparison (soft blue #4A90D9):**
- Function: `parseCmpExpr()`
- Operators: `==  !=  >  >=  <  <=`
- Example: `a > b`, `x == y`
- Band slightly narrower than Level 1

**Level 3 — Addition/Subtraction (soft green #5CB85C):**
- Function: `parseAddExpr()`
- Operators: `+  -`
- Example: `a + b`, `x - 1`
- Band narrower than Level 2

**Level 4 — Multiplication/Division (light purple #9B59B6):**
- Function: `parseTerm()`
- Operators: `*  /`
- Example: `a * b`, `x / 2`
- Band narrower than Level 3

**Level 5 — Unary (teal #17A2B8):**
- Function: `parseUnary()`
- Operators: `&  *  &mut` (prefix)
- Example: `&x`, `*ptr`, `&mut a`
- Band narrower than Level 4

**Level 6 — Atom (gray #6C757D):**
- Function: `parseAtom()`
- Operands: integer/bool constants, identifiers, `(expr)`, `[expr,...]`, `ID(args)`, `ID[index]`
- Band narrowest (highest precedence, tightest binding)

On the right side, add a vertical arrow labeled "Priority: Low → High" pointing downward. On the left side, label each band with its level number (1-6) and the parsing function name. Use downward arrows between bands to show that each level calls the next level down as its operand parser.

---

## 表达式解析调用链图

<!-- figure
id: 7
title: 表达式解析调用链图
section: 5.4 表达式解析与运算符优先级
output: figures/expression_call_chain.png
size: 1792x1024
-->

### prompt

Generate a clean, professional call chain / data flow diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for function boxes, light orange (#F0AD4E) for operator/decision labels, light green (#5CB85C) for result/output nodes. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict box/node visual rules (you MUST follow these):
1. ALL boxes must use **rounded rectangles** with a **small uniform corner radius** (approximately 6px). Differentiate box types ONLY by color, not by shape.
2. Each box may optionally contain an internal **horizontal divider line** (thin 1px, solid, same color as the box border) separating a header from the body. If present, header text is **bold**, body is regular weight. Apply consistently.
3. All box borders must be **1.5px solid gray (#999999)**, consistent across every box.

IMPORTANT — Strict arrow rules (you MUST follow these):
4. Every arrow must be **single-directional**: the arrowhead (triangular tip) appears ONLY at the destination end. The source end must have NO arrowhead — just a plain line leaving the source shape's edge.
5. Every arrow must connect **two concrete visual elements** (a box, a label node, a tree node). It must originate FROM a specific shape and point TO a specific shape. No arrows with one end dangling in empty space.
6. Every arrow must have a **short label** on or near it (e.g., "calls", "returns", "matches '+', calls again", "builds node"). No arrow should be unlabeled.
7. **Branch arrows**: when a function box has multiple outgoing arrows (e.g., a "calls" arrow going down to a child function AND a "returns" arrow going back up to the caller), each branch must have its own arrowhead at its own destination, and each must be independently labeled. Do NOT merge two branches into one bidirectional line.

The diagram illustrates how the expression `a + b * c` is parsed through the layered recursive descent parser. Layout: wide, landscape-oriented, divided into three sections.

**Left section — Call chain (vertical, top to bottom):**

Show nested function calls as indented rounded rectangle boxes connected by labeled downward arrows ("calls") and upward arrows ("returns"). Each box has a divider with function name in header and operator it handles in body:

```
Box: parseExpr() — handles: ".." range
  ↓ arrow labeled "calls"
Box: parseCmpExpr() — handles: == != > >= < <=
  ↓ arrow labeled "calls"
Box: parseAddExpr() — handles: + -
  ↓ arrow labeled "calls lhs"
Box: parseTerm() — handles: * /
  ↓ arrow labeled "calls"
Box: parseUnary() — handles: & * &mut
  ↓ arrow labeled "calls"
Box: parseAtom() — handles: literals, identifiers, (expr)
```

At `parseAtom()` the first call returns `<Identifier: a>`. Then:
- Arrow FROM parseAtom() back up TO parseTerm(), labeled "returns <Identifier: a>"
- Arrow FROM parseTerm() back up TO parseAddExpr(), labeled "returns <Identifier: a> (no * or /)"
- Decision label at parseAddExpr(): "check next token: matches '+'"
- Arrow FROM parseAddExpr() back down TO parseTerm() (second call), labeled "calls again (rhs after +)"
- Arrow FROM parseTerm() down TO parseUnary() → parseAtom(), labeled "calls"
- parseAtom() returns `<Identifier: b>` — arrow back up labeled "returns <Identifier: b>"
- Decision label at parseTerm(): "check next token: matches '*'"
- Arrow FROM parseTerm() down TO parseUnary() → parseAtom() again, labeled "calls again (rhs after *)"
- parseAtom() returns `<Identifier: c>` — arrow back up labeled "returns <Identifier: c>"
- Arrow FROM parseTerm() back up TO parseAddExpr(), labeled "returns <MulExpr: b * c>"
- Arrow FROM parseAddExpr() back up TO parseCmpExpr(), labeled "returns <AddExpr: a + (b * c)>"

**Right section — Resulting parse tree:**

A tree drawn with rounded rectangle nodes connected by lines (no arrowheads on tree edges, these are parent-child relationships not flow):

```
<AddExpr>
  ├── <Identifier: a>
  ├── Plus: +
  └── <MulExpr>
        ├── <Identifier: b>
        ├── Star: *
        └── <Identifier: c>
```

Color: blue (#4A90D9) for non-terminal nodes, orange (#F0AD4E) for leaf token nodes.

**Middle section — Connecting arrows:**

Dashed arrows FROM key call chain events (left) TO the corresponding tree nodes (right), showing which call builds which node:
- Arrow FROM "builds <MulExpr>" point (left) TO <MulExpr> node (right), labeled "constructs"
- Arrow FROM "builds <AddExpr>" point (left) TO <AddExpr> node (right), labeled "constructs"
Each dashed arrow has a single arrowhead at the tree node end.

Add a note at the bottom: "Higher-precedence operators (* /) are parsed deeper in the call chain, naturally binding tighter than lower-precedence operators (+ -)."

Ensure all arrows — call arrows, return arrows, branch arrows, and dashed connecting arrows — are single-directional, connect two concrete elements, and are clearly labeled.

---

## DFA 状态转换图

<!-- figure
id: 8
title: DFA 状态转换图（词法分析核心）
section: 3.1 词法分析理论基础
output: figures/dfa_state_transition.png
size: 1792x1024
-->

### prompt

Generate a clean, professional DFA state transition diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for normal state circles, light green (#5CB85C) with double circle for accepting states, light orange (#F0AD4E) for the start state arrow, gray border lines. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict DFA notation conventions (you MUST follow these):
1. Each state is drawn as a **circle** with the state name (e.g., S0, S1) centered inside.
2. The **initial/start state** must have a **short incoming arrow coming from nowhere** (an arrow that points to the start state circle but has no source node — it originates from empty space). This arrow should be drawn in orange (#F0AD4E) to visually distinguish it.
3. **Accepting/final states** must be drawn as **double circles** (a smaller circle drawn concentrically inside a larger circle, with a visible gap between them). All accepting states should be colored in green (#5CB85C).
4. **Transitions** are drawn as **single-direction arrows** connecting two state circles. The arrow line MUST start from the edge of the source circle and end at the edge of the target circle. The arrowhead (triangular tip) must appear ONLY at the destination end — the source end must have NO arrowhead, no decoration, just a plain line leaving the circle edge. This is a strict requirement: one arrow = one direction = one triangular tip.
5. Every single arrow (including self-loops and the start-state incoming arrow) must have a **transition condition label** written directly on or next to the arrow, clearly indicating what input symbol triggers the transition (e.g., "[a-zA-Z]", "[0-9]", "="). No arrow should be unlabeled.
6. For a DFA, each state has **at most one outgoing arrow per symbol** (deterministic). If multiple symbols lead to the same state, show them as a single arrow with a comma-separated label (e.g., "[a-zA-Z], _, [0-9]").
7. Self-loop transitions (state transitioning to itself) should be drawn as a **curved arrow looping back** to the same circle, with the transition condition label clearly written alongside the loop.
8. Use clean, well-spaced layout — do not overcrowd states or arrows. Ensure arrow labels are fully legible and do not overlap.

The diagram shows three related DFA automata side by side, illustrating how the lexer recognizes different token types for a Rust-like language:

**DFA 1 (left) — Identifier / Keyword recognition:**
- State S0 (start): on letter `[a-zA-Z]` or `_` → State S1
- State S1: on letter `[a-zA-Z]`, digit `[0-9]`, or `_` → stay in S1
- State S1 (accepting, double circle): on any other character → emit token, then check if matched string is a keyword (fn, let, if, else, while, for, in, loop, break, continue, return, mut, i32) → emit KEYWORD token; otherwise → emit IDENTIFIER token
- Show a small table below this DFA listing all keywords: fn, let, if, else, while, for, in, loop, break, continue, return, mut, i32

**DFA 2 (center) — Integer Constant recognition:**
- State T0 (start): on `0` → State T1; on `[1-9]` → State T2
- State T1: on `[bB]` → State T3 (binary); on `[xX]` → State T4 (hex); on `[0-7]` → State T5 (octal); on non-digit → accepting (value = 0)
- State T2: on `[0-9]` or `_` → stay T2; on non-digit → accepting (decimal)
- State T3: on `[01]` or `[01]` with `_` → stay T3; on non-binary → accepting (binary)
- State T4: on `[0-9a-fA-F]` → stay T4; on non-hex → accepting (hex)
- State T5: on `[0-7]` → stay T5; on non-octal → accepting (octal)
- All accepting states shown as double circles in green

**DFA 3 (right) — Multi-character Operator recognition (e.g., ==, !=, >=, <=, ->, ..):**
- State U0 (start): on `=` → State U1; on `!` → State U2; on `>` → State U3; on `<` → State U4; on `-` → State U5; on `.` → State U6
- State U1: on `=` → State U1a (accepting, emit `==`); else → accepting (emit `=`)
- State U2: on `=` → State U2a (accepting, emit `!=`)
- State U3: on `=` → State U3a (accepting, emit `>=`); else → accepting (emit `>`)
- State U4: on `=` → State U4a (accepting, emit `<=`); else → accepting (emit `<`)
- State U5: on `>` → State U5a (accepting, emit `->`); else → accepting (emit `-`)
- State U6: on `.` → State U6a (accepting, emit `..`); else → accepting (emit `.`)
- Show the "else → accepting" transitions as small arrows exiting the state to the right

Add a title at the top: "DFA State Transition Diagrams for Lexical Analysis". Use clear, well-spaced layout with the three DFAs arranged horizontally.

---

## 系统架构图

<!-- figure
id: 9
title: 系统架构图
section: 1.3 系统总体架构
output: figures/system_architecture.png
size: 1536x1024
-->

### prompt

Generate a clean, professional layered system architecture diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style with rounded rectangles for modules. Color scheme: soft blue (#4A90D9) for analysis modules, light green (#5CB85C) for input/output, light orange (#F0AD4E) for data/interface, light purple (#9B59B6) for GUI module, gray for build system. No gradients, no shadows, no 3D effects.

The diagram shows the overall architecture of a Rust-like language parser system, arranged as a top-to-bottom data flow pipeline with a side module:

**Top (input, green):**
- Source code file `.rs` (user input)

**Layer 1 — Lexical Analysis (blue):**
- Box: "Lexical Analyzer (Flex-generated)"
- Contains: `lexer.l` rules → Flex → `lex.yy.cpp`
- Sub-labels: keyword recognition, operator matching, constant parsing, comment filtering
- Left annotation: "Regular expressions → DFA"

**Interface Layer (orange):**
- Box: "TokenStream"
- Acts as a buffer between lexer and parser
- Shows the cursor-based API: `peek()`, `advance()`, `check()`, `expect()`
- Data: ordered list of Token objects `{type, category, value, pos, lineno, colno}`

**Layer 2 — Syntax Analysis (blue):**
- Box: "Recursive Descent Parser"
- Contains: `parseProgram()` → `parseFunction()` → `parseStmt()` → `parseExpr()`
- Sub-labels: statement dispatch, expression precedence chain, error recovery
- Right annotation: "LL(1) grammar, top-down"

**Bottom (output, green):**
- Parse Tree: shown as a small tree icon with `<Program>` root
- Node structure: `label`, `children[]`, `isLeaf`

**Side Module (purple, right side, spanning Layer 1–2):**
- Box: "GUI Visualization Module (ImGui + GLFW + OpenGL)"
- Top part: Code editor with syntax highlighting
- Middle: Token table viewer
- Bottom: Parse tree viewer with expand/collapse
- Arrow from TokenStream and Parse Tree into GUI box showing it reads both

**Side Module (gray, far right):**
- Small box: "Build System (CMake + MinGW)"
- Labels: Linux build / Windows cross-compile

Use thick downward arrows between layers showing data flow direction. Label each arrow with what data passes through (e.g., "character stream", "Token sequence", "Parse Tree"). Keep the diagram portrait-oriented.

---

## Flex 工作流程图

<!-- figure
id: 10
title: Flex 工作流程图
section: 3.2 Flex工具介绍
output: figures/flex_workflow.png
size: 1792x1024
-->

### prompt

Generate a clean, professional technical flowchart with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style. Color scheme: soft blue (#4A90D9) for process/transform steps, light green (#5CB85C) for output files, light orange (#F0AD4E) for input files, gray borders. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict box/node visual rules (you MUST follow these):
1. ALL boxes (process, file, I/O) must use **rounded rectangles** with a **small uniform corner radius** (approximately 6px). Do NOT use document-shaped polygons or parallelograms — use rounded rectangles for everything, differentiated only by color.
2. Each box may optionally contain an internal **horizontal divider line** separating a header/title row from the body content. The divider line must be **thin (1px), solid, same color as the box border**.
3. If a box has a divider line, the header section above it should contain the box title in **bold text**, and the body below should list the details in regular-weight text. This pattern must be applied consistently to ALL boxes that contain multiple lines of information.
4. Boxes without dividers (simple single-concept nodes) should have centered text without any internal line.
5. All box borders must be **1.5px solid gray (#999999)**, consistent across every box regardless of color.

The diagram shows the complete Flex-based lexer development pipeline, flowing left to right in 5 stages:

**Stage 1 — Input (orange, rounded rectangle with divider):**
- Header: `lexer.l`
- Body: three bullet lines:
  - Definition section (`%{...%}`): C includes, global variables
  - Rules section (`%%...%%`): pattern-action pairs (regex → C code)
  - User code section: `lex()` wrapper function

**Stage 2 — Flex Processing (blue, rounded rectangle with divider):**
- Header: "Flex 2.6.4 Compiler"
- Body:
  - Action: reads `lexer.l`, converts regex patterns to DFA state tables, generates C code
  - Command: `flex -o lex.yy.cpp lexer.l`
  - Small inline note: "Regex → NFA → DFA → C state machine"

**Stage 3 — Generated Output (green, rounded rectangle with divider):**
- Header: `lex.yy.cpp`
- Body:
  - `yylex()` function (state machine)
  - Token recognition logic
  - Input buffer management

**Stage 4 — Compilation (blue, rounded rectangle with divider):**
- Header: "C++ Compiler (CMake)"
- Body:
  - GCC/G++ with `-std=c++17`, links with `main.cpp`, `TokenStream.cpp`, `Type.cpp`
  - Command: `cmake .. && make -j4`
  - Output: `parser` executable

**Stage 5 — Runtime (green + orange, two boxes):**
- Box A (orange, with divider):
  - Header: `input.rs`
  - Body: source code characters fed into the lexer
- Box B (green, with divider):
  - Header: "TokenStream Output"
  - Body:
    - `yylex()` reads characters, matches patterns via DFA
    - Emits Token objects: `{type, category, value, pos, lineno, colno}`
    - Example: `"fn"` → `Token(KEYWORD, Fn, "fn", 1,1,1)`, `"main"` → `Token(IDENTIFIER, Identifier, "main", 2,1,4)`

Use thick arrows between stages labeled with the data being passed. Keep the diagram landscape-oriented with clear stage separation. Every arrow label should be in small italic text.

---

## TokenStream 工作原理图

<!-- figure
id: 11
title: TokenStream 工作原理图
section: 4.1 TokenStream接口设计
output: figures/tokenstream_mechanism.png
size: 1792x1280
-->

### prompt

Generate a clean, professional technical illustration diagram with white background and dark text, suitable for a LaTeX academic report. Use a modern flat style with rounded rectangles. Color scheme: soft blue (#4A90D9) for the TokenStream object, light green (#5CB85C) for the cursor pointer, light orange (#F0AD4E) for operation results, light purple (#9B59B6) for Token objects. No gradients, no shadows, no 3D effects.

IMPORTANT — Strict arrow rules (you MUST follow these):
1. Every arrow in this diagram must be **single-directional**: the arrowhead (triangular tip) appears ONLY at the destination end. The source end must have NO arrowhead, no decoration, just a plain line.
2. Every arrow must connect **two concrete visual elements** — it must originate FROM a specific shape (a box, a circle, a diamond, a cursor label) and point TO a specific shape. No arrows floating in empty space with one end dangling.
3. Every arrow must have a **short label** on or near it explaining its semantic meaning (e.g., "returns", "moves to", "checks", "throws error", "reads", "matches"). No arrow should be unlabeled.
4. Arrows indicating data flow should be colored differently from arrows indicating control flow if both appear in the same area.

The diagram illustrates the cursor-based access mechanism of a TokenStream object that serves as the interface between a lexer and a parser. Layout: portrait-oriented (tall), centered on the TokenStream structure.

**Top — Context (left box → arrow → TokenStream → arrow → right box):**
- Left box (orange, rounded rectangle): "Lexer" with subtitle "`yylex()` repeatedly"
  - Arrow FROM this box TO the TokenStream box, labeled "emits Tokens"
- Center box (blue, rounded rectangle): "TokenStream" with fields:
  - `vector<Token> toks` — the token array
  - `int cur` — current cursor position (highlighted)
- Right box (purple, rounded rectangle): "Parser" with subtitle "`parseProgram()` entry"
  - Arrow FROM TokenStream box TO this box, labeled "provides tokens via API"

**Middle — Token Array visualization (centered, the visual centerpiece):**

Show an array/table of Token objects as a row of small rounded rectangles:

| Index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|-------|---|---|---|---|---|---|---|---|---|
| Token | Fn | main | ( | ) | -> | i32 | { | let | mut |

Each box is a small rounded rectangle with the token value inside. The currently pointed token (index 0, "Fn") is highlighted with a green (#5CB85C) border.

Below the array, show a green cursor indicator: a downward arrow FROM a label `cur = 0` pointing TO the box at index 0. The arrow originates FROM the label and points TO the token box edge.

**Bottom — Three operation demonstrations (side by side):**

Show three mini-diagrams side by side, each demonstrating one core API operation. Each mini-diagram has its own token array snapshot and arrows with clear source → destination:

**Operation 1: `peek()` (left):**
- A labeled box "peek()" at the top
- Arrow FROM "peek()" box TO the token at cur position, labeled "reads"
- A curved return arrow FROM the token back TO a result label "→ Token(Fn)", labeled "returns"
- The cursor label stays at `cur = 0` — annotate: "cursor does NOT move"
- Annotation: "Lookahead — does NOT consume"

**Operation 2: `advance()` (center):**
- A labeled box "advance()" at the top
- Arrow FROM "advance()" box TO the token at cur position, labeled "reads"
- A curved return arrow FROM the token back TO a result label "→ Token(Fn)", labeled "returns"
- A second green arrow FROM the old cursor position TO the new position (0 → 1), labeled "cur increments"
- Before state: `cur = 0`, After state: `cur = 1`
- Annotation: "Consumes — cursor moves forward"

**Operation 3: `check('Identifier')" (right):**
- A labeled box "check('Identifier')" at the top
- Arrow FROM "check()" box TO the token at cur position, labeled "inspects category"
- A diamond decision shape below: "category == 'Identifier'?"
- Arrow FROM token TO diamond, labeled "compares"
- Arrow FROM diamond (Yes branch) TO label "true", labeled "matches"
- Arrow FROM diamond (No branch) TO label "false", labeled "not match"
- In this case current token is "Fn" (category = "Fn"), so the "false" arrow is highlighted
- Annotation: "Predicate — does NOT consume"

**Bottom edge — Two additional operations (compact row):**

Two smaller rounded rectangles side by side:

**`expect(cat, ctx)`:** Arrow FROM a box labeled "expect()" TO the token, then a diamond "matches?". Arrow FROM diamond (Yes) TO "advance() + return token". Arrow FROM diamond (No) TO a red error box "throw runtime_error".

**`atEnd()`:** Arrow FROM a box labeled "atEnd()" TO the end of the token array. Diamond "cur >= size?". Arrow FROM diamond (Yes) TO "return true". Arrow FROM diamond (No) TO "return false".

Use clear, well-spaced layout with generous vertical spacing between sections.
