// ============================================================
//  Recursive Descent Parser for Rust-like language
//  Input : TSV token stream produced by the flex lexer
//          (stdin or first command-line argument)
//  Output: Indented parse tree  →  stdout
//          Error messages        →  stderr
//
//  Build : g++ -std=c++17 -o parser parser.cpp
//  Run   : ./parser  ../flex/output.tsv
//          ./flex/clex < ./flex/input.rs | ./parser/parser
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include "../include/TokenStream.h"
#include "../include/lexer.h"
using namespace std;


// ============================================================
//  Parse-tree node
// ============================================================
struct Node {
    string label;
    vector<unique_ptr<Node>> children;
    bool isLeaf = false;

    explicit Node(string lbl, bool leaf = false)
        : label(move(lbl)), isLeaf(leaf) {}
};

using NodePtr = unique_ptr<Node>;

static NodePtr makeNode(const string& lbl) {
    return make_unique<Node>(lbl);
}
static NodePtr makeLeaf(const string& lbl) {
    return make_unique<Node>(lbl, true);
}
static void addChild(NodePtr& parent, NodePtr child) {
    parent->children.push_back(move(child));
}

// Indented print
static void printTree(const Node& n, int depth = 0) {
    string indent(depth * 2, ' ');
    if (n.isLeaf) {
        cout << indent << n.label << "\n";
    } else {
        cout << indent << "<" << n.label << ">\n";
        for (auto& c : n.children) printTree(*c, depth + 1);
    }
}

// ============================================================
//  Parser
// ============================================================
class Parser {
    TokenStream& ts;

    // Helper: consume a token and make a leaf node
    NodePtr consumeLeaf() {
        Token t = ts.advance();
        return makeLeaf(t.category + ": " + t.value);
    }

    NodePtr expectLeaf(const string& cat, const string& ctx = "") {
        Token t = ts.expect(cat, ctx);
        return makeLeaf(t.category + ": " + t.value);
    }

public:
    explicit Parser(TokenStream& ts) : ts(ts) {}

    NodePtr parse() {
        NodePtr root = parseProgram();
        if (ts.check("End")) ts.advance();
        if (!ts.atEnd()) {
            const Token& t = ts.peek();
            throw runtime_error(
                "Unexpected token after program end: '" + t.category +
                "' \"" + t.value + "\"");
        }
        return root;
    }

    // ----------------------------------------------------------
    //  1.1  Program -> { Function }
    // ----------------------------------------------------------
    NodePtr parseProgram() {
        auto node = makeNode("Program");
        while (!ts.atEnd()) {
            addChild(node, parseFunction());
        }
        return node;
    }

    // ----------------------------------------------------------
    //  Function -> FuncHeader Block
    // ----------------------------------------------------------
    NodePtr parseFunction() {
        auto node = makeNode("Function");
        addChild(node, parseFuncHeader());
        addChild(node, parseBlock());
        return node;
    }

    // ----------------------------------------------------------
    //  1.1 / 1.5  FuncHeader -> fn ID '(' ParamList ')' ['->' Type]
    // ----------------------------------------------------------
    NodePtr parseFuncHeader() {
        auto node = makeNode("FuncHeader");
        addChild(node, expectLeaf("Fn",         "fn keyword"));
        addChild(node, expectLeaf("Identifier", "function name"));
        addChild(node, expectLeaf("LParen",     "open paren"));
        addChild(node, parseParamList());
        addChild(node, expectLeaf("RParen",     "close paren"));
        if (ts.check("Arrow")) {                 // 1.5: return type
            addChild(node, consumeLeaf());
            addChild(node, parseType());
        }
        return node;
    }

    // ----------------------------------------------------------
    //  1.4  ParamList -> ε | Param {',' Param}
    // ----------------------------------------------------------
    NodePtr parseParamList() {
        auto node = makeNode("ParamList");
        if (ts.check("RParen")) return node;     // ε
        addChild(node, parseParam());
        while (ts.check("Comma")) {
            addChild(node, consumeLeaf());       // ','
            addChild(node, parseParam());
        }
        return node;
    }

    // Param -> [mut] ID ':' Type
    NodePtr parseParam() {
        auto node = makeNode("Param");
        if (ts.check("Mut")) addChild(node, consumeLeaf());  // 0.1 qualifier
        addChild(node, expectLeaf("Identifier", "parameter name"));
        addChild(node, expectLeaf("Colon",      "':'"));
        addChild(node, parseType());
        return node;
    }

