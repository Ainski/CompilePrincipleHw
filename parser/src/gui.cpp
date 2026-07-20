#ifdef WINDOWS_BUILD
#include "../include/gui.h"
#include "../include/TokenStream.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/SemanticAnalyzer.h"
#include "../include/IRGenerator.h"
#include "../include/logprintf.h"

// ImGui core headers
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

// OpenGL and GLFW



#include <GLFW/glfw3.h>
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <commdlg.h>
    // Windows headers define ERROR which conflicts with LogLevel::ERROR
    #ifdef ERROR
        #undef ERROR
    #endif


#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>

using namespace std;

// ============================================================
//  Global State
// ============================================================
struct AppState {
    string source_code;              // Source code input by user
    string loaded_file_path;         // Loaded file path
    bool parse_triggered = false;    // Whether parsing was triggered

    // Lexical analysis results
    vector<Token> tokens;            // Token list
    bool lex_success = false;
    string lex_error;

    // Parsing results
    unique_ptr<Node> parse_tree;     // Parse tree
    bool parse_success = false;
    string parse_error;

    // Intermediate code results
    vector<Quadruple> ir_code;       // Generated quadruples
    bool ir_success = false;
    string ir_error;

    // Right panel view: 0=TokenStream, 1=ParseTree, 2=IntermediateCode
    int active_right_view = 0;

    // File path buffer
    char file_path_buf[1024] = "";
};

static AppState g_state;

// ============================================================
//  Helper: Show file open dialog (Windows only)
// ============================================================
#ifdef WINDOWS_BUILD
static string show_file_dialog() {
    OPENFILENAMEA ofn;
    char szFile[1024] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Rust Files (*.rs)\0*.rs\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return string(szFile);
    }
    return "";
}
#endif

// ============================================================
//  Helper: Read file content
// ============================================================
static string read_file_content(const string& filepath) {
    ifstream ifs(filepath);
    if (!ifs) {
        throw runtime_error("Cannot open file: " + filepath);
    }
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// ============================================================
//  Helper: Perform lexical analysis
// ============================================================
static void perform_lex(const string& filepath) {
    g_state.lex_success = false;
    g_state.lex_error.clear();
    g_state.tokens.clear();

    try {
        log(LogLevel::INFO, "Performing lexical analysis on file: " + filepath);
        TokenStream ts = lex(filepath);

        // Extract all tokens
        while (!ts.atEnd()) {
            const Token& tok = ts.peek();
            g_state.tokens.push_back(tok);
            ts.advance();
        }

        g_state.lex_success = true;
        log(LogLevel::INFO, "Lexical analysis succeeded, " + to_string(g_state.tokens.size()) + " tokens");
    } catch (const exception& e) {
        g_state.lex_success = false;
        g_state.lex_error = e.what();
        log(LogLevel::ERROR, string("Lexical analysis failed: ") + e.what());
    }
}

// ============================================================
//  Helper: Perform parsing
// ============================================================
static void perform_parse() {
    g_state.parse_success = false;
    g_state.parse_error.clear();
    g_state.parse_tree.reset();

    if (!g_state.lex_success || g_state.tokens.empty()) {
        g_state.parse_error = "Lexical analysis failed, cannot parse";
        log(LogLevel::ERROR, "Lexical analysis not successful");
        return;
    }

    try {
        // Recreate TokenStream
        TokenStream ts = lex(g_state.loaded_file_path);
        ts.remove_comments();

        Parser parser(ts);
        g_state.parse_tree = parser.parse();

        g_state.parse_success = true;
        log(LogLevel::INFO, "Parsing succeeded");
    } catch (const exception& e) {
        g_state.parse_success = false;
        g_state.parse_error = e.what();
        log(LogLevel::ERROR, string("Parsing failed: ") + e.what());
    }
}

// ============================================================
//  Helper: Perform semantic analysis + IR generation
// ============================================================
static void perform_ir() {
    g_state.ir_success = false;
    g_state.ir_error.clear();
    g_state.ir_code.clear();

    if (!g_state.parse_success || !g_state.parse_tree) {
        g_state.ir_error = "Parse failed, cannot generate IR";
        log(LogLevel::ERROR, "Parse not successful, cannot generate IR");
        return;
    }

    try {
        // Run semantic analysis first
        SemanticAnalyzer analyzer;
        analyzer.analyze(g_state.parse_tree.get());

        if (analyzer.hasErrors()) {
            stringstream ss;
            analyzer.printErrors(ss);
            g_state.ir_error = "Semantic errors:\n" + ss.str();
            g_state.ir_success = false;
            log(LogLevel::WARN, "Semantic analysis found errors, IR may be incomplete");
            // Continue anyway to show partial IR
        }

        // Generate IR
        IRGenerator irgen;
        irgen.generate(g_state.parse_tree.get());
        g_state.ir_code = irgen.getIR();

        g_state.ir_success = true;
        log(LogLevel::INFO, "IR generation succeeded, " + to_string(g_state.ir_code.size()) + " quadruples");
    } catch (const exception& e) {
        g_state.ir_success = false;
        g_state.ir_error = e.what();
        log(LogLevel::ERROR, string("IR generation failed: ") + e.what());
    }
}

// ============================================================
//  Render parse tree (recursive helper)
// ============================================================
static void render_tree_node(const Node& node, const void* node_ptr) {
    if (node.isLeaf) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
        // Use pointer address as unique ID to avoid conflicts with same labels
        std::string id = node.label + "##" + std::to_string(reinterpret_cast<uintptr_t>(node_ptr));
        ImGui::TreeNodeEx(id.c_str(), flags);
    } else {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        // Use pointer address as unique ID to avoid conflicts with same labels
        std::string id = node.label + "##" + std::to_string(reinterpret_cast<uintptr_t>(node_ptr));
        bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);
        if (node_open) {
            for (const auto& child : node.children) {
                render_tree_node(*child, child.get());
            }
            ImGui::TreePop();
        }
    }
}

