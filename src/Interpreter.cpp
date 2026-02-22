#include "../include/Interpreter.h"
#include "../include/Error.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <random>
#include <iomanip>
#include <functional>
#include <thread>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// ─── Helpers ─────────────────────────────────────────────────────────────────

static double toNum(const QuantumValue& v, const std::string& ctx) {
    if (v.isNumber()) return v.asNumber();
    throw TypeError("Expected number in " + ctx + ", got " + v.typeName());
}

static long long toInt(const QuantumValue& v, const std::string& ctx) {
    return (long long)toNum(v, ctx);
}

// ─── Constructor ─────────────────────────────────────────────────────────────

Interpreter::Interpreter() {
    globals = std::make_shared<Environment>();
    env = globals;
    registerNatives();
}

void Interpreter::registerNatives() {
    auto reg = [&](const std::string& name, QuantumNativeFunc fn) {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = name;
        nat->fn   = std::move(fn);
        globals->define(name, QuantumValue(nat));
    };

    // I/O
    reg("__input__", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args.empty()) std::cout << args[0].toString();
        std::string line;
        std::getline(std::cin, line);
        return QuantumValue(line);
    });

    // Type conversion
    reg("num", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("num() requires 1 argument");
        if (args[0].isNumber()) return args[0];
        if (args[0].isString()) {
            try { return QuantumValue(std::stod(args[0].asString())); }
            catch (...) { throw TypeError("Cannot convert '" + args[0].asString() + "' to number"); }
        }
        if (args[0].isBool()) return QuantumValue(args[0].asBool() ? 1.0 : 0.0);
        throw TypeError("Cannot convert to number");
    });

    reg("str", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("str() requires 1 argument");
        return QuantumValue(args[0].toString());
    });

    reg("bool", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("bool() requires 1 argument");
        return QuantumValue(args[0].isTruthy());
    });

    // Math
    reg("abs",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::abs(toNum(a[0],"abs"))); });
    reg("sqrt",  [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::sqrt(toNum(a[0],"sqrt"))); });
    reg("floor", [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::floor(toNum(a[0],"floor"))); });
    reg("ceil",  [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::ceil(toNum(a[0],"ceil"))); });
    reg("round", [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::round(toNum(a[0],"round"))); });
    reg("pow",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::pow(toNum(a[0],"pow"),toNum(a[1],"pow"))); });
    reg("log",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::log(toNum(a[0],"log"))); });
    reg("log2",  [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::log2(toNum(a[0],"log2"))); });
    reg("sin",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::sin(toNum(a[0],"sin"))); });
    reg("cos",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::cos(toNum(a[0],"cos"))); });
    reg("tan",   [](std::vector<QuantumValue> a) -> QuantumValue { return QuantumValue(std::tan(toNum(a[0],"tan"))); });
    reg("min",   [](std::vector<QuantumValue> a) -> QuantumValue {
        double m = toNum(a[0],"min");
        for (size_t i=1;i<a.size();i++) m = std::min(m, toNum(a[i],"min"));
        return QuantumValue(m);
    });
    reg("max",   [](std::vector<QuantumValue> a) -> QuantumValue {
        double m = toNum(a[0],"max");
        for (size_t i=1;i<a.size();i++) m = std::max(m, toNum(a[i],"max"));
        return QuantumValue(m);
    });

    // Constants
    globals->define("PI", QuantumValue(M_PI));
    globals->define("E",  QuantumValue(M_E));
    globals->define("INF",QuantumValue(std::numeric_limits<double>::infinity()));

    // Utility
    reg("len", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("len() requires 1 argument");
        if (args[0].isString()) return QuantumValue((double)args[0].asString().size());
        if (args[0].isArray())  return QuantumValue((double)args[0].asArray()->size());
        if (args[0].isDict())   return QuantumValue((double)args[0].asDict()->size());
        throw TypeError("len() not supported for type " + args[0].typeName());
    });

    reg("type", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("type() requires 1 argument");
        return QuantumValue(args[0].typeName());
    });

    reg("range", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("range() requires arguments");
        double start = 0, end_, step = 1;
        if (args.size() == 1) { end_ = toNum(args[0], "range"); }
        else if (args.size() == 2) { start = toNum(args[0],"range"); end_ = toNum(args[1],"range"); }
        else { start = toNum(args[0],"range"); end_ = toNum(args[1],"range"); step = toNum(args[2],"range"); }
        auto arr = std::make_shared<Array>();
        if (step > 0) for (double i = start; i < end_; i += step) arr->push_back(QuantumValue(i));
        else          for (double i = start; i > end_; i += step) arr->push_back(QuantumValue(i));
        return QuantumValue(arr);
    });

    reg("rand", [](std::vector<QuantumValue> args) -> QuantumValue {
        static std::mt19937 rng(std::random_device{}());
        if (args.size() >= 2) {
            double lo = toNum(args[0],"rand"), hi = toNum(args[1],"rand");
            std::uniform_real_distribution<double> dist(lo, hi);
            return QuantumValue(dist(rng));
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return QuantumValue(dist(rng));
    });

    reg("rand_int", [](std::vector<QuantumValue> args) -> QuantumValue {
        static std::mt19937 rng(std::random_device{}());
        double lo = args.size()>=2 ? toNum(args[0],"rand_int") : 0;
        double hi = args.size()>=2 ? toNum(args[1],"rand_int") : toNum(args[0],"rand_int");
        std::uniform_int_distribution<long long> dist((long long)lo, (long long)hi);
        return QuantumValue((double)dist(rng));
    });

    reg("time", [](std::vector<QuantumValue>) -> QuantumValue {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return QuantumValue((double)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0);
    });

    reg("sleep", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty()) throw RuntimeError("sleep() requires seconds argument");
        long long ms = (long long)(toNum(args[0],"sleep") * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return QuantumValue();
    });

    reg("exit", [](std::vector<QuantumValue> args) -> QuantumValue {
        int code = args.empty() ? 0 : (int)toNum(args[0],"exit");
        std::exit(code);
    });

    reg("assert", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.empty() || !args[0].isTruthy()) {
            std::string msg = args.size() > 1 ? args[1].toString() : "Assertion failed";
            throw RuntimeError(msg);
        }
        return QuantumValue();
    });

    // String building
    reg("chr", [](std::vector<QuantumValue> args) -> QuantumValue {
        int code = (int)toNum(args[0],"chr");
        return QuantumValue(std::string(1, (char)code));
    });

    reg("ord", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isString() || args[0].asString().empty()) throw TypeError("ord() expects non-empty string");
        return QuantumValue((double)(unsigned char)args[0].asString()[0]);
    });

    reg("hex", [](std::vector<QuantumValue> args) -> QuantumValue {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << (long long)toNum(args[0],"hex");
        return QuantumValue(oss.str());
    });

    reg("bin", [](std::vector<QuantumValue> args) -> QuantumValue {
        long long n = (long long)toNum(args[0],"bin");
        if (n == 0) return QuantumValue(std::string("0b0"));
        std::string res;
        long long tmp = n;
        while (tmp) { res = (char)('0' + (tmp & 1)) + res; tmp >>= 1; }
        return QuantumValue("0b" + res);
    });

    // Array
    reg("array", [](std::vector<QuantumValue> args) -> QuantumValue {
        auto arr = std::make_shared<Array>();
        if (!args.empty()) arr->resize((size_t)toNum(args[0],"array"), args.size()>1 ? args[1] : QuantumValue());
        return QuantumValue(arr);
    });

    reg("keys", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isDict()) throw TypeError("keys() expects dict");
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *args[0].asDict()) arr->push_back(QuantumValue(k));
        return QuantumValue(arr);
    });

    reg("values", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isDict()) throw TypeError("values() expects dict");
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *args[0].asDict()) arr->push_back(v);
        return QuantumValue(arr);
    });

    // ─── Cybersecurity builtins (future expansion base) ────────────────────
    reg("xor_bytes", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("xor_bytes() expects two strings");
        const std::string& a = args[0].asString();
        const std::string& b = args[1].asString();
        std::string result(a.size(), '\0');
        for (size_t i = 0; i < a.size(); i++)
            result[i] = a[i] ^ b[i % b.size()];
        return QuantumValue(result);
    });

    reg("to_hex", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isString()) throw TypeError("to_hex() expects string");
        std::ostringstream oss;
        for (unsigned char c : args[0].asString())
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str());
    });

    reg("from_hex", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isString()) throw TypeError("from_hex() expects string");
        const std::string& h = args[0].asString();
        std::string result;
        for (size_t i = 0; i + 1 < h.size(); i += 2)
            result += (char)std::stoi(h.substr(i, 2), nullptr, 16);
        return QuantumValue(result);
    });

    reg("rot13", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isString()) throw TypeError("rot13() expects string");
        std::string s = args[0].asString();
        for (char& c : s) {
            if (std::isalpha(c)) {
                char base = std::islower(c) ? 'a' : 'A';
                c = base + (c - base + 13) % 26;
            }
        }
        return QuantumValue(s);
    });

    reg("base64_encode", [](std::vector<QuantumValue> args) -> QuantumValue {
        if (!args[0].isString()) throw TypeError("base64_encode() expects string");
        static const char* B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const std::string& in = args[0].asString();
        std::string out;
        for (size_t i = 0; i < in.size(); i += 3) {
            unsigned char b0 = in[i], b1 = i+1<in.size()?in[i+1]:0, b2 = i+2<in.size()?in[i+2]:0;
            out += B64[b0 >> 2];
            out += B64[((b0 & 3) << 4) | (b1 >> 4)];
            out += i+1<in.size() ? B64[((b1 & 0xf) << 2) | (b2 >> 6)] : '=';
            out += i+2<in.size() ? B64[b2 & 0x3f] : '=';
        }
        return QuantumValue(out);
    });
}

