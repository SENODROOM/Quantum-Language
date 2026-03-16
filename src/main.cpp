/*
 * quantum — Quantum Language Native Compiler
 *
 * Pipeline:  Source (.sa) → Lexer → Parser → AST → C Transpiler → g++ → Native EXE
 *
 * Usage:
 *   quantum <file.sa>              Compile to native executable
 *   quantum <file.sa> -o <out>     Compile with custom output name
 *   quantum <file.sa> --run        Compile and immediately run
 *   quantum <file.sa> --emit-c     Emit generated C++ source only
 *   quantum --version
 *   quantum --help
 */

#include "Lexer.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "Error.h"
#include "AST.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;

// ─── Colour codes (same as the rest of the project) ──────────────────────────
namespace Colors
{
    inline const char *RED = "\033[31m";
    inline const char *YELLOW = "\033[33m";
    inline const char *GREEN = "\033[32m";
    inline const char *CYAN = "\033[36m";
    inline const char *BOLD = "\033[1m";
    inline const char *RESET = "\033[0m";
}

// ─── Banner ───────────────────────────────────────────────────────────────────
static void printBanner()
{
    std::cout << Colors::CYAN << Colors::BOLD
              << "\n"
              << "  ██████╗ ██╗   ██╗ █████╗ ███╗   ██╗████████╗██╗   ██╗███╗   ███╗\n"
              << " ██╔═══██╗██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██║   ██║████╗ ████║\n"
              << " ██║   ██║██║   ██║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║\n"
              << " ██║▄▄ ██║██║   ██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║\n"
              << " ╚██████╔╝╚██████╔╝██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║\n"
              << "  ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝\n"
              << Colors::RESET
              << Colors::YELLOW
              << "  Quantum Language v2.0.0 | Native Compiler\n"
              << Colors::RESET << "\n";
}

static void printHelp(const char *prog)
{
    std::cout << Colors::BOLD << "Usage:\n"
              << Colors::RESET
              << "  " << prog << " <file.sa>              Compile to native executable\n"
              << "  " << prog << " <file.sa> -o <name>     Set output binary name\n"
              << "  " << prog << " <file.sa> --run          Compile and run immediately\n"
              << "  " << prog << " <file.sa> --emit-c       Print generated C++ (no compile)\n"
              << "  " << prog << " --version               Show version\n"
              << "  " << prog << " --help                  Show this help\n\n"
              << Colors::BOLD << "Interpreted mode (REPL / scripting):\n"
              << Colors::RESET
              << "  Use " << Colors::CYAN << "qrun" << Colors::RESET
              << " for fast iteration, REPL, and debugging.\n\n"
              << Colors::BOLD << "Notes:\n"
              << Colors::RESET
              << "  Requires g++ (MinGW on Windows) in PATH for the final compilation step.\n"
              << "  The compiler generates standard C++17 and links a small Quantum runtime.\n";
}

// ─── C++ Transpiler ───────────────────────────────────────────────────────────
// Walks the AST and emits valid C++17 code that, when compiled, behaves
// identically to the bytecode VM execution.
//
// Strategy: emit all Quantum values as a tagged union (QVal), all functions as
// C++ functions, all classes as C++ structs with vtable-style method maps.
// The runtime header (quantum_rt.h) is generated alongside the source.

class Transpiler
{
public:
    std::string transpile(ASTNode &root);

private:
    std::ostringstream out_;
    int indent_ = 0;
    int tmpCount_ = 0;
    std::unordered_set<std::string> declaredVars_;
    std::vector<std::string> scopeStack_;

    std::string freshTmp() { return "__t" + std::to_string(tmpCount_++); }
    std::string ind() { return std::string(indent_ * 4, ' '); }
    void push()
    {
        indent_++;
        scopeStack_.push_back("");
    }
    void pop()
    {
        indent_--;
        scopeStack_.pop_back();
    }

    void emitRuntime();
    void emitNode(ASTNode &node);
    void emitBlock(BlockStmt &b);
    std::string emitExpr(ASTNode &node);

    // Statement emitters
    void emitVarDecl(VarDecl &s, int line);
    void emitFunctionDecl(FunctionDecl &s, int line);
    void emitClassDecl(ClassDecl &s, int line);
    void emitIf(IfStmt &s, int line);
    void emitWhile(WhileStmt &s, int line);
    void emitFor(ForStmt &s, int line);
    void emitReturn(ReturnStmt &s, int line);
    void emitPrint(PrintStmt &s, int line);

    // Expression emitters (return C++ expression string)
    std::string emitBinary(BinaryExpr &e);
    std::string emitUnary(UnaryExpr &e);
    std::string emitCall(CallExpr &e);
    std::string emitIndex(IndexExpr &e);
    std::string emitMember(MemberExpr &e);
    std::string emitAssign(AssignExpr &e);
    std::string emitArray(ArrayLiteral &e);
    std::string emitDict(DictLiteral &e);
    std::string emitLambda(LambdaExpr &e);
    std::string emitTernary(TernaryExpr &e);
    std::string escapeString(const std::string &s);
};

// ─── Runtime header (emitted into a temp file alongside generated .cpp) ───────
static const char *QUANTUM_RUNTIME_H = R"(
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <cassert>

// ── QVal: universal Quantum value ─────────────────────────────────────────────
struct QNil {};
struct QVal;
using QArr  = std::vector<QVal>;
using QDict = std::unordered_map<std::string, QVal>;
using QFn   = std::function<QVal(std::vector<QVal>)>;

struct QVal {
    using Data = std::variant<QNil, bool, double, std::string,
                              std::shared_ptr<QArr>,
                              std::shared_ptr<QDict>,
                              QFn>;
    Data data;

    QVal()                           : data(QNil{}) {}
    QVal(bool b)                     : data(b) {}
    QVal(double d)                   : data(d) {}
    QVal(int i)                      : data((double)i) {}
    QVal(long long i)                : data((double)i) {}
    QVal(const char* s)              : data(std::string(s)) {}
    QVal(const std::string& s)       : data(s) {}
    QVal(std::shared_ptr<QArr>  a)   : data(std::move(a)) {}
    QVal(std::shared_ptr<QDict> d)   : data(std::move(d)) {}
    QVal(QFn f)                      : data(std::move(f)) {}

    bool isNil()    const { return std::holds_alternative<QNil>(data); }
    bool isBool()   const { return std::holds_alternative<bool>(data); }
    bool isNum()    const { return std::holds_alternative<double>(data); }
    bool isStr()    const { return std::holds_alternative<std::string>(data); }
    bool isArr()    const { return std::holds_alternative<std::shared_ptr<QArr>>(data); }
    bool isDict()   const { return std::holds_alternative<std::shared_ptr<QDict>>(data); }
    bool isFn()     const { return std::holds_alternative<QFn>(data); }