    // ----------------------------------------------------------
    //  0.2  Type -> i32 | ID | '[' Type ';' NUM ']' | '&' [mut] Type
    // ----------------------------------------------------------
    NodePtr parseType() {
        auto node = makeNode("Type");
        if (ts.check("I32")) {
            addChild(node, consumeLeaf());
        } else if (ts.check("Identifier")) {     // bool, usize, etc.
            addChild(node, consumeLeaf());
        } else if (ts.check("LBracket")) {       // [i32; 3]
            addChild(node, consumeLeaf());        // '['
            addChild(node, parseType());
            addChild(node, expectLeaf("Semicolon", "';' in array type"));
            // size constant
            if (ts.checkType("constant"))
                addChild(node, consumeLeaf());
            else
                addChild(node, expectLeaf("IntegerConstant", "array size"));
            addChild(node, expectLeaf("RBracket", "']'"));
        } else if (ts.check("Ampersand")) {      // &T or &mut T
            addChild(node, consumeLeaf());
            if (ts.check("Mut")) addChild(node, consumeLeaf());
            addChild(node, parseType());
        } else if (ts.check("LParen")) {         // (T, T, ...)
            addChild(node, consumeLeaf());
            if (!ts.check("RParen")) {
                addChild(node, parseType());
                while (ts.check("Comma")) {
                    addChild(node, consumeLeaf());
                    addChild(node, parseType());
                }
            }
            addChild(node, expectLeaf("RParen", "')' in tuple type"));
        } else {
            throw runtime_error(
                "Expected type, got '" + ts.peek().category +
                "' \"" + ts.peek().value + "\"  [token #" +
                to_string(ts.peek().pos) + "]");
        }
        return node;
    }

    // ----------------------------------------------------------
    //  Block -> '{' StmtList '}'
    // ----------------------------------------------------------
    NodePtr parseBlock() {
        auto node = makeNode("Block");
        addChild(node, expectLeaf("LBrace", "'{'"));
        while (!ts.check("RBrace") && !ts.atEnd()) {
            addChild(node, parseStmt());
        }
        addChild(node, expectLeaf("RBrace", "'}'"));
        return node;
    }

    // ----------------------------------------------------------
    //  Statement dispatch
    // ----------------------------------------------------------
    NodePtr parseStmt() {
        const Token& tok = ts.peek();

        // 1.2 empty statement
        if (tok.category == "Semicolon") {
            auto n = makeNode("EmptyStmt");
            addChild(n, consumeLeaf());
            return n;
        }
        // 2.1 / 2.3 let statement
        if (tok.category == "Let")      return parseLetStmt();
        // 1.3 / 1.5 return statement
        if (tok.category == "Return")   return parseReturnStmt();
        // 4.1  if statement
        if (tok.category == "If")       return parseIfStmt();
        // 5.1  while statement
        if (tok.category == "While")    return parseWhileStmt();
        // 5.2  for statement
        if (tok.category == "For")      return parseForStmt();
        // 5.3  loop statement
        if (tok.category == "Loop")     return parseLoopStmt();
        // 5.4  break
        if (tok.category == "Break") {
            auto n = makeNode("BreakStmt");
            addChild(n, consumeLeaf());
            addChild(n, expectLeaf("Semicolon", "';' after break"));
            return n;
        }
        // 5.4  continue
        if (tok.category == "Continue") {
            auto n = makeNode("ContinueStmt");
            addChild(n, consumeLeaf());
            addChild(n, expectLeaf("Semicolon", "';' after continue"));
            return n;
        }
        // nested block (e.g. block expression)
        if (tok.category == "LBrace")   return parseBlock();

        // 2.2 assignment or 3.1 expression statement
        return parseExprOrAssignStmt();
    }

    // ----------------------------------------------------------
    //  2.1 / 2.3  LetStmt -> let VarDecl ['=' Expr] ';'
    // ----------------------------------------------------------
    NodePtr parseLetStmt() {
        auto node = makeNode("LetStmt");
        addChild(node, expectLeaf("Let"));
        // 2.0 VarDecl: [mut] ID [':' Type]
        auto decl = makeNode("VarDecl");
        if (ts.check("Mut")) addChild(decl, consumeLeaf());  // 0.1
        addChild(decl, expectLeaf("Identifier", "variable name"));
        if (ts.check("Colon")) {                             // optional type
            addChild(decl, consumeLeaf());
            addChild(decl, parseType());
        }
        addChild(node, move(decl));
        if (ts.check("Assign")) {                            // 2.3 initializer
            addChild(node, consumeLeaf());
            addChild(node, parseExpr());
        }
        addChild(node, expectLeaf("Semicolon", "';' after let"));
        return node;
    }