// ─── Execute ─────────────────────────────────────────────────────────────────

void Interpreter::execute(ASTNode& node) {
    std::visit([&](auto& n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, BlockStmt>)      execBlock(n);
        else if constexpr (std::is_same_v<T, VarDecl>)       execVarDecl(n);
        else if constexpr (std::is_same_v<T, FunctionDecl>)  execFunctionDecl(n);
        else if constexpr (std::is_same_v<T, ClassDecl>)     execClassDecl(n);
        else if constexpr (std::is_same_v<T, IfStmt>)        execIf(n);
        else if constexpr (std::is_same_v<T, WhileStmt>)     execWhile(n);
        else if constexpr (std::is_same_v<T, ForStmt>)       execFor(n);
        else if constexpr (std::is_same_v<T, ReturnStmt>)    execReturn(n);
        else if constexpr (std::is_same_v<T, PrintStmt>)     execPrint(n);
        else if constexpr (std::is_same_v<T, ImportStmt>)    execImport(n);
        else if constexpr (std::is_same_v<T, ExprStmt>)      execExprStmt(n);
        else if constexpr (std::is_same_v<T, BreakStmt>)     throw BreakSignal{};
        else if constexpr (std::is_same_v<T, ContinueStmt>)  throw ContinueSignal{};
        else {
            // Could be an expression node used as statement
            evaluate(node);
        }
    }, node.node);
}