    bool        asBool() const { return std::get<bool>(data); }
    double      asNum()  const { return std::get<double>(data); }
    std::string asStr()  const { return std::get<std::string>(data); }
    std::shared_ptr<QArr>  asArr()  const { return std::get<std::shared_ptr<QArr>>(data); }
    std::shared_ptr<QDict> asDict() const { return std::get<std::shared_ptr<QDict>>(data); }
    QFn                    asFn()   const { return std::get<QFn>(data); }

    bool truthy() const {
        return std::visit([](const auto& v) -> bool {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T,QNil>)   return false;
            if constexpr (std::is_same_v<T,bool>)   return v;
            if constexpr (std::is_same_v<T,double>)  return v != 0.0;
            if constexpr (std::is_same_v<T,std::string>) return !v.empty();
            if constexpr (std::is_same_v<T,std::shared_ptr<QArr>>)  return !v->empty();
            return true;
        }, data);
    }

    std::string str() const {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T,QNil>)    return "nil";
            if constexpr (std::is_same_v<T,bool>)    return v ? "true" : "false";
            if constexpr (std::is_same_v<T,double>) {
                if (std::floor(v) == v && std::abs(v) < 1e15)
                    return std::to_string((long long)v);
                std::ostringstream o; o << v; return o.str();
            }
            if constexpr (std::is_same_v<T,std::string>) return v;
            if constexpr (std::is_same_v<T,std::shared_ptr<QArr>>) {
                std::string s = "[";
                for (size_t i = 0; i < v->size(); i++) {
                    if (i) s += ", ";
                    auto& el = (*v)[i];
                    if (el.isStr()) s += "\"" + el.str() + "\"";
                    else            s += el.str();
                }
                return s + "]";
            }
            if constexpr (std::is_same_v<T,std::shared_ptr<QDict>>) {
                std::string s = "{";
                bool first = true;
                for (auto& [k,val] : *v) {
                    if (!first) s += ", ";
                    s += "\"" + k + "\": ";
                    if (val.isStr()) s += "\"" + val.str() + "\"";
                    else             s += val.str();
                    first = false;
                }
                return s + "}";
            }
            if constexpr (std::is_same_v<T,QFn>) return "<function>";
            return "?";
        }, data);
    }

    // Arithmetic operators
    QVal operator+(const QVal& o) const {
        if (isStr() || o.isStr()) return QVal(str() + o.str());
        if (isNum() && o.isNum()) return QVal(asNum() + o.asNum());
        return QVal(str() + o.str());
    }
    QVal operator-(const QVal& o) const { return QVal(asNum() - o.asNum()); }
    QVal operator*(const QVal& o) const {
        if (isStr() && o.isNum()) {
            std::string r; for (int i = 0; i < (int)o.asNum(); i++) r += asStr();
            return QVal(r);
        }
        return QVal(asNum() * o.asNum());
    }
    QVal operator/(const QVal& o) const {
        if (o.asNum() == 0) throw std::runtime_error("Division by zero");
        return QVal(asNum() / o.asNum());
    }
    QVal operator%(const QVal& o) const {
        if (o.asNum() == 0) throw std::runtime_error("Modulo by zero");
        return QVal(std::fmod(asNum(), o.asNum()));
    }
    QVal pow(const QVal& o) const { return QVal(std::pow(asNum(), o.asNum())); }

    bool operator==(const QVal& o) const {
        if (isNil() && o.isNil()) return true;
        if (isBool() && o.isBool()) return asBool() == o.asBool();
        if (isNum()  && o.isNum())  return asNum()  == o.asNum();
        if (isStr()  && o.isStr())  return asStr()  == o.asStr();
        return false;
    }
    bool operator!=(const QVal& o) const { return !(*this == o); }
    bool operator< (const QVal& o) const { return asNum() <  o.asNum(); }
    bool operator<=(const QVal& o) const { return asNum() <= o.asNum(); }
    bool operator> (const QVal& o) const { return asNum() >  o.asNum(); }
    bool operator>=(const QVal& o) const { return asNum() >= o.asNum(); }

    // Index access
    QVal& operator[](const QVal& key) {
        if (isArr())  { int i = (int)key.asNum(); auto& a = *asArr(); if(i<0) i+=(int)a.size(); return a[i]; }
        if (isDict()) { return (*asDict())[key.str()]; }
        throw std::runtime_error("Cannot index type");
    }
    const QVal& operator[](const QVal& key) const {
        if (isArr())  { int i = (int)key.asNum(); auto& a = *asArr(); if(i<0) i+=(int)a.size(); return a[i]; }
        if (isDict()) { static QVal nil; auto it = asDict()->find(key.str()); return it!=asDict()->end()?it->second:nil; }
        throw std::runtime_error("Cannot index type");
    }
};

// ── Call helper ───────────────────────────────────────────────────────────────
inline QVal qcall(const QVal& fn, std::vector<QVal> args) {
    if (!fn.isFn()) throw std::runtime_error("Cannot call " + fn.str());
    return fn.asFn()(std::move(args));
}