    // ----------------------------------------------------------
    //  1.3 / 1.5  ReturnStmt -> return [Expr] ';'
    // ----------------------------------------------------------
    NodePtr parseReturnStmt() {
        auto node = makeNode("ReturnStmt");
        addChild(node, expectLeaf("Return"));
        if (!ts.check("Semicolon")) addChild(node, parseExpr());
        addChild(node, expectLeaf("Semicolon", "';' after return"));
        return node;
    }

    // ----------------------------------------------------------
    //  2.2 / 3.1  Expr [= Expr] ';'
    //  Strategy: parse an expression for the LHS, then check for '='.
    //  This correctly handles:
    //    x = expr;        (simple assignment, rule 2.2)
    //    arr[0] = expr;   (subscript assignment)
    //    expr;            (expression statement, rule 3.1)
    // ----------------------------------------------------------
    NodePtr parseExprOrAssignStmt() {
        NodePtr lhs = parseExpr();
        if (ts.check("Assign")) {
            auto node = makeNode("AssignStmt");
            addChild(node, move(lhs));
            addChild(node, consumeLeaf());   // '='
            addChild(node, parseExpr());
            addChild(node, expectLeaf("Semicolon", "';' after assignment"));
            return node;
        }
        // expression statement
        auto node = makeNode("ExprStmt");
        addChild(node, move(lhs));
        addChild(node, expectLeaf("Semicolon", "';' after expression"));
        return node;
    }

    // ----------------------------------------------------------
    //  4.1 / 4.2 / 4.3  IfStmt -> if Expr Block [ElseClause]
    // ----------------------------------------------------------
    NodePtr parseIfStmt() {
        auto node = makeNode("IfStmt");
        addChild(node, expectLeaf("If"));
        addChild(node, parseExpr());
        addChild(node, parseBlock());
        if (ts.check("Else")) {
            auto elseNode = makeNode("ElseClause");
            addChild(elseNode, consumeLeaf());    // 'else'
            if (ts.check("If")) {
                addChild(elseNode, parseIfStmt()); // 4.3 else-if (recursive)
            } else {
                addChild(elseNode, parseBlock());  // 4.2 else block
            }
            addChild(node, move(elseNode));
        }
        return node;
    }

    // ----------------------------------------------------------
    //  5.1  WhileStmt -> while Expr Block
    // ----------------------------------------------------------
    NodePtr parseWhileStmt() {
        auto node = makeNode("WhileStmt");
        addChild(node, expectLeaf("While"));
        addChild(node, parseExpr());
        addChild(node, parseBlock());
        return node;
    }

    // ----------------------------------------------------------
    //  5.2  ForStmt -> for [mut] ID in Expr Block
    //  The range start..end is handled as a range expression inside Expr.
    // ----------------------------------------------------------
    NodePtr parseForStmt() {
        auto node = makeNode("ForStmt");
        addChild(node, expectLeaf("For"));
        if (ts.check("Mut")) addChild(node, consumeLeaf());
        addChild(node, expectLeaf("Identifier", "loop variable"));
        addChild(node, expectLeaf("In", "'in'"));
        addChild(node, parseExpr());   // parses  start..end  as range expr
        addChild(node, parseBlock());
        return node;
    }

    // ----------------------------------------------------------
    //  5.3  LoopStmt -> loop Block
    // ----------------------------------------------------------
    NodePtr parseLoopStmt() {
        auto node = makeNode("LoopStmt");
        addChild(node, expectLeaf("Loop"));
        addChild(node, parseBlock());
        return node;
    }

    // ==========================================================
    //  Expressions  (left-recursion eliminated)
    //
    //  Precedence (low → high):
    //    DotDot   (..)       range
    //    Cmp      == != < <= > >=
    //    Add/Sub  + -
    //    Mul/Div  * /
    //    Unary    & &mut *
    //    Atom     literal | ID | ID(...) | ID[...] | (expr) | [...]
    // ==========================================================

    // Expr -> CmpExpr ['..' CmpExpr]
    NodePtr parseExpr() {
        NodePtr lhs = parseCmpExpr();
        if (ts.check("DotDot")) {
            auto node = makeNode("RangeExpr");
            addChild(node, move(lhs));
            addChild(node, consumeLeaf());      // '..'
            addChild(node, parseCmpExpr());
            return node;
        }
        return lhs;
    }

    // CmpExpr -> AddExpr { CmpOp AddExpr }
    NodePtr parseCmpExpr() {
        NodePtr lhs = parseAddExpr();
        while (isCmpOp()) {
            auto node = makeNode("CmpExpr");
            addChild(node, move(lhs));
            addChild(node, consumeLeaf());      // op
            addChild(node, parseAddExpr());
            lhs = move(node);
        }
        return lhs;
    }

    bool isCmpOp() const {
        const string& c = ts.peek().category;
        return c == "EqualEqual" || c == "NotEqual" ||
               c == "Greater"   || c == "GreaterEqual" ||
               c == "Less"      || c == "LessEqual";
    }