void Interpreter::execBlock(BlockStmt& s, std::shared_ptr<Environment> scope) {
    auto prev = env;
    env = scope ? scope : std::make_shared<Environment>(prev);
    try {
        for (auto& stmt : s.statements) execute(*stmt);
    } catch (...) {
        env = prev;
        throw;
    }
    env = prev;
}

void Interpreter::execVarDecl(VarDecl& s) {
    QuantumValue val;
    if (s.initializer) val = evaluate(*s.initializer);
    env->define(s.name, std::move(val), s.isConst);
}

void Interpreter::execFunctionDecl(FunctionDecl& s) {
    auto fn = std::make_shared<QuantumFunction>();
    fn->name    = s.name;
    fn->params  = s.params;
    fn->body    = s.body.get();
    fn->closure = env;
    env->define(s.name, QuantumValue(fn));
}

void Interpreter::execClassDecl(ClassDecl& s) {
    auto klass = std::make_shared<QuantumClass>();
    klass->name = s.name;
    for (auto& m : s.methods) {
        if (m->is<FunctionDecl>()) {
            auto& fd = m->as<FunctionDecl>();
            auto fn = std::make_shared<QuantumFunction>();
            fn->name   = fd.name;
            fn->params = fd.params;
            fn->body   = fd.body.get();
            fn->closure = env;
            klass->methods[fd.name] = fn;
        }
    }
    env->define(s.name, QuantumValue(std::make_shared<QuantumNative>(
        QuantumNative{s.name, [klass](std::vector<QuantumValue> args) -> QuantumValue {
            auto inst = std::make_shared<QuantumInstance>();
            inst->klass = klass;
            // Call __init__ if exists
            auto it = klass->methods.find("init");
            if (it != klass->methods.end()) {
                // We'd need interpreter ref here; for now store instance
            }
            return QuantumValue(inst);
        }}
    )));
}

void Interpreter::execIf(IfStmt& s) {
    if (evaluate(*s.condition).isTruthy())
        execute(*s.thenBranch);
    else if (s.elseBranch)
        execute(*s.elseBranch);
}

void Interpreter::execWhile(WhileStmt& s) {
    while (evaluate(*s.condition).isTruthy()) {
        try { execute(*s.body); }
        catch (BreakSignal&) { break; }
        catch (ContinueSignal&) { continue; }
    }
}