// ── Built-in functions ────────────────────────────────────────────────────────
inline QVal q_print(std::vector<QVal> args) {
    for (size_t i = 0; i < args.size(); i++) {
        if (i) std::cout << " ";
        std::cout << args[i].str();
    }
    std::cout << "\n";
    return QVal();
}
inline QVal q_str(std::vector<QVal> a)  { return a.empty()?QVal(""):QVal(a[0].str()); }
inline QVal q_num(std::vector<QVal> a)  { if(a.empty()) return QVal(0.0); if(a[0].isNum()) return a[0]; try{return QVal(std::stod(a[0].str()));}catch(...){return QVal(0.0);} }
inline QVal q_int(std::vector<QVal> a)  { if(a.empty()) return QVal(0.0); return QVal(std::floor(q_num(a).asNum())); }
inline QVal q_bool(std::vector<QVal> a) { return a.empty()?QVal(false):QVal(a[0].truthy()); }
inline QVal q_len(std::vector<QVal> a)  { if(a.empty()) return QVal(0.0); if(a[0].isStr()) return QVal((double)a[0].asStr().size()); if(a[0].isArr()) return QVal((double)a[0].asArr()->size()); if(a[0].isDict()) return QVal((double)a[0].asDict()->size()); return QVal(0.0); }
inline QVal q_type(std::vector<QVal> a) {
    if(a.empty()) return QVal("nil");
    auto& v=a[0];
    if(v.isNil()) return QVal("nil");
    if(v.isBool()) return QVal("bool");
    if(v.isNum()) return QVal("number");
    if(v.isStr()) return QVal("string");
    if(v.isArr()) return QVal("array");
    if(v.isDict()) return QVal("dict");
    if(v.isFn()) return QVal("function");
    return QVal("unknown");
}
inline QVal q_range(std::vector<QVal> a) {
    double start=0,end_=0,step=1;
    if(a.size()==1){end_=a[0].asNum();}
    else if(a.size()==2){start=a[0].asNum();end_=a[1].asNum();}
    else{start=a[0].asNum();end_=a[1].asNum();step=a[2].asNum();}
    auto arr=std::make_shared<QArr>();
    if(step>0) for(double i=start;i<end_;i+=step) arr->push_back(QVal(i));
    else for(double i=start;i>end_;i+=step) arr->push_back(QVal(i));
    return QVal(arr);
}
inline QVal q_input(std::vector<QVal> a) {
    if(!a.empty()) std::cout << a[0].str();
    std::string line; std::getline(std::cin,line); return QVal(line);
}
inline QVal q_abs(std::vector<QVal> a)   { return QVal(std::abs(a[0].asNum())); }
inline QVal q_sqrt(std::vector<QVal> a)  { return QVal(std::sqrt(a[0].asNum())); }
inline QVal q_floor(std::vector<QVal> a) { return QVal(std::floor(a[0].asNum())); }
inline QVal q_ceil(std::vector<QVal> a)  { return QVal(std::ceil(a[0].asNum())); }
inline QVal q_round(std::vector<QVal> a) { return QVal(std::round(a[0].asNum())); }
inline QVal q_pow(std::vector<QVal> a)   { return QVal(std::pow(a[0].asNum(),a[1].asNum())); }
inline QVal q_max(std::vector<QVal> a)   {
    if(a.size()==1&&a[0].isArr()){auto&v=*a[0].asArr();double m=v[0].asNum();for(auto&x:v)m=std::max(m,x.asNum());return QVal(m);}
    double m=a[0].asNum();for(auto&x:a)m=std::max(m,x.asNum());return QVal(m);
}
inline QVal q_min(std::vector<QVal> a)   {
    if(a.size()==1&&a[0].isArr()){auto&v=*a[0].asArr();double m=v[0].asNum();for(auto&x:v)m=std::min(m,x.asNum());return QVal(m);}
    double m=a[0].asNum();for(auto&x:a)m=std::min(m,x.asNum());return QVal(m);
}
inline QVal q_sorted(std::vector<QVal> a){
    if(a.empty()||!a[0].isArr()) return a.empty()?QVal():a[0];
    auto copy=std::make_shared<QArr>(*a[0].asArr());
    std::sort(copy->begin(),copy->end(),[](const QVal&x,const QVal&y){
        return x.isNum()&&y.isNum()?x.asNum()<y.asNum():x.str()<y.str();
    });
    return QVal(copy);
}
inline QVal q_reversed(std::vector<QVal> a){
    if(a.empty()||!a[0].isArr()) return a.empty()?QVal():a[0];
    auto copy=std::make_shared<QArr>(*a[0].asArr());
    std::reverse(copy->begin(),copy->end());
    return QVal(copy);
}
inline QVal q_sum(std::vector<QVal> a){
    if(a.empty()||!a[0].isArr()) return QVal(0.0);
    double s=0; for(auto&v:*a[0].asArr()) s+=v.asNum(); return QVal(s);
}
inline QVal q_enumerate(std::vector<QVal> a){
    if(a.empty()||!a[0].isArr()) return QVal(std::make_shared<QArr>());
    auto arr=std::make_shared<QArr>();
    int start=a.size()>1?(int)a[1].asNum():0;
    for(auto&v:*a[0].asArr()){
        auto pair=std::make_shared<QArr>();
        pair->push_back(QVal((double)start++));
        pair->push_back(v);
        arr->push_back(QVal(pair));
    }
    return QVal(arr);
}
inline QVal q_zip(std::vector<QVal> a){
    auto arr=std::make_shared<QArr>();
    if(a.empty()) return QVal(arr);
    size_t minLen=SIZE_MAX;
    for(auto&x:a) if(x.isArr()) minLen=std::min(minLen,x.asArr()->size());
    if(minLen==SIZE_MAX) minLen=0;
    for(size_t i=0;i<minLen;i++){
        auto t=std::make_shared<QArr>();
        for(auto&x:a) if(x.isArr()) t->push_back((*x.asArr())[i]);
        arr->push_back(QVal(t));
    }
    return QVal(arr);
}
inline QVal q_map(std::vector<QVal> a){
    if(a.size()<2||!a[1].isArr()) return QVal(std::make_shared<QArr>());
    auto arr=std::make_shared<QArr>();
    for(auto&v:*a[1].asArr()) arr->push_back(qcall(a[0],{v}));
    return QVal(arr);
}
inline QVal q_filter(std::vector<QVal> a){
    if(a.size()<2||!a[1].isArr()) return QVal(std::make_shared<QArr>());
    auto arr=std::make_shared<QArr>();
    for(auto&v:*a[1].asArr()) if(qcall(a[0],{v}).truthy()) arr->push_back(v);
    return QVal(arr);
}

// ── Array method dispatcher ───────────────────────────────────────────────────
inline QVal qArrMethod(std::shared_ptr<QArr> arr, const std::string& m, std::vector<QVal> a){
    if(m=="push"||m=="append"){arr->push_back(a.empty()?QVal():a[0]);return QVal(arr);}
    if(m=="pop"){if(arr->empty())throw std::runtime_error("pop on empty array");QVal v=arr->back();arr->pop_back();return v;}
    if(m=="length"||m=="size") return QVal((double)arr->size());
    if(m=="join"){std::string sep=a.empty()?",":a[0].str(),r;for(size_t i=0;i<arr->size();i++){if(i)r+=sep;r+=(*arr)[i].str();}return QVal(r);}
    if(m=="sort"){std::sort(arr->begin(),arr->end(),[](const QVal&x,const QVal&y){return x.isNum()&&y.isNum()?x.asNum()<y.asNum():x.str()<y.str();});return QVal(arr);}
    if(m=="reverse"){std::reverse(arr->begin(),arr->end());return QVal(arr);}
    if(m=="includes"||m=="contains"){for(auto&v:*arr)if(v==a[0])return QVal(true);return QVal(false);}
    if(m=="indexOf"){for(size_t i=0;i<arr->size();i++)if((*arr)[i]==a[0])return QVal((double)i);return QVal(-1.0);}
    if(m=="slice"){int s=a.empty()?0:(int)a[0].asNum(),e=a.size()>1?(int)a[1].asNum():(int)arr->size();int l=(int)arr->size();if(s<0)s=std::max(0,l+s);if(e<0)e=std::max(0,l+e);e=std::min(e,l);return QVal(std::make_shared<QArr>(arr->begin()+s,arr->begin()+e));}
    if(m=="clear"){arr->clear();return QVal();}
    if(m=="copy") return QVal(std::make_shared<QArr>(*arr));
    if(m=="insert"){if(a.size()>=2){int idx=(int)a[0].asNum();if(idx<0)idx=std::max(0,(int)arr->size()+idx);idx=std::min(idx,(int)arr->size());arr->insert(arr->begin()+idx,a[1]);}return QVal();}
    if(m=="remove"){for(auto it=arr->begin();it!=arr->end();++it)if(*it==a[0]){arr->erase(it);break;}return QVal();}
    if(m=="count"){int c=0;for(auto&v:*arr)if(v==a[0])c++;return QVal((double)c);}
    if(m=="extend"){if(!a.empty()&&a[0].isArr())for(auto&v:*a[0].asArr())arr->push_back(v);return QVal();}
    if(m=="flat"||m=="flatten"){auto r=std::make_shared<QArr>();for(auto&v:*arr){if(v.isArr())for(auto&x:*v.asArr())r->push_back(x);else r->push_back(v);}return QVal(r);}
    throw std::runtime_error("Array has no method '" + m + "'");
}