    // AddExpr -> Term { ('+' | '-') Term }
    NodePtr parseAddExpr() {
        NodePtr lhs = parseTerm();
        while (ts.check("Plus") || ts.check("Minus")) {
            auto node = makeNode("AddExpr");
            addChild(node, move(lhs));
            addChild(node, consumeLeaf());      // op
            addChild(node, parseTerm());
            lhs = move(node);
        }
        return lhs;
    }

    // Term -> Unary { ('*' | '/') Unary }
    NodePtr parseTerm() {
        NodePtr lhs = parseUnary();
        while (ts.check("Star") || ts.check("Slash")) {
            auto node = makeNode("MulExpr");
            addChild(node, move(lhs));
            addChild(node, consumeLeaf());      // op
            addChild(node, parseUnary());
            lhs = move(node);
        }
        return lhs;
    }

    // Unary -> '&' ['mut'] Unary  |  '*' Unary  |  Atom
    NodePtr parseUnary() {
        if (ts.check("Ampersand")) {
            auto node = makeNode("RefExpr");
            addChild(node, consumeLeaf());      // '&'
            if (ts.check("Mut")) addChild(node, consumeLeaf());
            addChild(node, parseUnary());
            return node;
        }
        if (ts.check("Star")) {
            auto node = makeNode("DerefExpr");
            addChild(node, consumeLeaf());      // '*'
            addChild(node, parseUnary());
            return node;
        }
        return parseAtom();
    }

    // Atom -> literal | '(' Expr ')' | '[' ExprList ']'
    //       | ID ['(' ArgList ')' | '[' Expr ']']
    NodePtr parseAtom() {
        const Token& tok = ts.peek();

        // numeric / string / char constant
        if (tok.type == "constant") {
            auto node = makeNode("Literal");
            addChild(node, consumeLeaf());
            return node;
        }

        // parenthesised expression
        if (tok.category == "LParen") {
            auto node = makeNode("ParenExpr");
            addChild(node, consumeLeaf());      // '('
            addChild(node, parseExpr());
            addChild(node, expectLeaf("RParen", "')'"));
            return node;
        }

        // array literal  [expr, expr, ...]
        if (tok.category == "LBracket") {
            auto node = makeNode("ArrayLit");
            addChild(node, consumeLeaf());      // '['
            if (!ts.check("RBracket")) {
                addChild(node, parseExpr());
                while (ts.check("Comma")) {
                    addChild(node, consumeLeaf());
                    addChild(node, parseExpr());
                }
            }
            addChild(node, expectLeaf("RBracket", "']'"));
            return node;
        }

        // identifier: function call, array subscript, or plain lvalue
        if (tok.category == "Identifier") {
            Token id = ts.advance();
            if (ts.check("LParen")) {           // 3.5 function call
                auto node = makeNode("CallExpr");
                addChild(node, makeLeaf("Identifier: " + id.value));
                addChild(node, consumeLeaf());  // '('
                addChild(node, parseArgList());
                addChild(node, expectLeaf("RParen", "')' after arguments"));
                return node;
            }
            if (ts.check("LBracket")) {         // array subscript
                auto node = makeNode("IndexExpr");
                addChild(node, makeLeaf("Identifier: " + id.value));
                addChild(node, consumeLeaf());  // '['
                addChild(node, parseExpr());
                addChild(node, expectLeaf("RBracket", "']'"));
                return node;
            }
            // plain identifier / lvalue
            auto node = makeNode("Identifier");
            addChild(node, makeLeaf("Identifier: " + id.value));
            return node;
        }

        throw runtime_error(
            "Expected expression, got '" + tok.category +
            "' \"" + tok.value + "\"  [token #" +
            to_string(tok.pos) + "]");
    }

    // ArgList -> ε | Expr {',' Expr}
    NodePtr parseArgList() {
        auto node = makeNode("ArgList");
        if (ts.check("RParen")) return node;    // ε
        addChild(node, parseExpr());
        while (ts.check("Comma")) {
            addChild(node, consumeLeaf());
            addChild(node, parseExpr());
        }
        return node;
    }
};

// ============================================================
//  main
// ============================================================
int main(int argc, char* argv[]) {
    string input = "./testfiles/input.rs" ;
    TokenStream ts = ReadLexer(input);

    Parser parser(ts);
    try {
        NodePtr tree = parser.parse();
        printTree(*tree);
        cout << "\n=== Parse successful ===\n";
        return 0;
    } catch (const exception& e) {
        cerr << "\n=== Parse error ===\n" << e.what() << "\n";
        return 1;
    }
}