void Interpreter::execFor(ForStmt& s) {
    auto iter = evaluate(*s.iterable);
    auto loop = [&](QuantumValue item) {
        auto scope = std::make_shared<Environment>(env);
        scope->define(s.var, std::move(item));
        try { execBlock(s.body->as<BlockStmt>(), scope); }
        catch (BreakSignal&) { throw; }
        catch (ContinueSignal&) {}
    };
    if (iter.isArray()) {
        for (auto& item : *iter.asArray()) {
            try { loop(item); }
            catch (BreakSignal&) { break; }
        }
    } else if (iter.isString()) {
        for (char c : iter.asString()) {
            try { loop(QuantumValue(std::string(1,c))); }
            catch (BreakSignal&) { break; }
        }
    } else if (iter.isDict()) {
        for (auto& [k, v] : *iter.asDict()) {
            try { loop(QuantumValue(k)); }
            catch (BreakSignal&) { break; }
        }
    } else {
        throw TypeError("Cannot iterate over " + iter.typeName());
    }
}

void Interpreter::execReturn(ReturnStmt& s) {
    QuantumValue val;
    if (s.value) val = evaluate(*s.value);
    throw ReturnSignal(std::move(val));
}

void Interpreter::execPrint(PrintStmt& s) {
    for (size_t i = 0; i < s.args.size(); i++) {
        if (i) std::cout << " ";
        std::cout << evaluate(*s.args[i]).toString();
    }
    if (s.newline) std::cout << "\n";
    std::cout.flush();
}

void Interpreter::execImport(ImportStmt& s) {
    // Module system stub - extendable
    // For now just note it
    (void)s;
}

void Interpreter::execExprStmt(ExprStmt& s) {
    evaluate(*s.expr);
}

// ─── Evaluate ────────────────────────────────────────────────────────────────

QuantumValue Interpreter::evaluate(ASTNode& node) {
    return std::visit([&](auto& n) -> QuantumValue {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, NumberLiteral>) return QuantumValue(n.value);
        else if constexpr (std::is_same_v<T, StringLiteral>) return QuantumValue(n.value);
        else if constexpr (std::is_same_v<T, BoolLiteral>)   return QuantumValue(n.value);
        else if constexpr (std::is_same_v<T, NilLiteral>)    return QuantumValue();
        else if constexpr (std::is_same_v<T, Identifier>)    return evalIdentifier(n);
        else if constexpr (std::is_same_v<T, BinaryExpr>)    return evalBinary(n);
        else if constexpr (std::is_same_v<T, UnaryExpr>)     return evalUnary(n);
        else if constexpr (std::is_same_v<T, AssignExpr>)    return evalAssign(n);
        else if constexpr (std::is_same_v<T, CallExpr>)      return evalCall(n);
        else if constexpr (std::is_same_v<T, IndexExpr>)     return evalIndex(n);
        else if constexpr (std::is_same_v<T, MemberExpr>)    return evalMember(n);
        else if constexpr (std::is_same_v<T, ArrayLiteral>)  return evalArray(n);
        else if constexpr (std::is_same_v<T, DictLiteral>)   return evalDict(n);
        else if constexpr (std::is_same_v<T, LambdaExpr>)    return evalLambda(n);
        else {
            execute(node);
            return QuantumValue();
        }
    }, node.node);
}

QuantumValue Interpreter::evalIdentifier(Identifier& e) {
    return env->get(e.name);
}