// ── String method dispatcher ──────────────────────────────────────────────────
inline QVal qStrMethod(const std::string& s, const std::string& m, std::vector<QVal> a){
    if(m=="length"||m=="size") return QVal((double)s.size());
    if(m=="upper"||m=="toUpperCase"){std::string r=s;std::transform(r.begin(),r.end(),r.begin(),::toupper);return QVal(r);}
    if(m=="lower"||m=="toLowerCase"){std::string r=s;std::transform(r.begin(),r.end(),r.begin(),::tolower);return QVal(r);}
    if(m=="trim"||m=="strip"){std::string r=s;while(!r.empty()&&std::isspace((unsigned char)r.front()))r.erase(r.begin());while(!r.empty()&&std::isspace((unsigned char)r.back()))r.pop_back();return QVal(r);}
    if(m=="includes"||m=="contains"){return QVal(!a.empty()&&s.find(a[0].str())!=std::string::npos);}
    if(m=="startsWith"||m=="startswith"){return QVal(!a.empty()&&s.substr(0,a[0].str().size())==a[0].str());}
    if(m=="endsWith"||m=="endswith"){auto t=a.empty()?std::string():a[0].str();return QVal(s.size()>=t.size()&&s.substr(s.size()-t.size())==t);}
    if(m=="indexOf"){auto p=a.empty()?std::string::npos:s.find(a[0].str());return QVal(p==std::string::npos?-1.0:(double)p);}
    if(m=="split"){
        std::string sep=a.empty()||a[0].isNil()?"":a[0].str();
        auto arr=std::make_shared<QArr>();
        if(sep.empty()){for(char c:s)arr->push_back(QVal(std::string(1,c)));}
        else{size_t p=0,f;while((f=s.find(sep,p))!=std::string::npos){arr->push_back(QVal(s.substr(p,f-p)));p=f+sep.size();}arr->push_back(QVal(s.substr(p)));}
        return QVal(arr);
    }
    if(m=="replace"){if(a.size()<2)return QVal(s);std::string r=s,from=a[0].str(),to=a[1].str();size_t p=r.find(from);if(p!=std::string::npos)r=r.substr(0,p)+to+r.substr(p+from.size());return QVal(r);}
    if(m=="replaceAll"){if(a.size()<2)return QVal(s);std::string r=s,from=a[0].str(),to=a[1].str();size_t p=0;while((p=r.find(from,p))!=std::string::npos){r=r.substr(0,p)+to+r.substr(p+from.size());p+=to.size();}return QVal(r);}
    if(m=="substring"||m=="substr"){int st=a.empty()?0:(int)a[0].asNum();int ln=(int)s.size()-st;if(a.size()>1)ln=(int)a[1].asNum();if(st<0)st=0;return QVal(s.substr(std::min((size_t)st,s.size()),std::max(0,ln)));}
    if(m=="repeat"){int n=a.empty()?0:(int)a[0].asNum();std::string r;for(int i=0;i<n;i++)r+=s;return QVal(r);}
    if(m=="format"){std::string r=s;size_t idx=0,p;while((p=r.find("{}"))!=std::string::npos&&idx<a.size()){r=r.substr(0,p)+a[idx++].str()+r.substr(p+2);}return QVal(r);}
    if(m=="isdigit"){for(char c:s)if(!std::isdigit((unsigned char)c))return QVal(false);return QVal(!s.empty());}
    if(m=="isalpha"){for(char c:s)if(!std::isalpha((unsigned char)c))return QVal(false);return QVal(!s.empty());}
    if(m=="count"){if(a.empty())return QVal((double)s.size());std::string sub=a[0].str();int cnt=0;size_t p=0;while((p=s.find(sub,p))!=std::string::npos){cnt++;p+=sub.size();}return QVal((double)cnt);}
    if(m=="charAt"){if(a.empty()||a[0].asNum()<0||(size_t)a[0].asNum()>=s.size())return QVal(std::string(""));return QVal(std::string(1,s[(int)a[0].asNum()]));}
    throw std::runtime_error("String has no method '" + m + "'");
}

// ── Dict method dispatcher ────────────────────────────────────────────────────
inline QVal qDictMethod(std::shared_ptr<QDict> d, const std::string& m, std::vector<QVal> a){
    if(m=="keys"){auto arr=std::make_shared<QArr>();for(auto&[k,v]:*d)arr->push_back(QVal(k));return QVal(arr);}
    if(m=="values"){auto arr=std::make_shared<QArr>();for(auto&[k,v]:*d)arr->push_back(v);return QVal(arr);}
    if(m=="items"||m=="entries"){auto arr=std::make_shared<QArr>();for(auto&[k,v]:*d){auto p=std::make_shared<QArr>();p->push_back(QVal(k));p->push_back(v);arr->push_back(QVal(p));}return QVal(arr);}
    if(m=="has"||m=="contains"){return a.empty()?QVal(false):QVal(d->count(a[0].str())>0);}
    if(m=="get"){if(a.empty())return QVal();auto it=d->find(a[0].str());return it!=d->end()?it->second:(a.size()>1?a[1]:QVal());}
    if(m=="set"){if(a.size()>=2)(*d)[a[0].str()]=a[1];return QVal(d);}
    if(m=="delete"){if(!a.empty())d->erase(a[0].str());return QVal(true);}
    if(m=="clear"){d->clear();return QVal();}
    if(m=="size"||m=="length") return QVal((double)d->size());
    throw std::runtime_error("Dict has no method '" + m + "'");
}

// ── Universal member/method dispatch ─────────────────────────────────────────
inline QVal qGetMember(QVal& obj, const std::string& name) {
    if (obj.isArr())  return QVal([arr=obj.asArr(),name](std::vector<QVal> a){ return qArrMethod(arr,name,a); });
    if (obj.isStr())  return QVal([s=obj.asStr(),name](std::vector<QVal> a){ return qStrMethod(s,name,a); });
    if (obj.isDict()) {
        auto it = obj.asDict()->find(name);
        if (it != obj.asDict()->end()) return it->second;
        return QVal([d=obj.asDict(),name](std::vector<QVal> a){ return qDictMethod(d,name,a); });
    }
    throw std::runtime_error("No member '" + name + "' on " + obj.str());
}
inline void qSetMember(QVal& obj, const std::string& name, QVal val) {
    if (obj.isDict()) { (*obj.asDict())[name] = val; return; }
    throw std::runtime_error("Cannot set member '" + name + "'");
}