// ============================================================
//  Main GUI function
// ============================================================
void runGui(const string& input_file) {
    log(LogLevel::INFO, "Launching GUI...");

    // Initialize GLFW
    if (!glfwInit()) {
        log(LogLevel::ERROR, "GLFW initialization failed");
        return;
    }

    // Create window
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1400, 900, "Rust Language Parser - GUI", nullptr, nullptr);
    if (!window) {
        log(LogLevel::ERROR, "Window creation failed");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.0f;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // If input file is provided, load it
    if (!input_file.empty()) {
        try {
            g_state.source_code = read_file_content(input_file);
            g_state.loaded_file_path = input_file;
            snprintf(g_state.file_path_buf, sizeof(g_state.file_path_buf), "%s", input_file.c_str());
        } catch (const exception& e) {
            log(LogLevel::WARN, string("Cannot load initial file: ") + e.what());
        }
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get current viewport size (updates dynamically on resize)
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float menu_bar_height = ImGui::GetFontSize() + 5.0f;
        float status_bar_height = 40.0f;
        float usable_height = viewport->Size.y - menu_bar_height - status_bar_height;

        // ============================================================
        //  Top: Menu bar
        // ============================================================
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    ImGui::OpenPopup("About");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // About popup
        if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Rust Language Parser GUI");
            ImGui::Separator();
            ImGui::Text("Version: 1.0");
            ImGui::Text("Based on Dear ImGui and Flex");
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // ============================================================
        //  Left panel: Code input (40% width)
        // ============================================================
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menu_bar_height));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.4f, usable_height));

        ImGui::Begin("Code Input", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // File path input
        ImGui::Text("File Path:");
        ImGui::SetNextItemWidth(-200);
        ImGui::InputText("##filepath", g_state.file_path_buf, sizeof(g_state.file_path_buf));
        ImGui::SameLine();

        if (ImGui::Button("Browse...")) {
#ifdef WINDOWS_BUILD
            string selected_path = show_file_dialog();
            if (!selected_path.empty()) {
                // Update file path buffer
                snprintf(g_state.file_path_buf, sizeof(g_state.file_path_buf), "%s", selected_path.c_str());
                
                // Load the file
                try {
                    g_state.source_code = read_file_content(selected_path);
                    g_state.loaded_file_path = selected_path;
                    g_state.parse_triggered = false;
                    g_state.lex_success = false;
                    g_state.parse_success = false;
                    log(LogLevel::INFO, "File loaded: " + selected_path);
                } catch (const exception& e) {
                    log(LogLevel::ERROR, string("Failed to load file: ") + e.what());
                }
            }
#else
            log(LogLevel::WARN, "File dialog not supported on this platform. Please enter path manually.");
#endif
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Load")) {
            string path(g_state.file_path_buf);
            if (!path.empty()) {
                try {
                    g_state.source_code = read_file_content(path);
                    g_state.loaded_file_path = path;
                    g_state.parse_triggered = false;
                    g_state.lex_success = false;
                    g_state.parse_success = false;
                    log(LogLevel::INFO, "File loaded: " + path);
                } catch (const exception& e) {
                    log(LogLevel::ERROR, string("Failed to load file: ") + e.what());
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Source Code:");

        // Code editor with line numbers
        {
            ImVec2 editor_size = ImVec2(-1.0f, -80.0f);
            
            // Count lines
            int line_count = 1;
            for (char c : g_state.source_code) {
                if (c == '\n') line_count++;
            }
            
            // Calculate line number width
            char line_num_buf[32];
            snprintf(line_num_buf, sizeof(line_num_buf), "%d", line_count);
            float line_num_width = ImGui::CalcTextSize(line_num_buf).x + 20.0f;
            
            // Create two child regions side by side
            ImGui::BeginChild("CodeEditor", editor_size, true);

            // Line numbers panel
            ImGui::BeginChild("LineNumbers", ImVec2(line_num_width, 0), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            for (int i = 1; i <= line_count; i++) {
                snprintf(line_num_buf, sizeof(line_num_buf), "%d", i);
                ImGui::Text("%*s", (int)strlen(line_num_buf), line_num_buf);
                ImGui::Dummy(ImVec2(0.0f, 1.0f)); // Small spacing between line numbers
            }
            ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::SameLine();

            // Code input area
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
            ImGui::InputTextMultiline(
                "##source",
                &g_state.source_code,
                ImVec2(-1.0f, -1.0f),
                ImGuiInputTextFlags_AllowTabInput
            );
            ImGui::PopStyleVar();
            
            ImGui::EndChild();
        }

        // Button row
        if (ImGui::Button("Save to Temp File")) {
            string temp_file = "temp_input.rs";
            ofstream ofs(temp_file);
            if (ofs) {
                ofs << g_state.source_code;
                ofs.close();
                g_state.loaded_file_path = temp_file;
                snprintf(g_state.file_path_buf, sizeof(g_state.file_path_buf), "%s", temp_file.c_str());
                log(LogLevel::INFO, "Saved to temp file: " + temp_file);
            }
        }

        ImGui::SameLine();

        // Parse button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        if (ImGui::Button("Parse", ImVec2(150, 35))) {
            if (g_state.source_code.length() > 0) {
                // Ensure file is saved
                if (g_state.loaded_file_path.empty()) {
                    g_state.loaded_file_path = "temp_input.rs";
                    snprintf(g_state.file_path_buf, sizeof(g_state.file_path_buf), "%s", g_state.loaded_file_path.c_str());
                }

                ofstream ofs(g_state.loaded_file_path);
                if (ofs) {
                    ofs << g_state.source_code;
                    ofs.close();

                    // Perform lexical analysis
                    perform_lex(g_state.loaded_file_path);

                    // If lexical analysis succeeds, perform parsing
                    if (g_state.lex_success) {
                        perform_parse();
                        // If parsing succeeds, run semantic analysis + IR generation
                        if (g_state.parse_success) {
                            perform_ir();
                        }
                    }
                    g_state.parse_triggered = true;
                } else {
                    g_state.lex_error = "Cannot write file: " + g_state.loaded_file_path;
                    g_state.lex_success = false;
                }
            } else {
                g_state.lex_error = "Please enter code or load a file";
                g_state.lex_success = false;
            }
        }
        ImGui::PopStyleColor(3);

        ImGui::End();

        // ============================================================
        //  Right panel: View selector + selected view (60% width)
        // ============================================================
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.4f, viewport->Pos.y + menu_bar_height));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.6f, usable_height));

        ImGui::Begin("Analysis Results", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // View toggle buttons
        {
            ImGui::PushStyleColor(ImGuiCol_Button, g_state.active_right_view == 0
                ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Token Stream", ImVec2(130, 25))) {
                g_state.active_right_view = 0;
            }
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, g_state.active_right_view == 1
                ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Parse Tree", ImVec2(130, 25))) {
                g_state.active_right_view = 1;
            }
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, g_state.active_right_view == 2
                ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Intermediate Code", ImVec2(160, 25))) {
                g_state.active_right_view = 2;
            }
            ImGui::PopStyleColor();
        }

        ImGui::Separator();

        // Show selected view in remaining area
        float remaining_height = ImGui::GetContentRegionAvail().y;
        ImGui::BeginChild("ViewContent", ImVec2(0, remaining_height > 0 ? remaining_height : 0), true);

        switch (g_state.active_right_view) {
            case 0: {
                // Token Stream view (inline)
                ImGui::Text("Total: %zu tokens", g_state.tokens.size());
                ImGui::Separator();
                if (ImGui::BeginTable("Tokens", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, 0))) {
                    ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80);
                    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 130);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 200);
                    ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableSetupColumn("Col", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < g_state.tokens.size(); i++) {
                        const auto& tok = g_state.tokens[i];
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%d", tok.pos);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", typeToString(tok.type).c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", tok.category.c_str());
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%s", tok.value.c_str());
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%d", tok.lineno);
                        ImGui::TableSetColumnIndex(5);
                        ImGui::Text("%d", tok.colno);
                    }
                    ImGui::EndTable();
                }
                break;
            }
            case 1: {
                // Parse Tree view (inline)
                if (g_state.parse_tree) {
                    render_tree_node(*g_state.parse_tree, g_state.parse_tree.get());
                } else {
                    ImGui::Text("Parse tree is empty");
                }
                break;
            }
            case 2: {
                // Intermediate Code view (inline)
                ImGui::Text("Total: %zu quadruples", g_state.ir_code.size());
                ImGui::Separator();
                if (ImGui::BeginTable("IR", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, 0))) {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
                    ImGui::TableSetupColumn("Op", ImGuiTableColumnFlags_WidthFixed, 110);
                    ImGui::TableSetupColumn("Arg1", ImGuiTableColumnFlags_WidthStretch, 150);
                    ImGui::TableSetupColumn("Arg2", ImGuiTableColumnFlags_WidthStretch, 150);
                    ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_WidthStretch, 150);
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < g_state.ir_code.size(); i++) {
                        const auto& q = g_state.ir_code[i];
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%zu", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", irOpToString(q.op).c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", q.arg1.empty() ? "-" : q.arg1.c_str());
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%s", q.arg2.empty() ? "-" : q.arg2.c_str());
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%s", q.result.empty() ? "-" : q.result.c_str());
                    }
                    ImGui::EndTable();
                }
                break;
            }
        }

        ImGui::EndChild();
        ImGui::End();

        // ============================================================
        //  Bottom status bar
        // ============================================================
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - status_bar_height));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, status_bar_height));

        ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        if (g_state.lex_success) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[OK] Lex: %zu tokens", g_state.tokens.size());
        } else if (!g_state.lex_error.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "[ERR] Lex: %s", g_state.lex_error.c_str());
        }

        ImGui::SameLine(200);

        if (g_state.parse_success) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[OK] Parse");
        } else if (!g_state.parse_error.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "[ERR] Parse: %s", g_state.parse_error.c_str());
        }

        ImGui::SameLine(400);

        if (g_state.ir_success) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[OK] IR: %zu quads", g_state.ir_code.size());
        } else if (!g_state.ir_error.empty()) {
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "[WARN] IR: %s", g_state.ir_error.c_str());
        }

        ImGui::End();

        // ============================================================
        //  Render
        // ============================================================
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    log(LogLevel::INFO, "GUI closed");
}
#endif