QuantumValue Interpreter::evalBinary(BinaryExpr& e) {
    // Short-circuit for and/or
    if (e.op == "and") {
        auto lv = evaluate(*e.left);
        if (!lv.isTruthy()) return lv;
        return evaluate(*e.right);
    }
    if (e.op == "or") {
        auto lv = evaluate(*e.left);
        if (lv.isTruthy()) return lv;
        return evaluate(*e.right);
    }

    auto lv = evaluate(*e.left);
    auto rv = evaluate(*e.right);
    const std::string& op = e.op;

    if (op == "+") {
        if (lv.isString() || rv.isString()) return QuantumValue(lv.toString() + rv.toString());
        if (lv.isNumber() && rv.isNumber()) return QuantumValue(lv.asNumber() + rv.asNumber());
        if (lv.isArray() && rv.isArray()) {
            auto arr = std::make_shared<Array>(*lv.asArray());
            for (auto& x : *rv.asArray()) arr->push_back(x);
            return QuantumValue(arr);
        }
        throw TypeError("Cannot add " + lv.typeName() + " and " + rv.typeName());
    }
    if (op == "-") return QuantumValue(toNum(lv,"-") - toNum(rv,"-"));
    if (op == "*") {
        if (lv.isNumber() && rv.isNumber()) return QuantumValue(lv.asNumber() * rv.asNumber());
        if (lv.isString() && rv.isNumber()) {
            std::string s; int n = (int)rv.asNumber();
            for (int i=0;i<n;i++) s += lv.asString();
            return QuantumValue(s);
        }
        throw TypeError("Cannot multiply " + lv.typeName() + " and " + rv.typeName());
    }
    if (op == "/") {
        double d = toNum(rv,"/");
        if (d == 0) throw RuntimeError("Division by zero");
        return QuantumValue(toNum(lv,"/") / d);
    }
    if (op == "%") {
        long long b = toInt(rv,"%");
        if (b == 0) throw RuntimeError("Modulo by zero");
        return QuantumValue((double)(toInt(lv,"%") % b));
    }
    if (op == "**") return QuantumValue(std::pow(toNum(lv,"**"), toNum(rv,"**")));
    if (op == "==") {
        if (lv.isNil() && rv.isNil()) return QuantumValue(true);
        if (lv.typeName() != rv.typeName()) return QuantumValue(false);
        if (lv.isNumber()) return QuantumValue(lv.asNumber() == rv.asNumber());
        if (lv.isBool())   return QuantumValue(lv.asBool() == rv.asBool());
        if (lv.isString()) return QuantumValue(lv.asString() == rv.asString());
        return QuantumValue(false);
    }
    if (op == "!=") {
        auto eq = BinaryExpr{"==", nullptr, nullptr};
        // inline evaluate
        bool eq_res = false;
        if (lv.isNil() && rv.isNil()) eq_res = true;
        else if (lv.typeName() == rv.typeName()) {
            if (lv.isNumber()) eq_res = lv.asNumber() == rv.asNumber();
            else if (lv.isBool()) eq_res = lv.asBool() == rv.asBool();
            else if (lv.isString()) eq_res = lv.asString() == rv.asString();
        }
        return QuantumValue(!eq_res);
    }
    if (op == "<")  return QuantumValue(toNum(lv,"<")  < toNum(rv,"<"));
    if (op == ">")  return QuantumValue(toNum(lv,">")  > toNum(rv,">"));
    if (op == "<=") return QuantumValue(toNum(lv,"<=") <= toNum(rv,"<="));
    if (op == ">=") return QuantumValue(toNum(lv,">=") >= toNum(rv,">="));

    // Bitwise
    if (op == "&")  return QuantumValue((double)(toInt(lv,"&")  & toInt(rv,"&")));
    if (op == "|")  return QuantumValue((double)(toInt(lv,"|")  | toInt(rv,"|")));
    if (op == "^")  return QuantumValue((double)(toInt(lv,"^")  ^ toInt(rv,"^")));
    if (op == "<<") return QuantumValue((double)(toInt(lv,"<<") << toInt(rv,"<<")));
    if (op == ">>") return QuantumValue((double)(toInt(lv,">>") >> toInt(rv,">>")));

    throw RuntimeError("Unknown operator: " + op);
}

QuantumValue Interpreter::evalUnary(UnaryExpr& e) {
    auto v = evaluate(*e.operand);
    if (e.op == "-")   return QuantumValue(-toNum(v, "unary -"));
    if (e.op == "not") return QuantumValue(!v.isTruthy());
    if (e.op == "~")   return QuantumValue((double)~toInt(v, "~"));
    throw RuntimeError("Unknown unary op: " + e.op);
}

void Interpreter::setLValue(ASTNode& target, QuantumValue val, const std::string& op) {
    auto applyOp = [&](QuantumValue old) -> QuantumValue {
        if (op == "=")  return val;
        if (op == "+=") { if (old.isString()) return QuantumValue(old.asString()+val.toString()); return QuantumValue(toNum(old,op)+toNum(val,op)); }
        if (op == "-=") return QuantumValue(toNum(old,op)-toNum(val,op));
        if (op == "*=") return QuantumValue(toNum(old,op)*toNum(val,op));
        if (op == "/=") { double d=toNum(val,op); if(!d) throw RuntimeError("Div by 0"); return QuantumValue(toNum(old,op)/d); }
        return val;
    };

    if (target.is<Identifier>()) {
        auto& name = target.as<Identifier>().name;
        if (op == "=") env->set(name, std::move(val));
        else env->set(name, applyOp(env->get(name)));
    } else if (target.is<IndexExpr>()) {
        auto& ie = target.as<IndexExpr>();
        auto obj = evaluate(*ie.object);
        auto idx = evaluate(*ie.index);
        if (obj.isArray()) {
            int i = (int)toNum(idx, "index");
            auto arr = obj.asArray();
            if (i < 0) i += arr->size();
            if (i < 0 || i >= (int)arr->size()) throw IndexError("Array index out of range");
            (*arr)[i] = applyOp((*arr)[i]);
        } else if (obj.isDict()) {
            std::string key = idx.toString();
            auto dict = obj.asDict();
            (*dict)[key] = applyOp(dict->count(key) ? (*dict)[key] : QuantumValue());
        }
    } else if (target.is<MemberExpr>()) {
        auto& me = target.as<MemberExpr>();
        auto obj = evaluate(*me.object);
        if (obj.isInstance()) {
            auto inst = obj.asInstance();
            auto cur = inst->fields.count(me.member) ? inst->fields[me.member] : QuantumValue();
            inst->setField(me.member, applyOp(cur));
        } else if (obj.isDict()) {
            (*obj.asDict())[me.member] = applyOp(obj.asDict()->count(me.member) ? (*obj.asDict())[me.member] : QuantumValue());
        }
    }
}

