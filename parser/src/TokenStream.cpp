// ============================================================
//  TokenStream  –  inline implementations
// ============================================================
#include "../include/TokenStream.h"

/**
 * @param str 输入字符串
 * @return 转义后的字符串（$被转义为$$，\n被转义为$n，\r被转义为$r，\t被转义为$t，\0被转义为$0，空格被转义为$_）
 */
char* rebuild_string(char* str) {
    int size=strlen(str);
    char* result=(char*)malloc(size*2+1);
    int result_index=0;
    for(int i=0;i<size;i++){
        if(str[i]=='$'){
            result[result_index++]='$';
            result[result_index++]='$';
        } else if(str[i]=='\n'){
            result[result_index++]='$';
            result[result_index++]='n';
        } else if(str[i]=='\r'){
            result[result_index++]='$';
            result[result_index++]='r';
        } else if(str[i]=='\t'){
            result[result_index++]='$';
            result[result_index++]='t';
        } else if(str[i]=='\0'){
            result[result_index++]='$';
            result[result_index++]='0';
        }
        else if (str[i] == ' '){
            result[result_index++]='$';
            result[result_index++]='_';
        }
        else{
            result[result_index++]=str[i];
        }
    }
    result[result_index]='\0';
    return result;
}
// ============================================================
//  Static helper
// ============================================================
string TokenStream::trim(const string& s) {
    size_t l = s.find_first_not_of(" \t\r\n");
    size_t r = s.find_last_not_of(" \t\r\n");
    return (l == string::npos) ? "" : s.substr(l, r - l + 1);
}

// ============================================================
//  TokenStream member functions
// ============================================================

void TokenStream::load(istream& in) {
    string line;
    getline(in, line); // skip header
    int idx = 0;
    while (getline(in, line)) {
        if (line.empty()) continue;

        // Split on the first two '\t' characters only.
        // Value field may legitimately contain escape sequences.
        size_t t1 = line.find('\t');
        if (t1 == string::npos) continue;
        size_t t2 = line.find('\t', t1 + 1);
        if (t2 == string::npos) continue;

        Token tok;
        tok.type     = StringToType(trim(line.substr(0, t1)));
        tok.category = trim(line.substr(t1 + 1, t2 - t1 - 1));
        tok.value    = line.substr(t2 + 1);   // keep as-is (may have $_ etc.)
        tok.pos      = idx;

        if (tok.type == Type::COMMENT) continue;  // skip comments
        if (tok.type == Type::TYPE_EOF) continue;

        toks.push_back(tok);
        ++idx;
    }
}

const Token& TokenStream::peek(int offset) const {
    int i = cur + offset;
    if (i < 0 || i >= (int)toks.size()) return EOF_TOKEN;
    return toks[i];
}

Token TokenStream::advance() {
    if (cur >= (int)toks.size()) return EOF_TOKEN;
    return toks[cur++];
}

Token TokenStream::expect(const string& cat, const string& ctx) {
    const Token& t = peek();
    if (t.category != cat) {
        string msg = "Expected '" + cat + "'";
        if (!ctx.empty()) msg += " (" + ctx + ")";
        msg += " but got '" + t.category + "'";
        if (!t.value.empty()) msg += " \"" + t.value + "\"";
        msg += "  [token #" + to_string(t.pos) + "]";
        throw runtime_error(msg);
    }
    return advance();
}

bool TokenStream::check(const string& cat) const {
    return peek().category == cat;
}

bool TokenStream::checkType(const string& tp, int offset) const {
    return typeToString(peek(offset).type) == tp;
}

bool TokenStream::atEnd() const {
    return cur >= (int)toks.size() || peek().category == "End" || peek().category == "EOF";
}

bool TokenStream::match(const string& cat) {
    if (check(cat)) { advance(); return true; }
    return false;
}
bool TokenStream::addToken(const Token& t,bool IsPrint) {

    if (IsPrint) {
        cout<<t.type<<"\t"<<t.category<<"\t"<<t.value<<"\t" << t.pos <<"\n";
    }
    
    toks.push_back(t);
    
    return true;
}

void TokenStream::displayTokens() const {
    for (const auto& t : toks) {
        cout << "Type\tCategory\tValue\tPos\tLine\tColumn\n";
        cout << t << endl;
    }
}

void TokenStream::to_file(const string& filename) const {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        cout << "Failed to open file: " << filename << endl;
        return;
    }
    ofs << "Type\tCategory\tValue\tPos\tLine\tColumn\n";
    for (const auto& t : toks) {
        ofs << t <<endl;
    }
    ofs.close();
}

void TokenStream::remove_comments() {
    vector<Token> new_toks;
    for (const auto& t : toks) {
        if (typeToString(t.type) != "comment") {
            new_toks.push_back(t);
        }
    }
    toks = move(new_toks);
}