// ── Globals setup ─────────────────────────────────────────────────────────────
inline std::unordered_map<std::string,QVal> qMakeGlobals() {
    std::unordered_map<std::string,QVal> g;
    g["print"]     = QVal(q_print);
    g["str"]       = QVal(q_str);
    g["num"]       = QVal(q_num);
    g["int"]       = QVal(q_int);
    g["float"]     = QVal(q_num);
    g["bool"]      = QVal(q_bool);
    g["len"]       = QVal(q_len);
    g["type"]      = QVal(q_type);
    g["range"]     = QVal(q_range);
    g["input"]     = QVal(q_input);
    g["abs"]       = QVal(q_abs);
    g["sqrt"]      = QVal(q_sqrt);
    g["floor"]     = QVal(q_floor);
    g["ceil"]      = QVal(q_ceil);
    g["round"]     = QVal(q_round);
    g["pow"]       = QVal(q_pow);
    g["max"]       = QVal(q_max);
    g["min"]       = QVal(q_min);
    g["sorted"]    = QVal(q_sorted);
    g["reversed"]  = QVal(q_reversed);
    g["sum"]       = QVal(q_sum);
    g["enumerate"] = QVal(q_enumerate);
    g["zip"]       = QVal(q_zip);
    g["map"]       = QVal(q_map);
    g["filter"]    = QVal(q_filter);
    g["PI"]        = QVal(3.14159265358979323846);
    g["E"]         = QVal(2.71828182845904523536);
    g["true"]      = QVal(true);
    g["false"]     = QVal(false);
    g["nil"]       = QVal();
    g["None"]      = QVal();
    g["null"]      = QVal();
    return g;
}
)";

// ─── Transpiler: emit the runtime header then translate the AST ───────────────

std::string Transpiler::escapeString(const std::string &s)
{
    std::string r;
    for (char c : s)
    {
        if (c == '"')
            r += "\\\"";
        else if (c == '\\')
            r += "\\\\";
        else if (c == '\n')
            r += "\\n";
        else if (c == '\r')
            r += "\\r";
        else if (c == '\t')
            r += "\\t";
        else
            r += c;
    }
    return r;
}

std::string Transpiler::transpile(ASTNode &root)
{
    out_.str("");
    out_ << "// Generated by Quantum Native Compiler v2.0\n";
    out_ << "#include \"quantum_rt.h\"\n\n";
    out_ << "int main() {\n";
    out_ << "    auto __g = qMakeGlobals();\n";
    out_ << "    (void)__g;\n";
    indent_ = 1;

    if (root.is<BlockStmt>())
        emitBlock(root.as<BlockStmt>());
    else
        emitNode(root);

    out_ << "    return 0;\n";
    out_ << "}\n";
    return out_.str();
}

void Transpiler::emitBlock(BlockStmt &b)
{
    for (auto &stmt : b.statements)
        emitNode(*stmt);
}

void Transpiler::emitNode(ASTNode &node)
{
    std::visit([&](auto &n)
               {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, BlockStmt>)     emitBlock(n);
        else if constexpr (std::is_same_v<T, VarDecl>)       emitVarDecl(n, node.line);
        else if constexpr (std::is_same_v<T, FunctionDecl>)  emitFunctionDecl(n, node.line);
        else if constexpr (std::is_same_v<T, ClassDecl>)     emitClassDecl(n, node.line);
        else if constexpr (std::is_same_v<T, IfStmt>)        emitIf(n, node.line);
        else if constexpr (std::is_same_v<T, WhileStmt>)     emitWhile(n, node.line);
        else if constexpr (std::is_same_v<T, ForStmt>)       emitFor(n, node.line);
        else if constexpr (std::is_same_v<T, ReturnStmt>)    emitReturn(n, node.line);
        else if constexpr (std::is_same_v<T, PrintStmt>)     emitPrint(n, node.line);
        else if constexpr (std::is_same_v<T, ExprStmt>) {
            out_ << ind() << emitExpr(*n.expr) << ";\n";
        }
        else if constexpr (std::is_same_v<T, BreakStmt>)    out_ << ind() << "break;\n";
        else if constexpr (std::is_same_v<T, ContinueStmt>) out_ << ind() << "continue;\n";
        else if constexpr (std::is_same_v<T, RaiseStmt>) {
            auto val = n.value ? emitExpr(*n.value) : "QVal()";
            out_ << ind() << "throw std::runtime_error((" << val << ").str());\n";
        }
        else if constexpr (std::is_same_v<T, TryStmt>) {
            out_ << ind() << "try {\n"; push();
            if (n.body) emitNode(*n.body); pop();
            for (auto& h : n.handlers) {
                std::string varName = h.alias.empty() ? h.errorType : h.alias;
                out_ << ind() << "} catch(const std::exception& ";
                out_ << (varName.empty() ? "__ex__" : ("__ex_" + varName)) << ") {\n";
                push();
                if (!varName.empty())
                    out_ << ind() << "QVal " << varName << " = QVal(std::string("
                         << (varName.empty() ? "__ex__" : ("__ex_" + varName))
                         << ".what()));\n";
                if (h.body) emitNode(*h.body);
                pop();
            }
            if (n.finallyBody) {
                out_ << ind() << "} // finally (inlined below)\n";
                emitNode(*n.finallyBody);
            } else {
                out_ << ind() << "}\n";
            }
        }
        else if constexpr (std::is_same_v<T, ImportStmt>) {
            out_ << ind() << "// import " << n.module << " (native imports not supported)\n";
        }
        else {
            // Expression used as statement
            out_ << ind() << emitExpr(node) << ";\n";
        } }, node.node);
}

void Transpiler::emitVarDecl(VarDecl &s, int /*line*/)
{
    std::string init = s.initializer ? emitExpr(*s.initializer) : "QVal()";
    out_ << ind() << "QVal " << s.name << " = " << init << ";\n";
    out_ << ind() << "__g[\"" << s.name << "\"] = " << s.name << "; (void)" << s.name << ";\n";
}

void Transpiler::emitFunctionDecl(FunctionDecl &s, int /*line*/)
{
    out_ << ind() << "QVal " << s.name << " = QVal([&](std::vector<QVal> __args) -> QVal {\n";
    push();
    // Bind parameters
    for (size_t i = 0; i < s.params.size(); i++)
        out_ << ind() << "QVal " << s.params[i] << " = __args.size() > " << i
             << " ? __args[" << i << "] : QVal();\n";
    if (s.body)
        emitNode(*s.body);
    out_ << ind() << "return QVal();\n";
    pop();
    out_ << ind() << "});\n";
    out_ << ind() << "__g[\"" << s.name << "\"] = " << s.name << ";\n";
}