QuantumValue Interpreter::evalAssign(AssignExpr& e) {
    auto val = evaluate(*e.value);
    setLValue(*e.target, val, e.op);
    return val;
}

QuantumValue Interpreter::evalCall(CallExpr& e) {
    // Special: method call via MemberExpr callee
    if (e.callee->is<MemberExpr>()) {
        auto& me = e.callee->as<MemberExpr>();
        auto obj = evaluate(*me.object);
        std::vector<QuantumValue> args;
        for (auto& a : e.args) args.push_back(evaluate(*a));
        return callMethod(obj, me.member, std::move(args));
    }

    auto callee = evaluate(*e.callee);
    std::vector<QuantumValue> args;
    for (auto& a : e.args) args.push_back(evaluate(*a));

    if (callee.isFunction()) {
        if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(callee.data))
            return callFunction(callee.asFunction(), std::move(args));
        if (std::holds_alternative<std::shared_ptr<QuantumNative>>(callee.data))
            return callNative(callee.asNative(), std::move(args));
    }
    throw TypeError("Cannot call " + callee.typeName());
}

QuantumValue Interpreter::callFunction(std::shared_ptr<QuantumFunction> fn, std::vector<QuantumValue> args) {
    auto scope = std::make_shared<Environment>(fn->closure);
    for (size_t i = 0; i < fn->params.size(); i++) {
        QuantumValue v = i < args.size() ? args[i] : QuantumValue();
        scope->define(fn->params[i], std::move(v));
    }
    try {
        execBlock(fn->body->as<BlockStmt>(), scope);
    } catch (ReturnSignal& r) {
        return std::move(r.value);
    }
    return QuantumValue();
}

QuantumValue Interpreter::callNative(std::shared_ptr<QuantumNative> fn, std::vector<QuantumValue> args) {
    return fn->fn(std::move(args));
}

QuantumValue Interpreter::evalIndex(IndexExpr& e) {
    auto obj = evaluate(*e.object);
    auto idx = evaluate(*e.index);
    if (obj.isArray()) {
        int i = (int)toNum(idx, "index");
        auto arr = obj.asArray();
        if (i < 0) i += arr->size();
        if (i < 0 || i >= (int)arr->size()) throw IndexError("Array index " + std::to_string(i) + " out of range");
        return (*arr)[i];
    }
    if (obj.isDict()) {
        auto dict = obj.asDict();
        auto key = idx.toString();
        auto it = dict->find(key);
        if (it == dict->end()) return QuantumValue();
        return it->second;
    }
    if (obj.isString()) {
        int i = (int)toNum(idx, "index");
        const auto& s = obj.asString();
        if (i < 0) i += s.size();
        if (i < 0 || i >= (int)s.size()) throw IndexError("String index out of range");
        return QuantumValue(std::string(1, s[i]));
    }
    throw TypeError("Cannot index " + obj.typeName());
}

QuantumValue Interpreter::evalMember(MemberExpr& e) {
    auto obj = evaluate(*e.object);
    if (obj.isInstance()) return obj.asInstance()->getField(e.member);
    if (obj.isDict()) {
        auto dict = obj.asDict();
        auto it = dict->find(e.member);
        return it != dict->end() ? it->second : QuantumValue();
    }
    // String len property
    if (obj.isString() && e.member == "length") return QuantumValue((double)obj.asString().size());
    if (obj.isArray() && e.member == "length")  return QuantumValue((double)obj.asArray()->size());
    throw TypeError("No member '" + e.member + "' on " + obj.typeName());
}

QuantumValue Interpreter::evalArray(ArrayLiteral& e) {
    auto arr = std::make_shared<Array>();
    for (auto& el : e.elements) arr->push_back(evaluate(*el));
    return QuantumValue(arr);
}

QuantumValue Interpreter::evalDict(DictLiteral& e) {
    auto dict = std::make_shared<Dict>();
    for (auto& [k, v] : e.pairs) {
        auto key = evaluate(*k);
        auto val = evaluate(*v);
        (*dict)[key.toString()] = std::move(val);
    }
    return QuantumValue(dict);
}

QuantumValue Interpreter::evalLambda(LambdaExpr& e) {
    auto fn = std::make_shared<QuantumFunction>();
    fn->name    = "<lambda>";
    fn->params  = e.params;
    fn->body    = e.body.get();
    fn->closure = env;
    return QuantumValue(fn);
}

// ─── Built-in Method Dispatch ────────────────────────────────────────────────

QuantumValue Interpreter::callMethod(QuantumValue& obj, const std::string& method, std::vector<QuantumValue> args) {
    if (obj.isArray())  return callArrayMethod(obj.asArray(), method, std::move(args));
    if (obj.isString()) return callStringMethod(obj.asString(), method, std::move(args));
    if (obj.isDict())   return callDictMethod(obj.asDict(), method, std::move(args));
    if (obj.isInstance()) {
        auto inst = obj.asInstance();
        auto field = inst->getField(method);
        if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(field.data)) {
            auto fn = field.asFunction();
            auto scope = std::make_shared<Environment>(fn->closure);
            scope->define("self", obj);
            for (size_t i = 0; i < fn->params.size(); i++) {
                std::string param = fn->params[i];
                if (param == "self") continue;
                size_t ai = (fn->params[0] == "self") ? i-1 : i;
                scope->define(param, ai < args.size() ? args[ai] : QuantumValue());
            }
            try { execBlock(fn->body->as<BlockStmt>(), scope); }
            catch (ReturnSignal& r) { return r.value; }
            return QuantumValue();
        }
    }
    throw TypeError("No method '" + method + "' on " + obj.typeName());
}

QuantumValue Interpreter::callArrayMethod(std::shared_ptr<Array> arr, const std::string& m, std::vector<QuantumValue> args) {
    if (m == "push")    { for (auto& a : args) arr->push_back(a); return QuantumValue(); }
    if (m == "pop")     { if (arr->empty()) throw IndexError("pop() on empty array"); auto v = arr->back(); arr->pop_back(); return v; }
    if (m == "shift")   { if (arr->empty()) throw IndexError("shift() on empty array"); auto v = arr->front(); arr->erase(arr->begin()); return v; }
    if (m == "unshift") { arr->insert(arr->begin(), args.begin(), args.end()); return QuantumValue(); }
    if (m == "length")  return QuantumValue((double)arr->size());
    if (m == "reverse") { std::reverse(arr->begin(), arr->end()); return QuantumValue(); }
    if (m == "contains"){ for (auto& v : *arr) { BinaryExpr eq{"==",nullptr,nullptr}; /* simplified */ if (v.toString() == args[0].toString()) return QuantumValue(true); } return QuantumValue(false); }
    if (m == "join") {
        std::string sep = args.empty() ? "," : args[0].toString();
        std::string res;
        for (size_t i = 0; i < arr->size(); i++) { if (i) res += sep; res += (*arr)[i].toString(); }
        return QuantumValue(res);
    }
    if (m == "slice") {
        int start = args.empty() ? 0 : (int)toNum(args[0],"slice");
        int end_  = args.size()>1 ? (int)toNum(args[1],"slice") : (int)arr->size();
        if (start < 0) start += arr->size();
        if (end_  < 0) end_  += arr->size();
        start = std::max(0, std::min(start, (int)arr->size()));
        end_  = std::max(0, std::min(end_,  (int)arr->size()));
        auto res = std::make_shared<Array>(arr->begin()+start, arr->begin()+end_);
        return QuantumValue(res);
    }
    if (m == "map") {
        if (args.empty()) throw RuntimeError("map() requires function argument");
        auto res = std::make_shared<Array>();
        for (auto& item : *arr) {
            std::vector<QuantumValue> callArgs = {item};
            QuantumValue fn = args[0];
            QuantumValue r;
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                r = callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                r = callNative(fn.asNative(), callArgs);
            res->push_back(r);
        }
        return QuantumValue(res);
    }
    if (m == "filter") {
        if (args.empty()) throw RuntimeError("filter() requires function argument");
        auto res = std::make_shared<Array>();
        for (auto& item : *arr) {
            std::vector<QuantumValue> callArgs = {item};
            QuantumValue fn = args[0];
            QuantumValue r;
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                r = callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                r = callNative(fn.asNative(), callArgs);
            if (r.isTruthy()) res->push_back(item);
        }
        return QuantumValue(res);
    }
    if (m == "sort") {
        std::sort(arr->begin(), arr->end(), [](const QuantumValue& a, const QuantumValue& b) {
            if (a.isNumber() && b.isNumber()) return a.asNumber() < b.asNumber();
            return a.toString() < b.toString();
        });
        return QuantumValue();
    }
    throw TypeError("Array has no method '" + m + "'");
}