void Transpiler::emitClassDecl(ClassDecl &s, int /*line*/)
{
    // Classes are represented as constructor functions returning a QDict
    out_ << ind() << "// class " << s.name << "\n";
    out_ << ind() << "QVal " << s.name << " = QVal([&](std::vector<QVal> __args) -> QVal {\n";
    push();
    out_ << ind() << "auto __self = std::make_shared<QDict>();\n";
    out_ << ind() << "QVal self = QVal(__self);\n";

    // Bind methods as dict entries
    for (auto &method : s.methods)
    {
        if (!method->is<FunctionDecl>())
            continue;
        auto &fd = method->as<FunctionDecl>();
        out_ << ind() << "(*__self)[\"" << fd.name << "\"] = QVal([&, __self](std::vector<QVal> __margs) -> QVal {\n";
        push();
        out_ << ind() << "QVal self = QVal(__self);\n";
        for (size_t i = 0; i < fd.params.size(); i++)
        {
            if (fd.params[i] == "self")
                continue; // self is already bound
            size_t argIdx = i > 0 ? i - 1 : 0;
            out_ << ind() << "QVal " << fd.params[i] << " = __margs.size() > " << argIdx
                 << " ? __margs[" << argIdx << "] : QVal();\n";
        }
        if (fd.body)
            emitNode(*fd.body);
        out_ << ind() << "return QVal();\n";
        pop();
        out_ << ind() << "});\n";
    }

    // Call init if args provided
    out_ << ind() << "if (!__args.empty()) {\n";
    push();
    out_ << ind() << "auto initIt = __self->find(\"init\");\n";
    out_ << ind() << "if (initIt == __self->end()) initIt = __self->find(\"__init__\");\n";
    out_ << ind() << "if (initIt != __self->end() && initIt->second.isFn())\n";
    out_ << ind() << "    initIt->second.asFn()(__args);\n";
    pop();
    out_ << ind() << "}\n";
    out_ << ind() << "return self;\n";
    pop();
    out_ << ind() << "});\n";
    out_ << ind() << "__g[\"" << s.name << "\"] = " << s.name << ";\n";
}

void Transpiler::emitIf(IfStmt &s, int /*line*/)
{
    out_ << ind() << "if ((" << emitExpr(*s.condition) << ").truthy()) {\n";
    push();
    emitNode(*s.thenBranch);
    pop();
    if (s.elseBranch)
    {
        out_ << ind() << "} else {\n";
        push();
        emitNode(*s.elseBranch);
        pop();
    }
    out_ << ind() << "}\n";
}

void Transpiler::emitWhile(WhileStmt &s, int /*line*/)
{
    out_ << ind() << "while ((" << emitExpr(*s.condition) << ").truthy()) {\n";
    push();
    emitNode(*s.body);
    pop();
    out_ << ind() << "}\n";
}

void Transpiler::emitFor(ForStmt &s, int /*line*/)
{
    std::string iterName = "__iter_" + std::to_string(tmpCount_++);
    out_ << ind() << "{\n";
    push();
    out_ << ind() << "auto " << iterName << " = (" << emitExpr(*s.iterable) << ");\n";
    out_ << ind() << "if (!" << iterName << ".isArr()) throw std::runtime_error(\"for: not iterable\");\n";
    out_ << ind() << "for (auto& " << s.var;
    if (!s.var2.empty())
        out_ << "__pair_";
    out_ << " : *" << iterName << ".asArr()) {\n";
    push();
    if (!s.var2.empty())
    {
        out_ << ind() << "QVal " << s.var << " = " << s.var << "__pair_.isArr() ? (*" << s.var << "__pair_.asArr())[0] : " << s.var << "__pair_;\n";
        out_ << ind() << "QVal " << s.var2 << " = " << s.var << "__pair_.isArr() && " << s.var << "__pair_.asArr()->size()>1 ? (*" << s.var << "__pair_.asArr())[1] : QVal();\n";
    }
    emitNode(*s.body);
    pop();
    out_ << ind() << "}\n";
    pop();
    out_ << ind() << "}\n";
}

void Transpiler::emitReturn(ReturnStmt &s, int /*line*/)
{
    if (s.value)
        out_ << ind() << "return " << emitExpr(*s.value) << ";\n";
    else
        out_ << ind() << "return QVal();\n";
}

void Transpiler::emitPrint(PrintStmt &s, int /*line*/)
{
    out_ << ind() << "{ std::vector<QVal> __pargs = {";
    for (size_t i = 0; i < s.args.size(); i++)
    {
        if (i)
            out_ << ", ";
        out_ << emitExpr(*s.args[i]);
    }
    out_ << "};\n";
    out_ << ind() << "  for(size_t __pi=0;__pi<__pargs.size();__pi++){if(__pi) std::cout<<\"" << escapeString(s.sep) << "\";std::cout<<__pargs[__pi].str();}\n";
    out_ << ind() << "  std::cout<<\"" << escapeString(s.end) << "\"; }\n";
}

std::string Transpiler::emitExpr(ASTNode &node)
{
    return std::visit([&](auto &n) -> std::string
                      {
        using T = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<T, NumberLiteral>)
            return "QVal(" + std::to_string(n.value) + ")";
        else if constexpr (std::is_same_v<T, StringLiteral>)
            return "QVal(std::string(\"" + escapeString(n.value) + "\"))";
        else if constexpr (std::is_same_v<T, BoolLiteral>)
            return n.value ? "QVal(true)" : "QVal(false)";
        else if constexpr (std::is_same_v<T, NilLiteral>)
            return "QVal()";
        else if constexpr (std::is_same_v<T, Identifier>) {
            return "(__g.count(\"" + n.name + "\") ? __g[\"" + n.name + "\"] : QVal())";
        }
        else if constexpr (std::is_same_v<T, BinaryExpr>)
            return emitBinary(n);
        else if constexpr (std::is_same_v<T, UnaryExpr>)
            return emitUnary(n);
        else if constexpr (std::is_same_v<T, AssignExpr>)
            return emitAssign(n);
        else if constexpr (std::is_same_v<T, CallExpr>)
            return emitCall(n);
        else if constexpr (std::is_same_v<T, IndexExpr>)
            return emitIndex(n);
        else if constexpr (std::is_same_v<T, MemberExpr>)
            return emitMember(n);
        else if constexpr (std::is_same_v<T, ArrayLiteral>)
            return emitArray(n);
        else if constexpr (std::is_same_v<T, DictLiteral>)
            return emitDict(n);
        else if constexpr (std::is_same_v<T, LambdaExpr>)
            return emitLambda(n);
        else if constexpr (std::is_same_v<T, TernaryExpr>)
            return emitTernary(n);
        else if constexpr (std::is_same_v<T, NewExpr>) {
            std::string cls = "(__g.count(\"" + n.typeName + "\") ? __g[\"" + n.typeName + "\"] : QVal())";
            std::string args = "{";
            for (size_t i = 0; i < n.args.size(); i++) {
                if (i) args += ", ";
                args += emitExpr(*n.args[i]);
            }
            args += "}";
            return "qcall(" + cls + ", " + args + ")";
        }
        else if constexpr (std::is_same_v<T, ListComp>) {
            std::string tmp = freshTmp();
            // Inline a lambda that builds the array
            std::string iter = emitExpr(*n.iterable);
            std::string result = "([&]() -> QVal { auto " + tmp + "=std::make_shared<QArr>(); auto __src_=(" + iter + "); if(__src_.isArr()) for(auto& ";
            result += n.vars.empty() ? "__lv__" : n.vars[0];
            result += " : *__src_.asArr()) {";
            if (n.condition) result += " if(!(" + emitExpr(*n.condition) + ").truthy()) continue;";
            result += " " + tmp + "->push_back(" + emitExpr(*n.expr) + "); }";
            result += " return QVal(" + tmp + "); })()";
            return result;
        }
        else if constexpr (std::is_same_v<T, SuperExpr>) {
            return "QVal(std::string(\"<super>\"))"; // simplified
        }
        else if constexpr (std::is_same_v<T, TupleLiteral>) {
            return emitArray(ArrayLiteral{[&]() { std::vector<ASTNodePtr> v; for (auto& e : n.elements) { v.push_back(std::make_unique<ASTNode>(e->node, e->line)); } return v; }()});
        }
        else {
            return "QVal()";
        } }, node.node);
}