QuantumValue Interpreter::callStringMethod(const std::string& str, const std::string& m, std::vector<QuantumValue> args) {
    if (m == "length" || m == "len") return QuantumValue((double)str.size());
    if (m == "upper") { std::string r=str; std::transform(r.begin(),r.end(),r.begin(),::toupper); return QuantumValue(r); }
    if (m == "lower") { std::string r=str; std::transform(r.begin(),r.end(),r.begin(),::tolower); return QuantumValue(r); }
    if (m == "trim")  {
        size_t s = str.find_first_not_of(" \t\n\r");
        size_t e = str.find_last_not_of(" \t\n\r");
        if (s == std::string::npos) return QuantumValue(std::string(""));
        return QuantumValue(str.substr(s, e-s+1));
    }
    if (m == "split") {
        std::string sep = args.empty() ? " " : args[0].toString();
        auto arr = std::make_shared<Array>();
        if (sep.empty()) { for (char c : str) arr->push_back(QuantumValue(std::string(1,c))); return QuantumValue(arr); }
        size_t pos = 0, found;
        while ((found = str.find(sep, pos)) != std::string::npos) {
            arr->push_back(QuantumValue(str.substr(pos, found-pos)));
            pos = found + sep.size();
        }
        arr->push_back(QuantumValue(str.substr(pos)));
        return QuantumValue(arr);
    }
    if (m == "contains") { return QuantumValue(str.find(args[0].toString()) != std::string::npos); }
    if (m == "starts_with") { auto p=args[0].toString(); return QuantumValue(str.size()>=p.size() && str.substr(0,p.size())==p); }
    if (m == "ends_with")   { auto p=args[0].toString(); return QuantumValue(str.size()>=p.size() && str.substr(str.size()-p.size())==p); }
    if (m == "replace") {
        if (args.size() < 2) throw RuntimeError("replace() requires 2 args");
        std::string from = args[0].toString(), to = args[1].toString();
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.size(), to);
            pos += to.size();
        }
        return QuantumValue(result);
    }
    if (m == "slice" || m == "substr") {
        int start = args.empty() ? 0 : (int)toNum(args[0],"slice");
        int len_  = args.size()>1 ? (int)toNum(args[1],"slice") : (int)str.size()-start;
        if (start < 0) start += str.size();
        return QuantumValue(str.substr(std::max(0,start), len_));
    }
    if (m == "index") {
        auto sub = args[0].toString();
        auto p = str.find(sub);
        return QuantumValue(p == std::string::npos ? QuantumValue(-1.0) : QuantumValue((double)p));
    }
    if (m == "repeat") {
        int n = (int)toNum(args[0],"repeat");
        std::string res;
        for (int i = 0; i < n; i++) res += str;
        return QuantumValue(res);
    }
    if (m == "chars") {
        auto arr = std::make_shared<Array>();
        for (char c : str) arr->push_back(QuantumValue(std::string(1,c)));
        return QuantumValue(arr);
    }
    throw TypeError("String has no method '" + m + "'");
}

QuantumValue Interpreter::callDictMethod(std::shared_ptr<Dict> dict, const std::string& m, std::vector<QuantumValue> args) {
    if (m == "has" || m == "contains") return QuantumValue(dict->count(args[0].toString()) > 0);
    if (m == "get") {
        auto key = args[0].toString();
        auto it = dict->find(key);
        if (it != dict->end()) return it->second;
        return args.size() > 1 ? args[1] : QuantumValue();
    }
    if (m == "set")    { (*dict)[args[0].toString()] = args[1]; return QuantumValue(); }
    if (m == "delete") { dict->erase(args[0].toString()); return QuantumValue(); }
    if (m == "keys") {
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *dict) arr->push_back(QuantumValue(k));
        return QuantumValue(arr);
    }
    if (m == "values") {
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *dict) arr->push_back(v);
        return QuantumValue(arr);
    }
    if (m == "length" || m == "size") return QuantumValue((double)dict->size());
    throw TypeError("Dict has no method '" + m + "'");
}