std::string Transpiler::emitBinary(BinaryExpr &e)
{
    std::string L = emitExpr(*e.left);
    std::string R = emitExpr(*e.right);
    if (e.op == "+")
        return "(" + L + " + " + R + ")";
    if (e.op == "-")
        return "(" + L + " - " + R + ")";
    if (e.op == "*")
        return "(" + L + " * " + R + ")";
    if (e.op == "/")
        return "(" + L + " / " + R + ")";
    if (e.op == "%")
        return "(" + L + " % " + R + ")";
    if (e.op == "**")
        return "(" + L + ").pow(" + R + ")";
    if (e.op == "//")
        return "QVal(std::floor((" + L + ").asNum() / (" + R + ").asNum()))";
    if (e.op == "==")
        return "QVal((" + L + ") == (" + R + "))";
    if (e.op == "!=")
        return "QVal((" + L + ") != (" + R + "))";
    if (e.op == "<")
        return "QVal((" + L + ") < (" + R + "))";
    if (e.op == "<=")
        return "QVal((" + L + ") <= (" + R + "))";
    if (e.op == ">")
        return "QVal((" + L + ") > (" + R + "))";
    if (e.op == ">=")
        return "QVal((" + L + ") >= (" + R + "))";
    if (e.op == "and" || e.op == "&&")
        return "QVal((" + L + ").truthy() && (" + R + ").truthy())";
    if (e.op == "or" || e.op == "||")
        return "QVal((" + L + ").truthy() || (" + R + ").truthy())";
    if (e.op == "&")
        return "QVal((double)((long long)(" + L + ").asNum() & (long long)(" + R + ").asNum()))";
    if (e.op == "|")
        return "QVal((double)((long long)(" + L + ").asNum() | (long long)(" + R + ").asNum()))";
    if (e.op == "^")
        return "QVal((double)((long long)(" + L + ").asNum() ^ (long long)(" + R + ").asNum()))";
    if (e.op == "<<")
        return "QVal((double)((long long)(" + L + ").asNum() << (int)(" + R + ").asNum()))";
    if (e.op == ">>")
        return "QVal((double)((long long)(" + L + ").asNum() >> (int)(" + R + ").asNum()))";
    return "QVal()";
}

std::string Transpiler::emitUnary(UnaryExpr &e)
{
    std::string V = emitExpr(*e.operand);
    if (e.op == "-")
        return "QVal(-(" + V + ").asNum())";
    if (e.op == "!" || e.op == "not")
        return "QVal(!(" + V + ").truthy())";
    if (e.op == "~")
        return "QVal((double)(~(long long)(" + V + ").asNum()))";
    if (e.op == "++")
        return "(" + V + " = QVal((" + V + ").asNum() + 1), " + V + ")";
    if (e.op == "--")
        return "(" + V + " = QVal((" + V + ").asNum() - 1), " + V + ")";
    return V;
}

std::string Transpiler::emitAssign(AssignExpr &e)
{
    std::string val = emitExpr(*e.value);

    // Apply compound operator
    if (e.op != "=")
    {
        std::string cur = emitExpr(*e.target);
        std::string opStr;
        if (e.op == "+=")
            opStr = "+";
        else if (e.op == "-=")
            opStr = "-";
        else if (e.op == "*=")
            opStr = "*";
        else if (e.op == "/=")
            opStr = "/";
        else if (e.op == "%=")
            opStr = "%";
        if (!opStr.empty())
            val = "(" + cur + " " + opStr + " " + val + ")";
    }

    if (e.target->is<Identifier>())
    {
        std::string n = e.target->as<Identifier>().name;
        return "(__g[\"" + n + "\"] = " + val + ")";
    }
    if (e.target->is<IndexExpr>())
    {
        auto &idx = e.target->as<IndexExpr>();
        return "([&]() -> QVal { auto& __obj = " + emitExpr(*idx.object) + "; __obj[" + emitExpr(*idx.index) + "] = " + val + "; return " + val + "; })()";
    }
    if (e.target->is<MemberExpr>())
    {
        auto &mem = e.target->as<MemberExpr>();
        return "([&]() -> QVal { auto __obj = " + emitExpr(*mem.object) + "; qSetMember(__obj, \"" + mem.member + "\", " + val + ");" + " return " + val + "; })()";
    }
    return val;
}

std::string Transpiler::emitCall(CallExpr &e)
{
    // Method call: obj.method(args)
    if (e.callee->is<MemberExpr>())
    {
        auto &mem = e.callee->as<MemberExpr>();
        std::string obj = emitExpr(*mem.object);
        std::string args = "{";
        for (size_t i = 0; i < e.args.size(); i++)
        {
            if (i)
                args += ", ";
            args += emitExpr(*e.args[i]);
        }
        args += "}";
        return "([&]() -> QVal { auto __o = " + obj + "; auto __m = qGetMember(__o, \"" + mem.member + "\"); return __m.isFn() ? __m.asFn()" + args + " : __m; })()";
    }

    std::string fn = emitExpr(*e.callee);
    std::string args = "{";
    for (size_t i = 0; i < e.args.size(); i++)
    {
        if (i)
            args += ", ";
        args += emitExpr(*e.args[i]);
    }
    args += "}";
    return "qcall(" + fn + ", " + args + ")";
}

std::string Transpiler::emitIndex(IndexExpr &e)
{
    return "(" + emitExpr(*e.object) + ")[" + emitExpr(*e.index) + "]";
}

std::string Transpiler::emitMember(MemberExpr &e)
{
    return "([&]() -> QVal { auto __o = " + emitExpr(*e.object) + "; return qGetMember(__o, \"" + e.member + "\"); })()";
}

std::string Transpiler::emitArray(ArrayLiteral &e)
{
    std::string r = "QVal(std::make_shared<QArr>(QArr{";
    for (size_t i = 0; i < e.elements.size(); i++)
    {
        if (i)
            r += ", ";
        r += emitExpr(*e.elements[i]);
    }
    return r + "}))";
}

std::string Transpiler::emitDict(DictLiteral &e)
{
    std::string r = "([&]() -> QVal { auto __d = std::make_shared<QDict>(); ";
    for (auto &[k, v] : e.pairs)
        r += "(*__d)[" + emitExpr(*k) + ".str()] = " + emitExpr(*v) + "; ";
    return r + "return QVal(__d); })()";
}

std::string Transpiler::emitLambda(LambdaExpr &e)
{
    std::string r = "QVal([&](std::vector<QVal> __args) -> QVal { ";
    for (size_t i = 0; i < e.params.size(); i++)
        r += "QVal " + e.params[i] + " = __args.size() > " + std::to_string(i) + " ? __args[" + std::to_string(i) + "] : QVal(); ";
    if (e.body)
    {
        if (e.body->is<BlockStmt>())
        {
            // Can't inline a block, wrap in lambda body
            std::ostringstream tmp;
            std::swap(tmp, out_);
            int savedIndent = indent_;
            indent_ = 0;
            push();
            emitNode(*e.body);
            pop();
            indent_ = savedIndent;
            std::swap(tmp, out_);
            r += " return QVal(); })";
            return r; // simplified: block lambdas return nil
        }
        r += "return " + emitExpr(*e.body) + "; ";
    }
    return r + "return QVal(); })";
}

std::string Transpiler::emitTernary(TernaryExpr &e)
{
    return "((" + emitExpr(*e.condition) + ").truthy() ? " + emitExpr(*e.thenExpr) + " : " + emitExpr(*e.elseExpr) + ")";
}

// ─── Find C++ compiler ────────────────────────────────────────────────────────
static std::string findCXXCompiler()
{
    for (const char *cc : {"g++", "c++", "clang++"})
    {
#ifdef _WIN32
        std::string cmd = std::string("where ") + cc + " >nul 2>&1";
#else
        std::string cmd = std::string("which ") + cc + " >/dev/null 2>&1";
#endif
        if (std::system(cmd.c_str()) == 0)
            return cc;
    }
    // Common MinGW paths on Windows
    for (const char *path : {
             "C:\\msys64\\ucrt64\\bin\\g++.exe",
             "C:\\msys64\\mingw64\\bin\\g++.exe",
             "C:\\MinGW\\bin\\g++.exe"})
    {
        if (fs::exists(path))
            return path;
    }
    return "";
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc == 1)
    {
        printBanner();
        printHelp(argv[0]);
        return 0;
    }

    std::string arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h")
    {
        printBanner();
        printHelp(argv[0]);
        return 0;
    }
    if (arg1 == "--version" || arg1 == "-v")
    {
        std::cout << "Quantum Language v2.0.0 (Native Compiler)\n"
                  << "Use 'qrun' for the bytecode VM interpreter.\n"
                  << "Built By Muhammad Saad Amin\n";
        return 0;
    }

    // ── Parse flags ───────────────────────────────────────────────────────────
    std::string inputFile;
    std::string outputFile;
    bool doRun = false;
    bool emitCOnly = false;

    for (int i = 1; i < argc; i++)
    {
        std::string a = argv[i];
        if (a == "--run")
        {
            doRun = true;
        }
        else if (a == "--emit-c")
        {
            emitCOnly = true;
        }
        else if (a == "-o" && i + 1 < argc)
        {
            outputFile = argv[++i];
        }
        else if (a[0] != '-')
        {
            inputFile = a;
        }
    }

    if (inputFile.empty())
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET << "No input file specified.\n";
        return 1;
    }

    std::ifstream file(inputFile);
    if (!file.is_open())
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET << "Cannot open: " << inputFile << "\n";
        return 1;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    // ── Parse ─────────────────────────────────────────────────────────────────
    std::cout << Colors::CYAN << "[1/3] " << Colors::RESET << "Parsing " << inputFile << "...\n";
    ASTNodePtr ast;
    try
    {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        ast = parser.parse();

        try
        {
            TypeChecker checker;
            checker.check(ast);
        }
        catch (const StaticTypeError &e)
        {
            std::cerr << Colors::YELLOW << "[TypeWarning] " << Colors::RESET
                      << e.what() << " (line " << e.line << ")\n";
        }
    }
    catch (const ParseError &e)
    {
        std::cerr << Colors::RED << "[ParseError] " << Colors::RESET
                  << e.what() << " (line " << e.line << ")\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET << e.what() << "\n";
        return 1;
    }

    // ── Transpile to C++ ──────────────────────────────────────────────────────
    std::cout << Colors::CYAN << "[2/3] " << Colors::RESET << "Transpiling to C++...\n";
    Transpiler transpiler;
    std::string cppSource = transpiler.transpile(*ast);

    if (emitCOnly)
    {
        std::cout << cppSource;
        return 0;
    }

    // ── Write temp files ──────────────────────────────────────────────────────
    fs::path tmpDir = fs::temp_directory_path() / "quantum_build";
    fs::create_directories(tmpDir);
    fs::path cppFile = tmpDir / "quantum_out.cpp";
    fs::path rtFile = tmpDir / "quantum_rt.h";

    {
        std::ofstream f(cppFile);
        f << cppSource;
    }
    {
        std::ofstream f(rtFile);
        f << QUANTUM_RUNTIME_H;
    }

    // ── Determine output path ─────────────────────────────────────────────────
    if (outputFile.empty())
    {
        fs::path inp(inputFile);
        outputFile = inp.stem().string();
#ifdef _WIN32
        outputFile += ".exe";
#endif
    }

    // ── Compile with g++ ──────────────────────────────────────────────────────
    std::string cxx = findCXXCompiler();
    if (cxx.empty())
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET
                  << "No C++ compiler found. Install g++ (MinGW) and add it to PATH.\n";
        return 1;
    }

    std::cout << Colors::CYAN << "[3/3] " << Colors::RESET
              << "Compiling with " << cxx << "...\n";

    std::string compileCmd = "\"" + cxx + "\" -std=c++17 -O2 " + "\"" + cppFile.string() + "\" " + "-I\"" + tmpDir.string() + "\" " + "-o \"" + outputFile + "\" 2>&1";

    int ret = std::system(compileCmd.c_str());
    if (ret != 0)
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET
                  << "Compilation failed. Use --emit-c to inspect generated code.\n";
        return 1;
    }

    std::cout << Colors::GREEN << "\n  ✓ " << Colors::RESET
              << "Compiled: " << Colors::YELLOW << outputFile << Colors::RESET << "\n\n";

    if (doRun)
    {
        std::cout << Colors::CYAN << "Running " << outputFile << "...\n"
                  << Colors::RESET;
        return std::system(("\"" + outputFile + "\"").c_str());
    }

    return 0;
}