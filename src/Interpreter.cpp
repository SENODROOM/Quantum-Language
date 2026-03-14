#include "../include/Interpreter.h"
#include "../include/Error.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <limits>
#include <fstream>
#include <chrono>
#include <ctime>
#include <random>
#include <iomanip>
#include <functional>
#include <thread>
#include <regex>

// Windows crypto headers (must come before any use of CryptGenRandom etc.)
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Token buffer for cin >> : C++ cin >> reads whitespace-delimited tokens,
// but std::getline reads a whole line. We buffer leftover tokens here so that
// "cin >> a; cin >> b;" with input "5 10\n" correctly gives a=5, b=10.
static std::vector<std::string> s_cinBuffer;

static double toNum(const QuantumValue &v, const std::string &ctx)
{
    if (v.isNumber())
        return v.asNumber();
    throw TypeError("Expected number in " + ctx + ", got " + v.typeName());
}

static long long toInt(const QuantumValue &v, const std::string &ctx)
{
    return (long long)toNum(v, ctx);
}

// ─── Format Engine ───────────────────────────────────────────────────────────
// Shared by printf(), format(), and sprintf()-style calls.
// Supports: %d %i %u %f %e %g %s %c %x %X %o %b %%
// Flags:    - (left-align)  + (force sign)  0 (zero-pad)  space
// Width:    %8d   %-10s
// Precision:%6.2f  %.5s (truncate string)

static std::string applyFormat(const std::string &fmt, const std::vector<QuantumValue> &args, size_t argStart = 1)
{
    std::string out;
    size_t argIdx = argStart;
    size_t i = 0;

    auto nextArg = [&]() -> QuantumValue
    {
        return argIdx < args.size() ? args[argIdx++] : QuantumValue();
    };

    while (i < fmt.size())
    {
        if (fmt[i] != '%')
        {
            out += fmt[i++];
            continue;
        }
        ++i;
        if (i >= fmt.size())
            break;
        if (fmt[i] == '%')
        {
            out += '%';
            ++i;
            continue;
        }

        // ── Collect flags ──────────────────────────────────────────────────
        bool flagMinus = false, flagPlus = false, flagSpace = false, flagZero = false, flagHash = false;
        while (i < fmt.size())
        {
            char f = fmt[i];
            if (f == '-')
            {
                flagMinus = true;
                ++i;
            }
            else if (f == '+')
            {
                flagPlus = true;
                ++i;
            }
            else if (f == ' ')
            {
                flagSpace = true;
                ++i;
            }
            else if (f == '0')
            {
                flagZero = true;
                ++i;
            }
            else if (f == '#')
            {
                flagHash = true;
                ++i;
            }
            else
                break;
        }

        // ── Width ──────────────────────────────────────────────────────────
        int width = 0;
        while (i < fmt.size() && std::isdigit(fmt[i]))
            width = width * 10 + (fmt[i++] - '0');

        // ── Precision ─────────────────────────────────────────────────────
        int prec = -1;
        if (i < fmt.size() && fmt[i] == '.')
        {
            ++i;
            prec = 0;
            while (i < fmt.size() && std::isdigit(fmt[i]))
                prec = prec * 10 + (fmt[i++] - '0');
        }

        if (i >= fmt.size())
            break;
        char conv = fmt[i++];
        QuantumValue arg = nextArg();

        // ── Pad helper ────────────────────────────────────────────────────
        auto pad = [&](std::string s, bool numericSign = false) -> std::string
        {
            if (width > 0 && (int)s.size() < width)
            {
                int pad = width - (int)s.size();
                if (flagMinus)
                    s += std::string(pad, ' ');
                else if (flagZero && numericSign)
                    s = std::string(pad, '0') + s;
                else
                    s = std::string(pad, ' ') + s;
            }
            return s;
        };

        char buf[256];
        switch (conv)
        {

        // ── Integer specifiers ────────────────────────────────────────────
        case 'd':
        case 'i':
        {
            long long n = arg.isNumber() ? (long long)arg.asNumber() : 0LL;
            std::string s;
            if (prec >= 0)
            {
                // precision on integers = minimum digits
                std::snprintf(buf, sizeof(buf), ("%0*lld"), prec, std::abs(n));
                s = buf;
                if (n < 0)
                    s = "-" + s;
                else if (flagPlus)
                    s = "+" + s;
                else if (flagSpace)
                    s = " " + s;
            }
            else
            {
                std::snprintf(buf, sizeof(buf), "%lld", n);
                s = buf;
                if (n >= 0 && flagPlus)
                    s = "+" + s;
                else if (n >= 0 && flagSpace)
                    s = " " + s;
            }
            out += pad(s, true);
            break;
        }
        case 'u':
        {
            unsigned long long n = arg.isNumber() ? (unsigned long long)(long long)arg.asNumber() : 0ULL;
            std::snprintf(buf, sizeof(buf), "%llu", n);
            out += pad(std::string(buf), true);
            break;
        }

        // ── Float specifiers ──────────────────────────────────────────────
        case 'f':
        case 'F':
        {
            double d = arg.isNumber() ? arg.asNumber() : 0.0;
            std::string spec = "%";
            if (flagPlus)
                spec += '+';
            else if (flagSpace)
                spec += ' ';
            if (prec >= 0)
                spec += "." + std::to_string(prec);
            spec += 'f';
            std::snprintf(buf, sizeof(buf), spec.c_str(), d);
            out += pad(std::string(buf), true);
            break;
        }
        case 'e':
        case 'E':
        {
            double d = arg.isNumber() ? arg.asNumber() : 0.0;
            std::string spec = "%";
            if (prec >= 0)
                spec += "." + std::to_string(prec);
            spec += conv;
            std::snprintf(buf, sizeof(buf), spec.c_str(), d);
            out += pad(std::string(buf), true);
            break;
        }
        case 'g':
        case 'G':
        {
            double d = arg.isNumber() ? arg.asNumber() : 0.0;
            std::string spec = "%";
            if (prec >= 0)
                spec += "." + std::to_string(prec);
            spec += conv;
            std::snprintf(buf, sizeof(buf), spec.c_str(), d);
            out += pad(std::string(buf), true);
            break;
        }

        // ── String specifier ─────────────────────────────────────────────
        case 's':
        {
            std::string s = arg.toString();
            if (prec >= 0 && (int)s.size() > prec)
                s = s.substr(0, prec); // truncate to precision
            out += pad(s, false);
            break;
        }

        // ── Char specifier ────────────────────────────────────────────────
        case 'c':
        {
            char c = arg.isString() && !arg.asString().empty()
                         ? arg.asString()[0]
                         : (char)(arg.isNumber() ? (int)arg.asNumber() : 0);
            out += pad(std::string(1, c), false);
            break;
        }

        // ── Hex specifiers ────────────────────────────────────────────────
        case 'x':
        case 'X':
        {
            unsigned long long n = (unsigned long long)(long long)(arg.isNumber() ? arg.asNumber() : 0.0);
            std::string spec = "%";
            if (flagHash)
                spec += '#';
            if (prec >= 0)
                spec += "." + std::to_string(prec);
            spec += (conv == 'x') ? "llx" : "llX";
            std::snprintf(buf, sizeof(buf), spec.c_str(), n);
            std::string s = buf;
            // prefix 0x/0X if not already there (flagHash adds it)
            if (flagHash && n != 0 && s.substr(0, 2) != "0x" && s.substr(0, 2) != "0X")
                s = (conv == 'x' ? "0x" : "0X") + s;
            out += pad(s, true);
            break;
        }

        // ── Octal specifier ───────────────────────────────────────────────
        case 'o':
        {
            unsigned long long n = (unsigned long long)(long long)(arg.isNumber() ? arg.asNumber() : 0.0);
            std::snprintf(buf, sizeof(buf), flagHash ? "%#llo" : "%llo", n);
            out += pad(std::string(buf), true);
            break;
        }

        // ── Binary specifier (non-standard, Quantum extension) ────────────
        case 'b':
        {
            long long n = arg.isNumber() ? (long long)arg.asNumber() : 0LL;
            if (n == 0)
            {
                out += pad(flagHash ? "0b0" : "0", true);
            }
            else
            {
                std::string bits;
                unsigned long long u = (unsigned long long)n;
                while (u)
                {
                    bits = (char)('0' + (u & 1)) + bits;
                    u >>= 1;
                }
                if (flagHash)
                    bits = "0b" + bits;
                out += pad(bits, true);
            }
            break;
        }

        // ── Boolean specifier (Quantum extension) ─────────────────────────
        case 'B':
        {
            out += pad(arg.isTruthy() ? "true" : "false", false);
            break;
        }

        // ── Type name specifier (Quantum extension) ───────────────────────
        case 't':
        {
            out += pad(arg.typeName(), false);
            break;
        }

        default:
            out += '%';
            out += conv;
            break;
        }
    }
    return out;
}

// ─── Constructor ─────────────────────────────────────────────────────────────

Interpreter::Interpreter()
{
    globals = std::make_shared<Environment>();
    env = globals;
    stepCount_ = 0;
    s_cinBuffer.clear(); // reset between interpreter runs so stale tokens don't leak
    registerNatives();
}

void Interpreter::registerNatives()
{
    auto reg = [&](const std::string &name, QuantumNativeFunc fn)
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = name;
        nat->fn = std::move(fn);
        globals->define(name, QuantumValue(nat));
    };

    // I/O
    reg("__input__", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty()) std::cout << args[0].toString();
        std::string line;
        std::getline(std::cin, line);
        return QuantumValue(line); });

    reg("input", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty()) std::cout << args[0].toString();
        std::string line;
        std::getline(std::cin, line);
        return QuantumValue(line); });

    reg("scanf", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("scanf() requires at least a format string");
        std::string fmt = args[0].isString() ? args[0].asString() : args[0].toString();

        // Collect pointer targets (skip format string)
        std::vector<std::shared_ptr<QuantumPointer>> targets;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i].isPointer()) {
                targets.push_back(args[i].asPointer());
            }
        }

        // Read one line of input
        std::string input;
        std::getline(std::cin, input);
        // Trim trailing CR/whitespace
        while (!input.empty() && (input.back() == '\r' || input.back() == ' ' || input.back() == '\t'))
            input.pop_back();

        // Parse format specifiers and write values back through pointers
        size_t targetIdx = 0;
        std::istringstream iss(input);
        size_t fi = 0;
        while (fi < fmt.size() && targetIdx < targets.size()) {
            if (fmt[fi] == '%' && fi + 1 < fmt.size()) {
                char spec = fmt[fi + 1];
                fi += 2;
                auto& ptr = targets[targetIdx++];
                if (!ptr || ptr->isNull()) continue;
                if (spec == 'd' || spec == 'i') {
                    long long v = 0; iss >> v;
                    ptr->deref() = QuantumValue((double)v);
                } else if (spec == 'f' || spec == 'g' || spec == 'e') {
                    double v = 0; iss >> v;
                    ptr->deref() = QuantumValue(v);
                } else if (spec == 's') {
                    std::string v; iss >> v;
                    ptr->deref() = QuantumValue(v);
                } else if (spec == 'c') {
                    char c = 0; iss >> c;
                    ptr->deref() = QuantumValue(std::string(1, c));
                } else {
                    std::string v; iss >> v;
                    ptr->deref() = QuantumValue(v);
                }
            } else {
                fi++;
            }
        }

        if (targets.empty()) return QuantumValue(input);
        return QuantumValue(); });

    // Type conversion
    reg("num", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("num() requires 1 argument");
        if (args[0].isNumber()) return args[0];
        if (args[0].isString()) {
            try { return QuantumValue(std::stod(args[0].asString())); }
            catch (...) { throw TypeError("Cannot convert '" + args[0].asString() + "' to number"); }
        }
        if (args[0].isBool()) return QuantumValue(args[0].asBool() ? 1.0 : 0.0);
        throw TypeError("Cannot convert to number"); });

    reg("str", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("str() requires 1 argument");
        return QuantumValue(args[0].toString()); });

    reg("bool", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("bool() requires 1 argument");
        return QuantumValue(args[0].isTruthy()); });

    reg("setw", [](std::vector<QuantumValue> args) -> QuantumValue
        { return QuantumValue(std::string("")); });

    reg("setprecision", [](std::vector<QuantumValue> args) -> QuantumValue
        { return QuantumValue(std::string("")); });

    // Dummy 'fixed' global variable
    globals->define("fixed", QuantumValue(std::string("")));

    // Math
    reg("abs", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::abs(toNum(a[0], "abs"))); });
    reg("sqrt", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::sqrt(toNum(a[0], "sqrt"))); });
    reg("floor", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::floor(toNum(a[0], "floor"))); });
    reg("ceil", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::ceil(toNum(a[0], "ceil"))); });
    reg("round", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::round(toNum(a[0], "round"))); });
    reg("pow", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::pow(toNum(a[0], "pow"), toNum(a[1], "pow"))); });
    reg("acos", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::acos(toNum(a[0], "acos"))); });
    reg("log", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::log(toNum(a[0], "log"))); });
    reg("log2", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::log2(toNum(a[0], "log2"))); });
    reg("sin", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::sin(toNum(a[0], "sin"))); });
    reg("cos", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::cos(toNum(a[0], "cos"))); });
    reg("tan", [](std::vector<QuantumValue> a) -> QuantumValue
        { return QuantumValue(std::tan(toNum(a[0], "tan"))); });
    reg("min", [](std::vector<QuantumValue> a) -> QuantumValue
        {
        if (a.empty()) throw RuntimeError("min() expected at least 1 argument");
        // min(array)  — single iterable argument
        if (a.size() == 1 && a[0].isArray()) {
            auto &arr = *a[0].asArray();
            if (arr.empty()) throw RuntimeError("min() arg is an empty sequence");
            double m = toNum(arr[0], "min");
            for (size_t i = 1; i < arr.size(); i++) m = std::min(m, toNum(arr[i], "min"));
            return QuantumValue(m);
        }
        // min(a, b, c, ...)  — multiple numeric arguments
        double m = toNum(a[0], "min");
        for (size_t i = 1; i < a.size(); i++) m = std::min(m, toNum(a[i], "min"));
        return QuantumValue(m); });
    reg("max", [](std::vector<QuantumValue> a) -> QuantumValue
        {
        if (a.empty()) throw RuntimeError("max() expected at least 1 argument");
        // max(array)  — single iterable argument
        if (a.size() == 1 && a[0].isArray()) {
            auto &arr = *a[0].asArray();
            if (arr.empty()) throw RuntimeError("max() arg is an empty sequence");
            double m = toNum(arr[0], "max");
            for (size_t i = 1; i < arr.size(); i++) m = std::max(m, toNum(arr[i], "max"));
            return QuantumValue(m);
        }
        // max(a, b, c, ...)  — multiple numeric arguments
        double m = toNum(a[0], "max");
        for (size_t i = 1; i < a.size(); i++) m = std::max(m, toNum(a[i], "max"));
        return QuantumValue(m); });

    // Constants
    globals->define("PI", QuantumValue(M_PI));
    globals->define("E", QuantumValue(M_E));
    globals->define("INF", QuantumValue(std::numeric_limits<double>::infinity()));
    // JavaScript compatibility aliases
    globals->define("null", QuantumValue());
    globals->define("undefined", QuantumValue());
    globals->define("NaN", QuantumValue(std::numeric_limits<double>::quiet_NaN()));
    // Python dunder globals
    globals->define("__name__", QuantumValue(std::string("__main__")));
    globals->define("__file__", QuantumValue(std::string("")));
    globals->define("__doc__", QuantumValue());
    globals->define("__package__", QuantumValue());
    globals->define("__spec__", QuantumValue());

    // Utility
    reg("len", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("len() requires 1 argument");
        if (args[0].isString()) return QuantumValue((double)args[0].asString().size());
        if (args[0].isArray())  return QuantumValue((double)args[0].asArray()->size());
        if (args[0].isDict())   return QuantumValue((double)args[0].asDict()->size());
        throw TypeError("len() not supported for type " + args[0].typeName()); });

    reg("type", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("type() requires 1 argument");
        return QuantumValue(args[0].typeName()); });

    // isinstance(val, type_name_str_or_type)
    // Supports Python-style: isinstance(x, int), isinstance(x, str), isinstance(x, list), etc.
    reg("isinstance", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2) throw RuntimeError("isinstance() requires 2 arguments");
        auto &val = args[0];
        // Second arg: string type name, or a native/class constructor whose name we check
        std::string typeName;
        if (args[1].isString())
            typeName = args[1].asString();
        else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(args[1].data))
            typeName = args[1].asNative()->name;
        else
            typeName = args[1].typeName();

        // Normalise Python type names → Quantum type names
        auto matches = [&](const std::string &t) -> bool
        {
            if (t == "int" || t == "float" || t == "number" || t == "double")
                return val.isNumber();
            if (t == "str" || t == "string")
                return val.isString();
            if (t == "bool")
                return val.isBool();
            if (t == "list" || t == "array" || t == "tuple")
                return val.isArray();
            if (t == "dict")
                return val.isDict();
            if (t == "NoneType" || t == "nil")
                return val.isNil();
            if (t == "function" || t == "callable")
                return val.isFunction();
            return val.typeName() == t;
        };
        return QuantumValue(matches(typeName)); });

    // id() — Python identity function (returns a fake unique number here)
    reg("id", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("id() requires 1 argument");
        // Return a stable pseudo-id based on pointer or value
        return QuantumValue(0.0); });

    reg("range", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("range() requires arguments");
        double start = 0, end_, step = 1;
        if (args.size() == 1) { end_ = toNum(args[0], "range"); }
        else if (args.size() == 2) { start = toNum(args[0],"range"); end_ = toNum(args[1],"range"); }
        else { start = toNum(args[0],"range"); end_ = toNum(args[1],"range"); step = toNum(args[2],"range"); }
        auto arr = std::make_shared<Array>();
        if (step > 0) for (double i = start; i < end_; i += step) arr->push_back(QuantumValue(i));
        else          for (double i = start; i > end_; i += step) arr->push_back(QuantumValue(i));
        return QuantumValue(arr); });

    // enumerate(iterable, start=0) → array of [index, value] pairs
    reg("enumerate", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("enumerate() requires an argument");
        double startIdx = args.size() > 1 ? toNum(args[1], "enumerate") : 0;
        auto result = std::make_shared<Array>();
        auto addPair = [&](double i, QuantumValue v) {
            auto pair = std::make_shared<Array>();
            pair->push_back(QuantumValue(i));
            pair->push_back(std::move(v));
            result->push_back(QuantumValue(pair));
        };
        if (args[0].isArray()) {
            double i = startIdx;
            for (auto &v : *args[0].asArray()) addPair(i++, v);
        } else if (args[0].isString()) {
            double i = startIdx;
            for (char c : args[0].asString()) addPair(i++, QuantumValue(std::string(1,c)));
        }
        return QuantumValue(result); });

    // sum(iterable, start=0)
    reg("sum", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("sum() requires an argument");
        double total = args.size() > 1 ? toNum(args[1], "sum") : 0.0;
        if (args[0].isArray())
            for (auto &v : *args[0].asArray()) total += toNum(v, "sum");
        return QuantumValue(total); });

    // any(iterable) / all(iterable)
    reg("any", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isArray()) return QuantumValue(false);
        for (auto &v : *args[0].asArray()) if (v.isTruthy()) return QuantumValue(true);
        return QuantumValue(false); });
    reg("all", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isArray()) return QuantumValue(true);
        for (auto &v : *args[0].asArray()) if (!v.isTruthy()) return QuantumValue(false);
        return QuantumValue(true); });

    // sorted(iterable, reverse=False)
    reg("sorted", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("sorted() requires an argument");
        std::shared_ptr<Array> arr;
        if (args[0].isArray()) arr = std::make_shared<Array>(*args[0].asArray());
        else { arr = std::make_shared<Array>(); }
        bool rev = args.size() > 1 && args[1].isTruthy();
        std::sort(arr->begin(), arr->end(), [&](const QuantumValue &a, const QuantumValue &b) {
            if (a.isNumber() && b.isNumber())
                return rev ? a.asNumber() > b.asNumber() : a.asNumber() < b.asNumber();
            return rev ? a.toString() > b.toString() : a.toString() < b.toString();
        });
        return QuantumValue(arr); });

    reg("rand", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        static std::mt19937 rng(std::random_device{}());
        if (args.size() >= 2) {
            double lo = toNum(args[0],"rand"), hi = toNum(args[1],"rand");
            std::uniform_real_distribution<double> dist(lo, hi);
            return QuantumValue(dist(rng));
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return QuantumValue(dist(rng)); });

    reg("rand_int", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        static std::mt19937 rng(std::random_device{}());
        double lo = args.size()>=2 ? toNum(args[0],"rand_int") : 0;
        double hi = args.size()>=2 ? toNum(args[1],"rand_int") : toNum(args[0],"rand_int");
        std::uniform_int_distribution<long long> dist((long long)lo, (long long)hi);
        return QuantumValue((double)dist(rng)); });

    reg("time", [](std::vector<QuantumValue>) -> QuantumValue
        {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return QuantumValue((double)std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0); });

    reg("sleep", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("sleep() requires seconds argument");
        long long ms = (long long)(toNum(args[0],"sleep") * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return QuantumValue(); });

    reg("exit", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        int code = args.empty() ? 0 : (int)toNum(args[0],"exit");
        std::exit(code); });

    reg("assert", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isTruthy()) {
            std::string msg = args.size() > 1 ? args[1].toString() : "Assertion failed";
            throw RuntimeError(msg);
        }
        return QuantumValue(); });

    // String building
    reg("chr", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        int code = (int)toNum(args[0],"chr");
        return QuantumValue(std::string(1, (char)code)); });

    reg("ord", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args[0].isString() || args[0].asString().empty()) throw TypeError("ord() expects non-empty string");
        return QuantumValue((double)(unsigned char)args[0].asString()[0]); });

    reg("hex", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << (long long)toNum(args[0],"hex");
        return QuantumValue(oss.str()); });

    reg("bin", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        long long n = (long long)toNum(args[0],"bin");
        if (n == 0) return QuantumValue(std::string("0b0"));
        std::string res;
        long long tmp = n;
        while (tmp) { res = (char)('0' + (tmp & 1)) + res; tmp >>= 1; }
        return QuantumValue("0b" + res); });

    // Array
    reg("array", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        auto arr = std::make_shared<Array>();
        if (!args.empty()) arr->resize((size_t)toNum(args[0],"array"), args.size()>1 ? args[1] : QuantumValue());
        return QuantumValue(arr); });

    reg("keys", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args[0].isDict()) throw TypeError("keys() expects dict");
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *args[0].asDict()) arr->push_back(QuantumValue(k));
        return QuantumValue(arr); });

    reg("values", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args[0].isDict()) throw TypeError("values() expects dict");
        auto arr = std::make_shared<Array>();
        for (auto& [k,v] : *args[0].asDict()) arr->push_back(v);
        return QuantumValue(arr); });

    // ═══════════════════════════════════════════════════════════════════════════
    // ─── Cybersecurity Builtins ────────────────────────────────────────────────
    // ═══════════════════════════════════════════════════════════════════════════
    //
    //  Encoding / decoding
    //    to_hex(s)            → hex string of bytes
    //    from_hex(s)          → raw bytes string from hex
    //    base64_encode(s)     → base64 encoded string
    //    base64_decode(s)     → decoded bytes string
    //
    //  XOR / legacy
    //    xor_bytes(a, b)      → XOR of two byte strings (repeating key)
    //    rot13(s)             → ROT-13 (kept for compat; not a security primitive)
    //
    //  Hashing  (pure C++17, no external deps)
    //    sha256(s)            → hex digest (256-bit)
    //    sha1(s)              → hex digest (160-bit)
    //    md5(s)               → hex digest (128-bit)
    //    hmac_sha256(key, msg)→ hex digest of HMAC-SHA256
    //
    //  Symmetric encryption
    //    aes128_ecb_encrypt(key16, plaintext)  → ciphertext string (PKCS#7 padded)
    //    aes128_ecb_decrypt(key16, ciphertext) → plaintext string
    //    vigenere_encrypt(key, plain)          → Vigenère ciphertext
    //    vigenere_decrypt(key, cipher)         → Vigenère plaintext
    //
    //  Secure randomness
    //    secure_random_bytes(n) → n random bytes as raw string
    //    secure_random_hex(n)   → n random bytes as hex string
    //    secure_random_int(lo, hi) → cryptographically random integer in [lo,hi]
    //
    //  Utilities
    //    constant_time_eq(a, b)  → bool, timing-safe string comparison
    //    pkcs7_pad(s, blocksize) → PKCS#7 padded string
    //    pkcs7_unpad(s)          → PKCS#7 unpadded string
    //    luhn_check(s)           → bool, Luhn algorithm validation
    //    entropy(s)              → Shannon entropy (bits per byte)
    //
    // ─────────────────────────────────────────────────────────────────────────

    // ── Helpers (lambdas visible only in this scope) ──────────────────────────

    // SHA-256 implementation (FIPS 180-4)
    auto sha256_impl = [](const std::string &msg) -> std::string
    {
        static const uint32_t K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        auto rotr = [](uint32_t x, int n)
        { return (x >> n) | (x << (32 - n)); };
        auto ch = [](uint32_t e, uint32_t f, uint32_t g)
        { return (e & f) ^ (~e & g); };
        auto maj = [](uint32_t a, uint32_t b, uint32_t c)
        { return (a & b) ^ (a & c) ^ (b & c); };
        auto sig0 = [&](uint32_t x)
        { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
        auto sig1 = [&](uint32_t x)
        { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
        auto gam0 = [&](uint32_t x)
        { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
        auto gam1 = [&](uint32_t x)
        { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };

        // Pre-processing
        std::vector<uint8_t> data(msg.begin(), msg.end());
        uint64_t bit_len = (uint64_t)data.size() * 8;
        data.push_back(0x80);
        while (data.size() % 64 != 56)
            data.push_back(0x00);
        for (int i = 7; i >= 0; i--)
            data.push_back((uint8_t)(bit_len >> (i * 8)));

        uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                         0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        for (size_t i = 0; i < data.size(); i += 64)
        {
            uint32_t w[64];
            for (int j = 0; j < 16; j++)
                w[j] = (data[i + j * 4] << 24) | (data[i + j * 4 + 1] << 16) | (data[i + j * 4 + 2] << 8) | data[i + j * 4 + 3];
            for (int j = 16; j < 64; j++)
                w[j] = gam1(w[j - 2]) + w[j - 7] + gam0(w[j - 15]) + w[j - 16];
            uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int j = 0; j < 64; j++)
            {
                uint32_t t1 = hh + sig1(e) + ch(e, f, g) + K[j] + w[j];
                uint32_t t2 = sig0(a) + maj(a, b, c);
                hh = g;
                g = f;
                f = e;
                e = d + t1;
                d = c;
                c = b;
                b = a;
                a = t1 + t2;
            }
            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }
        std::ostringstream oss;
        for (int i = 0; i < 8; i++)
            oss << std::hex << std::setw(8) << std::setfill('0') << h[i];
        return oss.str();
    };

    // SHA-1 implementation (FIPS 180-4)
    auto sha1_impl = [](const std::string &msg) -> std::string
    {
        auto rotl = [](uint32_t x, int n)
        { return (x << n) | (x >> (32 - n)); };
        std::vector<uint8_t> data(msg.begin(), msg.end());
        uint64_t bit_len = (uint64_t)data.size() * 8;
        data.push_back(0x80);
        while (data.size() % 64 != 56)
            data.push_back(0);
        for (int i = 7; i >= 0; i--)
            data.push_back((uint8_t)(bit_len >> (i * 8)));
        uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
        for (size_t i = 0; i < data.size(); i += 64)
        {
            uint32_t w[80];
            for (int j = 0; j < 16; j++)
                w[j] = (data[i + j * 4] << 24) | (data[i + j * 4 + 1] << 16) | (data[i + j * 4 + 2] << 8) | data[i + j * 4 + 3];
            for (int j = 16; j < 80; j++)
                w[j] = rotl(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);
            uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
            for (int j = 0; j < 80; j++)
            {
                uint32_t f, k;
                if (j < 20)
                {
                    f = (b & c) | (~b & d);
                    k = 0x5A827999;
                }
                else if (j < 40)
                {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (j < 60)
                {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                }
                else
                {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }
                uint32_t t = rotl(a, 5) + f + e + k + w[j];
                e = d;
                d = c;
                c = rotl(b, 30);
                b = a;
                a = t;
            }
            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
        }
        std::ostringstream oss;
        for (int i = 0; i < 5; i++)
            oss << std::hex << std::setw(8) << std::setfill('0') << h[i];
        return oss.str();
    };

    // MD5 implementation (RFC 1321)
    auto md5_impl = [](const std::string &msg) -> std::string
    {
        static const uint32_t S[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                       5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
        static const uint32_t K_[64] = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
        auto rotl = [](uint32_t x, int n)
        { return (x << n) | (x >> (32 - n)); };
        std::vector<uint8_t> data(msg.begin(), msg.end());
        uint64_t bit_len = (uint64_t)data.size() * 8;
        data.push_back(0x80);
        while (data.size() % 64 != 56)
            data.push_back(0);
        for (int i = 0; i < 8; i++)
            data.push_back((uint8_t)(bit_len >> (i * 8)));
        uint32_t a0 = 0x67452301, b0 = 0xefcdab89, c0 = 0x98badcfe, d0 = 0x10325476;
        for (size_t i = 0; i < data.size(); i += 64)
        {
            uint32_t M[16];
            for (int j = 0; j < 16; j++)
                M[j] = (data[i + j * 4]) | (data[i + j * 4 + 1] << 8) | (data[i + j * 4 + 2] << 16) | (data[i + j * 4 + 3] << 24);
            uint32_t A = a0, B = b0, C = c0, D = d0;
            for (int j = 0; j < 64; j++)
            {
                uint32_t F, g;
                if (j < 16)
                {
                    F = (B & C) | (~B & D);
                    g = j;
                }
                else if (j < 32)
                {
                    F = (D & B) | (~D & C);
                    g = (5 * j + 1) % 16;
                }
                else if (j < 48)
                {
                    F = B ^ C ^ D;
                    g = (3 * j + 5) % 16;
                }
                else
                {
                    F = C ^ (B | (~D));
                    g = (7 * j) % 16;
                }
                F = F + A + K_[j] + M[g];
                A = D;
                D = C;
                C = B;
                B = B + rotl(F, S[j]);
            }
            a0 += A;
            b0 += B;
            c0 += C;
            d0 += D;
        }
        std::ostringstream oss;
        auto le = [&](uint32_t x)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << (x & 0xff)
                << std::hex << std::setw(2) << std::setfill('0') << ((x >> 8) & 0xff)
                << std::hex << std::setw(2) << std::setfill('0') << ((x >> 16) & 0xff)
                << std::hex << std::setw(2) << std::setfill('0') << ((x >> 24) & 0xff);
        };
        le(a0);
        le(b0);
        le(c0);
        le(d0);
        return oss.str();
    };

    // AES-128 ECB (FIPS 197) — pure C++17, no external deps
    // Uses compile-time S-box and key schedule
    auto aes128_ecb = [](const std::string &key, const std::string &plaintext, bool encrypt_mode) -> std::string
    {
        if (key.size() != 16)
            throw RuntimeError("aes128_ecb: key must be exactly 16 bytes");

        static const uint8_t SBOX[256] = {
            0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
            0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
            0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
            0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
            0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
            0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
            0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
            0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
            0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
            0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
            0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
            0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
            0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
            0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
            0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
            0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
        static const uint8_t INV_SBOX[256] = {
            0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
            0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
            0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
            0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
            0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
            0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
            0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
            0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
            0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
            0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
            0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
            0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
            0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
            0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
            0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
            0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};
        static const uint8_t RCON[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
        auto xtime = [](uint8_t a) -> uint8_t
        { return (a << 1) ^ ((a >> 7) ? 0x1b : 0); };
        auto gmul = [&](uint8_t a, uint8_t b) -> uint8_t
        {
            uint8_t r=0;for(int i=0;i<8;i++){if(b&1)r^=a;a=xtime(a);b>>=1;}return r; };

        // Key expansion
        uint8_t rk[11][16];
        for (int i = 0; i < 16; i++)
            rk[0][i] = (uint8_t)key[i];
        for (int r = 1; r <= 10; r++)
        {
            uint8_t t[4] = {rk[r - 1][13], rk[r - 1][14], rk[r - 1][15], rk[r - 1][12]};
            for (int i = 0; i < 4; i++)
                t[i] = SBOX[t[i]];
            t[0] ^= RCON[r];
            for (int i = 0; i < 4; i++)
                rk[r][i] = rk[r - 1][i] ^ t[i];
            for (int i = 4; i < 16; i++)
                rk[r][i] = rk[r - 1][i] ^ rk[r][i - 4];
        }

        auto encrypt_block = [&](uint8_t s[16])
        {
            // AddRoundKey
            for (int i = 0; i < 16; i++)
                s[i] ^= rk[0][i];
            for (int r = 1; r <= 10; r++)
            {
                // SubBytes
                for (int i = 0; i < 16; i++)
                    s[i] = SBOX[s[i]];
                // ShiftRows
                uint8_t t;
                t = s[1];
                s[1] = s[5];
                s[5] = s[9];
                s[9] = s[13];
                s[13] = t;
                t = s[2];
                s[2] = s[10];
                s[10] = t;
                t = s[6];
                s[6] = s[14];
                s[14] = t;
                t = s[15];
                s[15] = s[11];
                s[11] = s[7];
                s[7] = s[3];
                s[3] = t;
                // MixColumns (skip on last round)
                if (r < 10)
                {
                    for (int c = 0; c < 4; c++)
                    {
                        uint8_t a = s[c * 4], b = s[c * 4 + 1], cc = s[c * 4 + 2], d = s[c * 4 + 3];
                        s[c * 4] = gmul(a, 2) ^ gmul(b, 3) ^ cc ^ d;
                        s[c * 4 + 1] = a ^ gmul(b, 2) ^ gmul(cc, 3) ^ d;
                        s[c * 4 + 2] = a ^ b ^ gmul(cc, 2) ^ gmul(d, 3);
                        s[c * 4 + 3] = gmul(a, 3) ^ b ^ cc ^ gmul(d, 2);
                    }
                }
                // AddRoundKey
                for (int i = 0; i < 16; i++)
                    s[i] ^= rk[r][i];
            }
        };

        auto decrypt_block = [&](uint8_t s[16])
        {
            for (int i = 0; i < 16; i++)
                s[i] ^= rk[10][i];
            for (int r = 9; r >= 0; r--)
            {
                // InvShiftRows
                uint8_t t;
                t = s[13];
                s[13] = s[9];
                s[9] = s[5];
                s[5] = s[1];
                s[1] = t;
                t = s[2];
                s[2] = s[10];
                s[10] = t;
                t = s[6];
                s[6] = s[14];
                s[14] = t;
                t = s[3];
                s[3] = s[7];
                s[7] = s[11];
                s[11] = s[15];
                s[15] = t;
                // InvSubBytes
                for (int i = 0; i < 16; i++)
                    s[i] = INV_SBOX[s[i]];
                // AddRoundKey
                for (int i = 0; i < 16; i++)
                    s[i] ^= rk[r][i];
                // InvMixColumns (skip on round 0)
                if (r > 0)
                {
                    for (int c = 0; c < 4; c++)
                    {
                        uint8_t a = s[c * 4], b = s[c * 4 + 1], cc = s[c * 4 + 2], d = s[c * 4 + 3];
                        s[c * 4] = gmul(a, 0x0e) ^ gmul(b, 0x0b) ^ gmul(cc, 0x0d) ^ gmul(d, 0x09);
                        s[c * 4 + 1] = gmul(a, 0x09) ^ gmul(b, 0x0e) ^ gmul(cc, 0x0b) ^ gmul(d, 0x0d);
                        s[c * 4 + 2] = gmul(a, 0x0d) ^ gmul(b, 0x09) ^ gmul(cc, 0x0e) ^ gmul(d, 0x0b);
                        s[c * 4 + 3] = gmul(a, 0x0b) ^ gmul(b, 0x0d) ^ gmul(cc, 0x09) ^ gmul(d, 0x0e);
                    }
                }
            }
        };

        std::string result;
        if (encrypt_mode)
        {
            // PKCS#7 padding
            std::string padded = plaintext;
            uint8_t pad = (uint8_t)(16 - (padded.size() % 16));
            padded.append(pad, (char)pad);
            for (size_t i = 0; i < padded.size(); i += 16)
            {
                uint8_t block[16];
                for (int j = 0; j < 16; j++)
                    block[j] = (uint8_t)padded[i + j];
                encrypt_block(block);
                for (int j = 0; j < 16; j++)
                    result += (char)block[j];
            }
        }
        else
        {
            if (plaintext.size() % 16 != 0)
                throw RuntimeError("aes128_ecb_decrypt: ciphertext length must be multiple of 16");
            for (size_t i = 0; i < plaintext.size(); i += 16)
            {
                uint8_t block[16];
                for (int j = 0; j < 16; j++)
                    block[j] = (uint8_t)plaintext[i + j];
                decrypt_block(block);
                for (int j = 0; j < 16; j++)
                    result += (char)block[j];
            }
            // Remove PKCS#7 padding
            if (!result.empty())
            {
                uint8_t pad = (uint8_t)result.back();
                if (pad > 0 && pad <= 16)
                    result.resize(result.size() - pad);
            }
        }
        return result;
    };

    // ── Register all cybersec natives ────────────────────────────────────────

    // Encoding
    reg("to_hex", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("to_hex() expects string");
        std::ostringstream oss;
        for (unsigned char c : args[0].asString())
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str()); });

    reg("from_hex", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("from_hex() expects string");
        const std::string& h = args[0].asString();
        std::string result;
        for (size_t i = 0; i + 1 < h.size(); i += 2)
            result += (char)std::stoi(h.substr(i, 2), nullptr, 16);
        return QuantumValue(result); });

    reg("base64_encode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("base64_encode() expects string");
        static const char* B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const std::string& in = args[0].asString();
        std::string out;
        for (size_t i = 0; i < in.size(); i += 3) {
            unsigned char b0 = in[i], b1 = i+1<in.size()?(unsigned char)in[i+1]:0, b2 = i+2<in.size()?(unsigned char)in[i+2]:0;
            out += B64[b0 >> 2];
            out += B64[((b0 & 3) << 4) | (b1 >> 4)];
            out += i+1<in.size() ? B64[((b1 & 0xf) << 2) | (b2 >> 6)] : '=';
            out += i+2<in.size() ? B64[b2 & 0x3f] : '=';
        }
        return QuantumValue(out); });

    reg("base64_decode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("base64_decode() expects string");
        static const int8_t T[256] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1, 0,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        };
        const std::string& in = args[0].asString();
        std::string out;
        int buf=0, bits=0;
        for(unsigned char c : in){
            if(c=='=') break;
            int v=T[c]; if(v<0) continue;
            buf=(buf<<6)|v; bits+=6;
            if(bits>=8){ out+=(char)((buf>>(bits-8))&0xff); bits-=8; }
        }
        return QuantumValue(out); });

    // XOR
    reg("xor_bytes", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("xor_bytes() expects two strings");
        const std::string& a = args[0].asString();
        const std::string& b = args[1].asString();
        std::string result(a.size(), '\0');
        for (size_t i = 0; i < a.size(); i++)
            result[i] = a[i] ^ b[i % b.size()];
        return QuantumValue(result); });

    // Legacy / compat
    reg("rot13", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("rot13() expects string");
        std::string s = args[0].asString();
        for (char& c : s) {
            if (std::isalpha(c)) {
                char base = std::islower(c) ? 'a' : 'A';
                c = base + (c - base + 13) % 26;
            }
        }
        return QuantumValue(s); });

    // Hashing
    reg("sha256", [sha256_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("sha256() expects string");
        return QuantumValue(sha256_impl(args[0].asString())); });

    reg("sha1", [sha1_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("sha1() expects string");
        return QuantumValue(sha1_impl(args[0].asString())); });

    reg("md5", [md5_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("md5() expects string");
        return QuantumValue(md5_impl(args[0].asString())); });

    reg("hmac_sha256", [sha256_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("hmac_sha256(key, msg) expects two strings");
        std::string key = args[0].asString();
        const std::string& msg = args[1].asString();
        // Normalize key to 64 bytes (SHA-256 block size)
        if (key.size() > 64) key = std::string(sha256_impl(key)); // hex is 64 chars — good
        key.resize(64, '\0');
        std::string o_key(64,'\0'), i_key(64,'\0');
        for(int i=0;i<64;i++){o_key[i]=key[i]^0x5c; i_key[i]=key[i]^0x36;}
        // Convert inner SHA result (hex string) to raw bytes for outer hash
        std::string inner_hex = sha256_impl(i_key + msg);
        std::string inner_raw;
        for(size_t i=0;i+1<inner_hex.size();i+=2)
            inner_raw+=(char)std::stoi(inner_hex.substr(i,2),nullptr,16);
        return QuantumValue(sha256_impl(o_key + inner_raw)); });

    // Symmetric encryption
    reg("aes128_ecb_encrypt", [aes128_ecb](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("aes128_ecb_encrypt(key, plaintext) expects two strings");
        return QuantumValue(aes128_ecb(args[0].asString(), args[1].asString(), true)); });

    reg("aes128_ecb_decrypt", [aes128_ecb](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("aes128_ecb_decrypt(key, ciphertext) expects two strings");
        return QuantumValue(aes128_ecb(args[0].asString(), args[1].asString(), false)); });

    reg("vigenere_encrypt", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("vigenere_encrypt(key, plaintext) expects two strings");
        const std::string& key = args[0].asString();
        const std::string& plain = args[1].asString();
        if (key.empty()) throw RuntimeError("vigenere_encrypt: key must not be empty");
        std::string result;
        size_t ki = 0;
        for (char c : plain) {
            if (std::isalpha(c)) {
                char base = std::islower(c) ? 'a' : 'A';
                char kb = std::tolower(key[ki % key.size()]);
                result += (char)(base + (c - base + (kb - 'a')) % 26);
                ki++;
            } else result += c;
        }
        return QuantumValue(result); });

    reg("vigenere_decrypt", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("vigenere_decrypt(key, ciphertext) expects two strings");
        const std::string& key = args[0].asString();
        const std::string& cipher = args[1].asString();
        if (key.empty()) throw RuntimeError("vigenere_decrypt: key must not be empty");
        std::string result;
        size_t ki = 0;
        for (char c : cipher) {
            if (std::isalpha(c)) {
                char base = std::islower(c) ? 'a' : 'A';
                char kb = std::tolower(key[ki % key.size()]);
                result += (char)(base + ((c - base) - (kb - 'a') + 26) % 26);
                ki++;
            } else result += c;
        }
        return QuantumValue(result); });

    // Secure randomness (OS CSPRNG)
    reg("secure_random_bytes", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("secure_random_bytes(n) requires 1 argument");
        size_t n = (size_t)toNum(args[0], "secure_random_bytes");
        std::string result(n, '\0');
// Use /dev/urandom on POSIX, or CryptGenRandom on Windows
#if defined(_WIN32)
            HCRYPTPROV hProv;
            if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
                throw RuntimeError("secure_random_bytes: CryptAcquireContext failed");
            CryptGenRandom(hProv, (DWORD)n, (BYTE*)result.data());
            CryptReleaseContext(hProv, 0);
#else
            std::ifstream urandom("/dev/urandom", std::ios::binary);
            if (!urandom) throw RuntimeError("secure_random_bytes: could not open /dev/urandom");
            urandom.read(result.data(), n);
#endif
        return QuantumValue(result); });

    reg("secure_random_hex", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("secure_random_hex(n) requires 1 argument");
        size_t n = (size_t)toNum(args[0], "secure_random_hex");
        std::string raw(n, '\0');
#if defined(_WIN32)
            HCRYPTPROV hProv;
            if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
                throw RuntimeError("secure_random_hex: CryptAcquireContext failed");
            CryptGenRandom(hProv, (DWORD)n, (BYTE*)raw.data());
            CryptReleaseContext(hProv, 0);
#else
            std::ifstream urandom("/dev/urandom", std::ios::binary);
            if (!urandom) throw RuntimeError("secure_random_hex: could not open /dev/urandom");
            urandom.read(raw.data(), n);
#endif
        std::ostringstream oss;
        for (unsigned char c : raw) oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str()); });

    reg("secure_random_int", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2) throw RuntimeError("secure_random_int(lo, hi) requires 2 arguments");
        long long lo = (long long)toNum(args[0], "secure_random_int");
        long long hi = (long long)toNum(args[1], "secure_random_int");
        if (lo > hi) throw RuntimeError("secure_random_int: lo must be <= hi");
        uint64_t raw = 0;
#if defined(_WIN32)
            HCRYPTPROV hProv;
            CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
            CryptGenRandom(hProv, 8, (BYTE*)&raw);
            CryptReleaseContext(hProv, 0);
#else
            std::ifstream urandom("/dev/urandom", std::ios::binary);
            urandom.read((char*)&raw, 8);
#endif
        uint64_t range = (uint64_t)(hi - lo + 1);
        return QuantumValue((double)(lo + (long long)(raw % range))); });

    // Timing-safe comparison (constant-time)
    reg("constant_time_eq", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("constant_time_eq(a, b) expects two strings");
        const std::string& a = args[0].asString();
        const std::string& b = args[1].asString();
        // Compare all bytes even if lengths differ to avoid length oracle
        size_t maxlen = std::max(a.size(), b.size());
        unsigned char result = (unsigned char)(a.size() ^ b.size()); // nonzero if different lengths
        for (size_t i = 0; i < maxlen; i++) {
            unsigned char ca = i < a.size() ? (unsigned char)a[i] : 0;
            unsigned char cb = i < b.size() ? (unsigned char)b[i] : 0;
            result |= ca ^ cb;
        }
        return QuantumValue(result == 0); });

    // PKCS#7 padding
    reg("pkcs7_pad", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString())
            throw TypeError("pkcs7_pad(data, blocksize) expects string and number");
        const std::string& data = args[0].asString();
        int bs = (int)toNum(args[1], "pkcs7_pad");
        if (bs < 1 || bs > 255) throw RuntimeError("pkcs7_pad: blocksize must be 1-255");
        uint8_t pad = (uint8_t)(bs - (data.size() % bs));
        std::string result = data;
        result.append(pad, (char)pad);
        return QuantumValue(result); });

    reg("pkcs7_unpad", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("pkcs7_unpad() expects string");
        std::string s = args[0].asString();
        if (s.empty()) return QuantumValue(s);
        uint8_t pad = (uint8_t)s.back();
        if (pad == 0 || pad > s.size()) throw RuntimeError("pkcs7_unpad: invalid padding");
        for (size_t i = s.size() - pad; i < s.size(); i++)
            if ((uint8_t)s[i] != pad) throw RuntimeError("pkcs7_unpad: invalid padding bytes");
        s.resize(s.size() - pad);
        return QuantumValue(s); });

    // Luhn algorithm (credit card / ID validation)
    reg("luhn_check", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("luhn_check() expects string");
        const std::string& s = args[0].asString();
        int sum = 0; bool alt = false;
        for (int i = (int)s.size()-1; i >= 0; i--) {
            if (!std::isdigit(s[i])) continue;
            int d = s[i] - '0';
            if (alt) { d *= 2; if (d > 9) d -= 9; }
            sum += d; alt = !alt;
        }
        return QuantumValue(sum % 10 == 0); });

    // Shannon entropy
    reg("entropy", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("entropy() expects string");
        const std::string& s = args[0].asString();
        if (s.empty()) return QuantumValue(0.0);
        int freq[256] = {};
        for (unsigned char c : s) freq[c]++;
        double e = 0.0;
        for (int f : freq) {
            if (f > 0) { double p = (double)f / s.size(); e -= p * std::log2(p); }
        }
        return QuantumValue(e); });

    // ═══════════════════════════════════════════════════════════════════════════
    // ─── Extended Hacking / Offensive Security Builtins ───────────────────────
    // ═══════════════════════════════════════════════════════════════════════════
    //
    //  String / byte manipulation
    //    str_reverse(s)              → reverse a string
    //    str_to_bits(s)              → binary bit string of every byte
    //    bits_to_str(s)              → parse binary bit string back to bytes
    //    byte_at(s, i)               → integer value of byte at index i
    //    bytes_to_ints(s)            → array of byte integers
    //    ints_to_bytes(arr)          → string from array of integers
    //    zero_pad(s, n)              → left-pad string with null bytes to length n
    //
    //  Encoding attacks
    //    url_encode(s)               → percent-encode a string
    //    url_decode(s)               → decode percent-encoded string
    //    html_encode(s)              → escape <>&"' for HTML injection prevention / detection
    //    html_decode(s)              → unescape HTML entities
    //    double_url_encode(s)        → double percent-encode (bypass WAF filters)
    //    hex_encode_all(s)           → encode every char as %XX (full hex URL encoding)
    //
    //  Hashing extras
    //    sha512(s)                   → SHA-512 hex digest (512-bit)
    //    crc32(s)                    → CRC-32 checksum as hex string
    //    hash_file_sim(chunks)       → rolling SHA-256 over array of string chunks
    //
    //  Crypto extras
    //    aes128_cbc_encrypt(key,iv,pt) → AES-128 CBC encrypt (PKCS#7)
    //    aes128_cbc_decrypt(key,iv,ct) → AES-128 CBC decrypt
    //    rc4(key, data)              → RC4 (Arcfour) stream cipher
    //    chacha20_block(key32,nonce12,counter) → single ChaCha20 keystream block (64 bytes)
    //
    //  Password / credential attacks
    //    wordlist_hash(words, algo)  → array of {word, hash} dicts — for demo/CTF use
    //    crack_hash(target, words, algo) → find plaintext from wordlist that matches hash
    //    gen_wordlist_mutations(word) → common password mutations of a base word
    //
    //  Network / protocol helpers
    //    ip_to_int(ip)               → IPv4 string → 32-bit integer
    //    int_to_ip(n)                → 32-bit integer → IPv4 string
    //    ip_in_cidr(ip, cidr)        → check if IP is inside a CIDR range
    //    cidr_hosts(cidr)            → array of all host IPs in a /prefix
    //    parse_http_request(raw)     → dict {method, path, version, headers, body}
    //    build_http_request(method,host,path,headers,body) → raw HTTP/1.1 string
    //
    //  Forensics / analysis
    //    find_pattern(data, pattern) → array of byte offsets where pattern appears
    //    byte_histogram(data)        → dict of byte→count for all bytes in data
    //    ic_score(text)              → index of coincidence (language / cipher detector)
    //    hamming_distance(a, b)      → bit-level Hamming distance between two strings
    //    edit_distance(a, b)         → Levenshtein edit distance
    //
    //  Encoding / obfuscation
    //    str_to_hex_escape(s)        → "\x41\x42..." escape sequence string
    //    str_to_unicode_escape(s)    → "\u0041\u0042..." escape sequence string
    //    obfuscate_js(s)             → wrap string as JS char-code concat expression
    //
    //  Number theory (RSA building blocks)
    //    is_prime(n)                 → Miller-Rabin primality test
    //    gcd(a, b)                   → greatest common divisor
    //    mod_pow(base, exp, mod)     → fast modular exponentiation
    //    mod_inv(a, m)               → modular multiplicative inverse (extended Euclidean)
    //
    // ─────────────────────────────────────────────────────────────────────────

    // ── String / byte manipulation ────────────────────────────────────────────

    reg("str_reverse", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("str_reverse() expects string");
        std::string s = args[0].asString();
        std::reverse(s.begin(), s.end());
        return QuantumValue(s); });

    reg("str_to_bits", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("str_to_bits() expects string");
        const std::string& s = args[0].asString();
        std::string out;
        for (unsigned char c : s)
            for (int i = 7; i >= 0; i--) out += (char)('0' + ((c >> i) & 1));
        return QuantumValue(out); });

    reg("bits_to_str", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("bits_to_str() expects string");
        const std::string& b = args[0].asString();
        std::string out;
        for (size_t i = 0; i + 7 < b.size(); i += 8) {
            unsigned char c = 0;
            for (int j = 0; j < 8; j++) c = (c << 1) | (b[i+j] == '1' ? 1 : 0);
            out += (char)c;
        }
        return QuantumValue(out); });

    reg("byte_at", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString()) throw TypeError("byte_at(s, i) expects string and index");
        const std::string& s = args[0].asString();
        int idx = (int)toNum(args[1], "byte_at");
        if (idx < 0) idx = (int)s.size() + idx;
        if (idx < 0 || (size_t)idx >= s.size()) throw RuntimeError("byte_at: index out of range");
        return QuantumValue((double)(unsigned char)s[idx]); });

    reg("bytes_to_ints", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("bytes_to_ints() expects string");
        auto arr = std::make_shared<Array>();
        for (unsigned char c : args[0].asString())
            arr->push_back(QuantumValue((double)c));
        return QuantumValue(arr); });

    reg("ints_to_bytes", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isArray()) throw TypeError("ints_to_bytes() expects array");
        std::string out;
        for (auto& v : *args[0].asArray())
            out += (char)(unsigned char)(int)toNum(v, "ints_to_bytes");
        return QuantumValue(out); });

    reg("zero_pad", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString()) throw TypeError("zero_pad(s, n) expects string and length");
        std::string s = args[0].asString();
        size_t n = (size_t)toNum(args[1], "zero_pad");
        if (s.size() < n) s = std::string(n - s.size(), '\0') + s;
        return QuantumValue(s); });

    // ── Encoding attacks ──────────────────────────────────────────────────────

    reg("url_encode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("url_encode() expects string");
        const std::string& s = args[0].asString();
        std::ostringstream oss;
        for (unsigned char c : s) {
            if (std::isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~')
                oss << c;
            else
                oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        return QuantumValue(oss.str()); });

    reg("url_decode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("url_decode() expects string");
        const std::string& s = args[0].asString();
        std::string out;
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '%' && i+2 < s.size() && std::isxdigit(s[i+1]) && std::isxdigit(s[i+2])) {
                out += (char)std::stoi(s.substr(i+1,2), nullptr, 16);
                i += 2;
            } else if (s[i] == '+') {
                out += ' ';
            } else {
                out += s[i];
            }
        }
        return QuantumValue(out); });

    reg("html_encode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("html_encode() expects string");
        const std::string& s = args[0].asString();
        std::string out;
        for (char c : s) {
            switch (c) {
                case '&':  out += "&amp;";  break;
                case '<':  out += "&lt;";   break;
                case '>':  out += "&gt;";   break;
                case '"':  out += "&quot;"; break;
                case '\'': out += "&#39;";  break;
                default:   out += c;
            }
        }
        return QuantumValue(out); });

    reg("html_decode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("html_decode() expects string");
        std::string s = args[0].asString();
        auto replace_all = [](std::string& str, const std::string& from, const std::string& to) {
            size_t p = 0;
            while ((p = str.find(from, p)) != std::string::npos) {
                str.replace(p, from.size(), to);
                p += to.size();
            }
        };
        replace_all(s, "&amp;",  "&");
        replace_all(s, "&lt;",   "<");
        replace_all(s, "&gt;",   ">");
        replace_all(s, "&quot;", "\"");
        replace_all(s, "&#39;",  "'");
        return QuantumValue(s); });

    reg("double_url_encode", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("double_url_encode() expects string");
        const std::string& s = args[0].asString();
        // First pass: encode everything non-alnum
        std::ostringstream p1;
        for (unsigned char c : s) {
            if (std::isalnum(c)) p1 << c;
            else p1 << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        // Second pass: encode the % signs from first pass
        std::string first = p1.str();
        std::ostringstream p2;
        for (unsigned char c : first) {
            if (c == '%') p2 << "%25";
            else p2 << c;
        }
        return QuantumValue(p2.str()); });

    reg("hex_encode_all", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("hex_encode_all() expects string");
        std::ostringstream oss;
        for (unsigned char c : args[0].asString())
            oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str()); });

    // ── Hashing extras ────────────────────────────────────────────────────────

    // SHA-512 (FIPS 180-4)
    reg("sha512", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("sha512() expects string");
        const std::string& msg = args[0].asString();
        static const uint64_t K[80] = {
            0x428a2f98d728ae22ULL,0x7137449123ef65cdULL,0xb5c0fbcfec4d3b2fULL,0xe9b5dba58189dbbcULL,
            0x3956c25bf348b538ULL,0x59f111f1b605d019ULL,0x923f82a4af194f9bULL,0xab1c5ed5da6d8118ULL,
            0xd807aa98a3030242ULL,0x12835b0145706fbeULL,0x243185be4ee4b28cULL,0x550c7dc3d5ffb4e2ULL,
            0x72be5d74f27b896fULL,0x80deb1fe3b1696b1ULL,0x9bdc06a725c71235ULL,0xc19bf174cf692694ULL,
            0xe49b69c19ef14ad2ULL,0xefbe4786384f25e3ULL,0x0fc19dc68b8cd5b5ULL,0x240ca1cc77ac9c65ULL,
            0x2de92c6f592b0275ULL,0x4a7484aa6ea6e483ULL,0x5cb0a9dcbd41fbd4ULL,0x76f988da831153b5ULL,
            0x983e5152ee66dfabULL,0xa831c66d2db43210ULL,0xb00327c898fb213fULL,0xbf597fc7beef0ee4ULL,
            0xc6e00bf33da88fc2ULL,0xd5a79147930aa725ULL,0x06ca6351e003826fULL,0x142929670a0e6e70ULL,
            0x27b70a8546d22ffcULL,0x2e1b21385c26c926ULL,0x4d2c6dfc5ac42aedULL,0x53380d139d95b3dfULL,
            0x650a73548baf63deULL,0x766a0abb3c77b2a8ULL,0x81c2c92e47edaee6ULL,0x92722c851482353bULL,
            0xa2bfe8a14cf10364ULL,0xa81a664bbc423001ULL,0xc24b8b70d0f89791ULL,0xc76c51a30654be30ULL,
            0xd192e819d6ef5218ULL,0xd69906245565a910ULL,0xf40e35855771202aULL,0x106aa07032bbd1b8ULL,
            0x19a4c116b8d2d0c8ULL,0x1e376c085141ab53ULL,0x2748774cdf8eeb99ULL,0x34b0bcb5e19b48a8ULL,
            0x391c0cb3c5c95a63ULL,0x4ed8aa4ae3418acbULL,0x5b9cca4f7763e373ULL,0x682e6ff3d6b2b8a3ULL,
            0x748f82ee5defb2fcULL,0x78a5636f43172f60ULL,0x84c87814a1f0ab72ULL,0x8cc702081a6439ecULL,
            0x90befffa23631e28ULL,0xa4506cebde82bde9ULL,0xbef9a3f7b2c67915ULL,0xc67178f2e372532bULL,
            0xca273eceea26619cULL,0xd186b8c721c0c207ULL,0xeada7dd6cde0eb1eULL,0xf57d4f7fee6ed178ULL,
            0x06f067aa72176fbaULL,0x0a637dc5a2c898a6ULL,0x113f9804bef90daeULL,0x1b710b35131c471bULL,
            0x28db77f523047d84ULL,0x32caab7b40c72493ULL,0x3c9ebe0a15c9bebcULL,0x431d67c49c100d4cULL,
            0x4cc5d4becb3e42b6ULL,0x597f299cfc657e2aULL,0x5fcb6fab3ad6faecULL,0x6c44198c4a475817ULL
        };
        auto rotr64=[](uint64_t x,int n){return (x>>n)|(x<<(64-n));};
        auto ch64 =[](uint64_t e,uint64_t f,uint64_t g){return (e&f)^(~e&g);};
        auto maj64=[](uint64_t a,uint64_t b,uint64_t c){return (a&b)^(a&c)^(b&c);};
        auto sig0=[&](uint64_t x){return rotr64(x,28)^rotr64(x,34)^rotr64(x,39);};
        auto sig1=[&](uint64_t x){return rotr64(x,14)^rotr64(x,18)^rotr64(x,41);};
        auto gam0=[&](uint64_t x){return rotr64(x,1)^rotr64(x,8)^(x>>7);};
        auto gam1=[&](uint64_t x){return rotr64(x,19)^rotr64(x,61)^(x>>6);};
        std::vector<uint8_t> data(msg.begin(),msg.end());
        uint64_t bit_len=(uint64_t)data.size()*8;
        data.push_back(0x80);
        while(data.size()%128!=112) data.push_back(0);
        // append 128-bit big-endian length (high 64 bits are 0)
        for(int i=0;i<8;i++) data.push_back(0);
        for(int i=7;i>=0;i--) data.push_back((uint8_t)(bit_len>>(i*8)));
        uint64_t h[8]={
            0x6a09e667f3bcc908ULL,0xbb67ae8584caa73bULL,0x3c6ef372fe94f82bULL,0xa54ff53a5f1d36f1ULL,
            0x510e527fade682d1ULL,0x9b05688c2b3e6c1fULL,0x1f83d9abfb41bd6bULL,0x5be0cd19137e2179ULL};
        for(size_t i=0;i<data.size();i+=128){
            uint64_t w[80];
            for(int j=0;j<16;j++){
                w[j]=0;
                for(int k=0;k<8;k++) w[j]=(w[j]<<8)|data[i+j*8+k];
            }
            for(int j=16;j<80;j++) w[j]=gam1(w[j-2])+w[j-7]+gam0(w[j-15])+w[j-16];
            uint64_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
            for(int j=0;j<80;j++){
                uint64_t t1=hh+sig1(e)+ch64(e,f,g)+K[j]+w[j];
                uint64_t t2=sig0(a)+maj64(a,b,c);
                hh=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
            }
            h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
        }
        std::ostringstream oss;
        for(int i=0;i<8;i++) oss<<std::hex<<std::setw(16)<<std::setfill('0')<<h[i];
        return QuantumValue(oss.str()); });

    // CRC-32
    reg("crc32", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("crc32() expects string");
        uint32_t crc = 0xFFFFFFFF;
        for (unsigned char c : args[0].asString()) {
            crc ^= c;
            for (int i = 0; i < 8; i++)
                crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(0u - (crc & 1u)));
        }
        crc ^= 0xFFFFFFFF;
        std::ostringstream oss;
        oss << std::hex << std::setw(8) << std::setfill('0') << crc;
        return QuantumValue(oss.str()); });

    // Rolling SHA-256 over chunks (simulate file hashing in pieces)
    reg("hash_file_sim", [sha256_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isArray()) throw TypeError("hash_file_sim() expects array of strings");
        std::string combined;
        for (auto& chunk : *args[0].asArray()) {
            if (!chunk.isString()) throw TypeError("hash_file_sim: all chunks must be strings");
            combined += chunk.asString();
        }
        return QuantumValue(sha256_impl(combined)); });

    // ── Crypto extras ─────────────────────────────────────────────────────────

    // AES-128 CBC (builds on ECB block function with IV chaining)
    reg("aes128_cbc_encrypt", [aes128_ecb](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3 || !args[0].isString() || !args[1].isString() || !args[2].isString())
            throw TypeError("aes128_cbc_encrypt(key, iv, plaintext) expects three strings");
        const std::string& key = args[0].asString();
        const std::string& iv  = args[1].asString();
        if (key.size() != 16) throw RuntimeError("aes128_cbc_encrypt: key must be 16 bytes");
        if (iv.size()  != 16) throw RuntimeError("aes128_cbc_encrypt: iv must be 16 bytes");
        // PKCS#7 pad
        std::string plain = args[2].asString();
        uint8_t pad = (uint8_t)(16 - (plain.size() % 16));
        plain.append(pad, (char)pad);
        std::string result;
        std::string prev = iv;
        for (size_t i = 0; i < plain.size(); i += 16) {
            // XOR plaintext block with previous ciphertext (or IV)
            std::string block(16, '\0');
            for (int j = 0; j < 16; j++) block[j] = plain[i+j] ^ prev[j];
            // ECB encrypt single block (no padding — block is already 16 bytes)
            std::string ct_block = aes128_ecb(key, block, true);
            // aes128_ecb adds PKCS#7 on a 16-byte input → 32 bytes; take first 16
            ct_block.resize(16);
            result += ct_block;
            prev = ct_block;
        }
        return QuantumValue(result); });

    reg("aes128_cbc_decrypt", [aes128_ecb](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3 || !args[0].isString() || !args[1].isString() || !args[2].isString())
            throw TypeError("aes128_cbc_decrypt(key, iv, ciphertext) expects three strings");
        const std::string& key = args[0].asString();
        const std::string& iv  = args[1].asString();
        const std::string& ct  = args[2].asString();
        if (key.size() != 16) throw RuntimeError("aes128_cbc_decrypt: key must be 16 bytes");
        if (iv.size()  != 16) throw RuntimeError("aes128_cbc_decrypt: iv must be 16 bytes");
        if (ct.size() % 16 != 0) throw RuntimeError("aes128_cbc_decrypt: ciphertext must be multiple of 16 bytes");
        std::string result;
        std::string prev = iv;
        for (size_t i = 0; i < ct.size(); i += 16) {
            std::string block = ct.substr(i, 16);
            // Decrypt block with ECB (pass as-is, no padding added since it's exactly 16)
            // Pad block to 32 bytes to trick ECB into thinking it needs padding removal
            std::string dec = aes128_ecb(key, block, false);
            // XOR with previous ciphertext block (or IV)
            for (int j = 0; j < 16; j++) dec[j] ^= prev[j];
            result += dec;
            prev = block;
        }
        // Remove PKCS#7
        if (!result.empty()) {
            uint8_t p = (uint8_t)result.back();
            if (p > 0 && p <= 16) result.resize(result.size() - p);
        }
        return QuantumValue(result); });

    // RC4 (Arcfour) stream cipher
    reg("rc4", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("rc4(key, data) expects two strings");
        const std::string& key  = args[0].asString();
        const std::string& data = args[1].asString();
        if (key.empty()) throw RuntimeError("rc4: key must not be empty");
        uint8_t S[256];
        for (int i = 0; i < 256; i++) S[i] = (uint8_t)i;
        int j = 0;
        for (int i = 0; i < 256; i++) {
            j = (j + S[i] + (unsigned char)key[i % key.size()]) & 0xFF;
            std::swap(S[i], S[j]);
        }
        int ii = 0; j = 0;
        std::string out;
        for (unsigned char c : data) {
            ii = (ii + 1) & 0xFF;
            j  = (j + S[ii]) & 0xFF;
            std::swap(S[ii], S[j]);
            out += (char)(c ^ S[(S[ii] + S[j]) & 0xFF]);
        }
        return QuantumValue(out); });

    // ChaCha20 single keystream block (RFC 7539)
    reg("chacha20_block", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3 || !args[0].isString() || !args[1].isString())
            throw TypeError("chacha20_block(key32, nonce12, counter) expects key(32B), nonce(12B), counter");
        const std::string& key   = args[0].asString();
        const std::string& nonce = args[1].asString();
        uint32_t counter = args.size() > 2 ? (uint32_t)toNum(args[2], "chacha20_block") : 0;
        if (key.size()   != 32) throw RuntimeError("chacha20_block: key must be 32 bytes");
        if (nonce.size() != 12) throw RuntimeError("chacha20_block: nonce must be 12 bytes");
        auto le32 = [](const uint8_t* p) -> uint32_t {
            return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
        };
        auto rotl32 = [](uint32_t x, int n) { return (x << n) | (x >> (32-n)); };
        auto qr = [&](uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d){
            a+=b; d^=a; d=rotl32(d,16);
            c+=d; b^=c; b=rotl32(b,12);
            a+=b; d^=a; d=rotl32(d, 8);
            c+=d; b^=c; b=rotl32(b, 7);
        };
        uint32_t s[16];
        s[0]=0x61707865; s[1]=0x3320646e; s[2]=0x79622d32; s[3]=0x6b206574;
        const uint8_t* k = (const uint8_t*)key.data();
        for (int i=0;i<8;i++) s[4+i] = le32(k + i*4);
        s[12] = counter;
        const uint8_t* n = (const uint8_t*)nonce.data();
        s[13] = le32(n);  s[14] = le32(n+4);  s[15] = le32(n+8);
        uint32_t ws[16]; for(int i=0;i<16;i++) ws[i]=s[i];
        for (int i=0;i<10;i++){
            qr(ws[0],ws[4],ws[8], ws[12]); qr(ws[1],ws[5],ws[9], ws[13]);
            qr(ws[2],ws[6],ws[10],ws[14]); qr(ws[3],ws[7],ws[11],ws[15]);
            qr(ws[0],ws[5],ws[10],ws[15]); qr(ws[1],ws[6],ws[11],ws[12]);
            qr(ws[2],ws[7],ws[8], ws[13]); qr(ws[3],ws[4],ws[9], ws[14]);
        }
        std::string out(64, '\0');
        for (int i=0;i<16;i++) {
            uint32_t v = ws[i] + s[i];
            out[i*4+0] = (char)(v & 0xFF);
            out[i*4+1] = (char)((v>>8) & 0xFF);
            out[i*4+2] = (char)((v>>16) & 0xFF);
            out[i*4+3] = (char)((v>>24) & 0xFF);
        }
        return QuantumValue(out); });

    // ── Password / credential attacks ─────────────────────────────────────────

    reg("wordlist_hash", [sha256_impl, md5_impl, sha1_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isArray() || !args[1].isString())
            throw TypeError("wordlist_hash(words, algo) expects array and algo string");
        const std::string& algo = args[1].asString();
        auto result = std::make_shared<Array>();
        for (auto& w : *args[0].asArray()) {
            if (!w.isString()) continue;
            const std::string& word = w.asString();
            std::string h;
            if      (algo == "sha256") h = sha256_impl(word);
            else if (algo == "sha1")   h = sha1_impl(word);
            else if (algo == "md5")    h = md5_impl(word);
            else throw RuntimeError("wordlist_hash: algo must be 'sha256', 'sha1', or 'md5'");
            auto entry = std::make_shared<Dict>();
            (*entry)["word"] = QuantumValue(word);
            (*entry)["hash"] = QuantumValue(h);
            result->push_back(QuantumValue(entry));
        }
        return QuantumValue(result); });

    reg("crack_hash", [sha256_impl, md5_impl, sha1_impl](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3 || !args[0].isString() || !args[1].isArray() || !args[2].isString())
            throw TypeError("crack_hash(target, words, algo) expects string, array, string");
        const std::string& target = args[0].asString();
        const std::string& algo   = args[2].asString();
        for (auto& w : *args[1].asArray()) {
            if (!w.isString()) continue;
            const std::string& word = w.asString();
            std::string h;
            if      (algo == "sha256") h = sha256_impl(word);
            else if (algo == "sha1")   h = sha1_impl(word);
            else if (algo == "md5")    h = md5_impl(word);
            else throw RuntimeError("crack_hash: algo must be 'sha256', 'sha1', or 'md5'");
            if (h == target) return QuantumValue(word);
        }
        return QuantumValue(); }); // nil = not found

    reg("gen_wordlist_mutations", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("gen_wordlist_mutations() expects string");
        std::string w = args[0].asString();
        auto arr = std::make_shared<Array>();
        auto add = [&](const std::string& s){ arr->push_back(QuantumValue(s)); };
        // Capitalizations
        std::string cap = w; if (!cap.empty()) cap[0] = (char)std::toupper(cap[0]);
        std::string upper = w; for (char& c : upper) c = (char)std::toupper(c);
        std::string lower = w; for (char& c : lower) c = (char)std::tolower(c);
        add(w); add(cap); add(upper); add(lower);
        // Common suffixes
        for (const std::string& suf : {"1","12","123","1234","12345","!","@","#","$","!!","123!"}) {
            add(w + suf); add(cap + suf); add(lower + suf);
        }
        // Common substitutions (leet speak)
        std::string leet = lower;
        for (char& c : leet) {
            if (c=='a') c='@'; else if (c=='e') c='3';
            else if (c=='i') c='1'; else if (c=='o') c='0';
            else if (c=='s') c='$'; else if (c=='t') c='+';
        }
        add(leet); add(leet + "1"); add(leet + "!");
        // Year suffixes
        for (const std::string& yr : {"2020","2021","2022","2023","2024","2025"}) {
            add(w + yr); add(cap + yr);
        }
        // Reverse
        std::string rev = w; std::reverse(rev.begin(), rev.end());
        add(rev);
        return QuantumValue(arr); });

    // ── Network / protocol helpers ────────────────────────────────────────────

    reg("ip_to_int", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("ip_to_int() expects string");
        const std::string& ip = args[0].asString();
        uint32_t result = 0;
        int octet = 0, shift = 24;
        for (size_t i = 0; i <= ip.size(); i++) {
            if (i == ip.size() || ip[i] == '.') {
                if (octet < 0 || octet > 255) throw RuntimeError("ip_to_int: invalid IPv4 address");
                result |= ((uint32_t)octet << shift);
                shift -= 8; octet = 0;
            } else if (std::isdigit(ip[i])) {
                octet = octet * 10 + (ip[i] - '0');
            } else throw RuntimeError("ip_to_int: invalid character in IP address");
        }
        return QuantumValue((double)result); });

    reg("int_to_ip", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("int_to_ip() requires 1 argument");
        uint32_t n = (uint32_t)toNum(args[0], "int_to_ip");
        return QuantumValue(std::to_string((n>>24)&0xFF) + "." + std::to_string((n>>16)&0xFF) + "." +
                            std::to_string((n>>8) &0xFF) + "." + std::to_string(n&0xFF)); });

    reg("ip_in_cidr", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("ip_in_cidr(ip, cidr) expects two strings");
        // Parse IP
        auto parse_ip = [](const std::string& ip) -> uint32_t {
            uint32_t r=0; int oct=0, sh=24;
            for (size_t i=0;i<=ip.size();i++){
                if (i==ip.size()||ip[i]=='.'){r|=((uint32_t)oct<<sh);sh-=8;oct=0;}
                else oct=oct*10+(ip[i]-'0');
            }
            return r;
        };
        // Parse CIDR
        const std::string& cidr = args[1].asString();
        size_t slash = cidr.find('/');
        if (slash == std::string::npos) throw RuntimeError("ip_in_cidr: CIDR must contain '/'");
        uint32_t net = parse_ip(cidr.substr(0, slash));
        int prefix   = std::stoi(cidr.substr(slash+1));
        if (prefix < 0 || prefix > 32) throw RuntimeError("ip_in_cidr: prefix must be 0-32");
        uint32_t mask = prefix == 0 ? 0 : (0xFFFFFFFF << (32 - prefix));
        uint32_t ip   = parse_ip(args[0].asString());
        return QuantumValue((ip & mask) == (net & mask)); });

    reg("cidr_hosts", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("cidr_hosts() expects CIDR string");
        const std::string& cidr = args[0].asString();
        size_t slash = cidr.find('/');
        if (slash == std::string::npos) throw RuntimeError("cidr_hosts: must contain '/'");
        auto parse_ip = [](const std::string& s) -> uint32_t {
            uint32_t r=0; int oct=0, sh=24;
            for (size_t i=0;i<=s.size();i++){
                if(i==s.size()||s[i]=='.'){r|=((uint32_t)oct<<sh);sh-=8;oct=0;}
                else oct=oct*10+(s[i]-'0');
            }
            return r;
        };
        uint32_t net    = parse_ip(cidr.substr(0, slash));
        int prefix      = std::stoi(cidr.substr(slash+1));
        if (prefix < 0 || prefix > 32) throw RuntimeError("cidr_hosts: prefix 0-32");
        uint32_t mask   = prefix == 0 ? 0 : (0xFFFFFFFF << (32-prefix));
        uint32_t host_count = (~mask); // excludes network and broadcast
        if (host_count > 65536) throw RuntimeError("cidr_hosts: range too large (max /16)");
        auto arr = std::make_shared<Array>();
        uint32_t base = net & mask;
        for (uint32_t i = 1; i < host_count; i++) {
            uint32_t n = base + i;
            std::string ip = std::to_string((n>>24)&0xFF)+"."+std::to_string((n>>16)&0xFF)+
                             "."+std::to_string((n>>8)&0xFF)+"."+std::to_string(n&0xFF);
            arr->push_back(QuantumValue(ip));
        }
        return QuantumValue(arr); });

    reg("parse_http_request", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("parse_http_request() expects string");
        const std::string& raw = args[0].asString();
        auto result = std::make_shared<Dict>();
        auto headers = std::make_shared<Dict>();
        std::istringstream stream(raw);
        std::string line;
        // Request line
        if (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::istringstream rl(line);
            std::string method, path, version;
            rl >> method >> path >> version;
            (*result)["method"]  = QuantumValue(method);
            (*result)["path"]    = QuantumValue(path);
            (*result)["version"] = QuantumValue(version);
        }
        // Headers
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string hname = line.substr(0, colon);
                std::string hval  = line.substr(colon+1);
                while (!hval.empty() && hval[0]==' ') hval.erase(0,1);
                (*headers)[hname] = QuantumValue(hval);
            }
        }
        (*result)["headers"] = QuantumValue(headers);
        // Body (rest of stream)
        std::string body((std::istreambuf_iterator<char>(stream)), {});
        (*result)["body"] = QuantumValue(body);
        return QuantumValue(result); });

    reg("build_http_request", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3) throw RuntimeError("build_http_request(method,host,path,headers,body)");
        std::string method = args[0].isString() ? args[0].asString() : "GET";
        std::string host   = args[1].isString() ? args[1].asString() : "";
        std::string path_  = args[2].isString() ? args[2].asString() : "/";
        std::string body   = args.size() > 4 && args[4].isString() ? args[4].asString() : "";
        std::ostringstream req;
        req << method << " " << path_ << " HTTP/1.1\r\n";
        req << "Host: " << host << "\r\n";
        // Extra headers from dict
        if (args.size() > 3 && args[3].isDict()) {
            for (auto& [k,v] : *args[3].asDict())
                req << k << ": " << v.toString() << "\r\n";
        }
        if (!body.empty())
            req << "Content-Length: " << body.size() << "\r\n";
        req << "Connection: close\r\n\r\n";
        req << body;
        return QuantumValue(req.str()); });

    // ── Forensics / analysis ──────────────────────────────────────────────────

    reg("find_pattern", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("find_pattern(data, pattern) expects two strings");
        const std::string& data    = args[0].asString();
        const std::string& pattern = args[1].asString();
        auto offsets = std::make_shared<Array>();
        if (pattern.empty()) return QuantumValue(offsets);
        size_t pos = 0;
        while ((pos = data.find(pattern, pos)) != std::string::npos) {
            offsets->push_back(QuantumValue((double)pos));
            pos++;
        }
        return QuantumValue(offsets); });

    reg("byte_histogram", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("byte_histogram() expects string");
        auto result = std::make_shared<Dict>();
        int freq[256] = {};
        for (unsigned char c : args[0].asString()) freq[c]++;
        for (int i = 0; i < 256; i++) {
            if (freq[i] > 0) {
                std::ostringstream k;
                k << std::hex << std::setw(2) << std::setfill('0') << i;
                (*result)[k.str()] = QuantumValue((double)freq[i]);
            }
        }
        return QuantumValue(result); });

    reg("ic_score", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        // Index of Coincidence — near 0.065 for English, near 0.038 for random
        if (args.empty() || !args[0].isString()) throw TypeError("ic_score() expects string");
        const std::string& s = args[0].asString();
        int freq[26] = {};
        int n = 0;
        for (char c : s) {
            if (std::isalpha(c)) { freq[std::tolower(c)-'a']++; n++; }
        }
        if (n < 2) return QuantumValue(0.0);
        double ic = 0.0;
        for (int i = 0; i < 26; i++) ic += (double)freq[i] * (freq[i]-1);
        ic /= (double)n * (n-1);
        return QuantumValue(ic); });

    reg("hamming_distance", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("hamming_distance(a, b) expects two strings");
        const std::string& a = args[0].asString();
        const std::string& b = args[1].asString();
        size_t maxlen = std::max(a.size(), b.size());
        int dist = 0;
        for (size_t i = 0; i < maxlen; i++) {
            uint8_t ca = i < a.size() ? (uint8_t)a[i] : 0;
            uint8_t cb = i < b.size() ? (uint8_t)b[i] : 0;
            uint8_t x  = ca ^ cb;
            while (x) { dist += x & 1; x >>= 1; }
        }
        return QuantumValue((double)dist); });

    reg("edit_distance", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2 || !args[0].isString() || !args[1].isString())
            throw TypeError("edit_distance(a, b) expects two strings");
        const std::string& a = args[0].asString();
        const std::string& b = args[1].asString();
        size_t m = a.size(), n = b.size();
        std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1, 0));
        for (size_t i=0;i<=m;i++) dp[i][0]=(int)i;
        for (size_t j=0;j<=n;j++) dp[0][j]=(int)j;
        for (size_t i=1;i<=m;i++)
            for (size_t j=1;j<=n;j++)
                dp[i][j] = a[i-1]==b[j-1] ? dp[i-1][j-1] :
                            1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
        return QuantumValue((double)dp[m][n]); });

    // ── Encoding / obfuscation ────────────────────────────────────────────────

    reg("str_to_hex_escape", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("str_to_hex_escape() expects string");
        std::ostringstream oss;
        for (unsigned char c : args[0].asString())
            oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str()); });

    reg("str_to_unicode_escape", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("str_to_unicode_escape() expects string");
        std::ostringstream oss;
        for (unsigned char c : args[0].asString())
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
        return QuantumValue(oss.str()); });

    reg("obfuscate_js", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty() || !args[0].isString()) throw TypeError("obfuscate_js() expects string");
        // Produces: String.fromCharCode(72,101,108,108,111)
        std::ostringstream oss;
        oss << "String.fromCharCode(";
        bool first = true;
        for (unsigned char c : args[0].asString()) {
            if (!first) oss << ",";
            oss << (int)c;
            first = false;
        }
        oss << ")";
        return QuantumValue(oss.str()); });

    // ── Number theory (RSA building blocks) ───────────────────────────────────

    reg("is_prime", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("is_prime() requires 1 argument");
        long long n = (long long)toNum(args[0], "is_prime");
        if (n < 2) return QuantumValue(false);
        if (n == 2 || n == 3 || n == 5 || n == 7) return QuantumValue(true);
        if (n % 2 == 0 || n % 3 == 0) return QuantumValue(false);
        // Miller-Rabin deterministic witnesses for n < 3.2 * 10^18
        uint64_t un = (uint64_t)n;
        uint64_t d = un - 1; int r = 0;
        while (d % 2 == 0) { d /= 2; r++; }
        auto mulmod = [](uint64_t a, uint64_t b, uint64_t m) -> uint64_t {
            // Portable 128-bit-free multiply-mod via Russian peasant algorithm
            // Works on MSVC, GCC, Clang on all platforms
            uint64_t result = 0;
            a %= m;
            while (b > 0) {
                if (b & 1) { result += a; if (result >= m) result -= m; }
                a <<= 1; if (a >= m) a -= m;
                b >>= 1;
            }
            return result;
        };
        auto powmod = [&](uint64_t base, uint64_t exp, uint64_t mod) -> uint64_t {
            uint64_t result = 1; base %= mod;
            while (exp > 0) {
                if (exp & 1) result = mulmod(result, base, mod);
                base = mulmod(base, base, mod); exp >>= 1;
            }
            return result;
        };
        for (uint64_t a : {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL}) {
            if (a >= un) continue;
            uint64_t x = powmod(a, d, un);
            if (x == 1 || x == un-1) continue;
            bool composite = true;
            for (int i = 0; i < r-1; i++) {
                x = mulmod(x, x, un);
                if (x == un-1) { composite = false; break; }
            }
            if (composite) return QuantumValue(false);
        }
        return QuantumValue(true); });

    reg("gcd", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2) throw RuntimeError("gcd(a, b) requires 2 arguments");
        long long a = std::abs((long long)toNum(args[0], "gcd"));
        long long b = std::abs((long long)toNum(args[1], "gcd"));
        while (b) { a %= b; std::swap(a, b); }
        return QuantumValue((double)a); });

    reg("mod_pow", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 3) throw RuntimeError("mod_pow(base, exp, mod) requires 3 arguments");
        uint64_t base = (uint64_t)toNum(args[0], "mod_pow");
        uint64_t exp  = (uint64_t)toNum(args[1], "mod_pow");
        uint64_t mod  = (uint64_t)toNum(args[2], "mod_pow");
        if (mod == 0) throw RuntimeError("mod_pow: modulus must not be zero");
        // Portable multiply-mod — no __uint128_t needed
        auto mulmod64 = [](uint64_t a, uint64_t b, uint64_t m) -> uint64_t {
            uint64_t result = 0;
            a %= m;
            while (b > 0) {
                if (b & 1) { result += a; if (result >= m) result -= m; }
                a <<= 1; if (a >= m) a -= m;
                b >>= 1;
            }
            return result;
        };
        uint64_t result = 1; base %= mod;
        while (exp > 0) {
            if (exp & 1) result = mulmod64(result, base, mod);
            base = mulmod64(base, base, mod);
            exp >>= 1;
        }
        return QuantumValue((double)result); });

    reg("mod_inv", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.size() < 2) throw RuntimeError("mod_inv(a, m) requires 2 arguments");
        long long a = (long long)toNum(args[0], "mod_inv");
        long long m = (long long)toNum(args[1], "mod_inv");
        if (m <= 0) throw RuntimeError("mod_inv: modulus must be positive");
        long long m0=m, x0=0, x1=1;
        if (m == 1) return QuantumValue(0.0);
        while (a > 1) {
            long long q = a / m;
            long long t = m;
            m = a % m; a = t;
            t = x0; x0 = x1 - q*x0; x1 = t;
        }
        if (x1 < 0) x1 += m0;
        return QuantumValue((double)x1); });

    // ─── Formatted output / string building ───────────────────────────────
    // printf("fmt", args...)  — print formatted, no implicit newline
    // printf("fmt", args...) — C-style printf
    reg("printf", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("printf() requires a format string");
        // Dereference any pointer args before formatting
        std::vector<QuantumValue> resolved;
        resolved.reserve(args.size());
        for (auto& a : args) {
            if (a.isPointer()) {
                auto ptr = a.asPointer();
                resolved.push_back(ptr && !ptr->isNull() ? ptr->deref() : QuantumValue());
            } else {
                resolved.push_back(a);
            }
        }
        std::cout << applyFormat(resolved[0].toString(), resolved, 1);
        std::cout.flush();
        return QuantumValue(); });

    reg("__printf__", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("printf() requires a format string");
        std::cout << applyFormat(args[0].toString(), args, 1);
        std::cout.flush();
        return QuantumValue(); });

    // format("fmt", args...)  — return formatted string (like sprintf)
    reg("format", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("format() requires a format string");
        return QuantumValue(applyFormat(args[0].toString(), args, 1)); });

    // sprintf("fmt", args...)  — alias for format()
    reg("sprintf", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) throw RuntimeError("sprintf() requires a format string");
        return QuantumValue(applyFormat(args[0].toString(), args, 1)); });

    // system("pause") / system("cls") / system("clear") — no-op stubs for C++ compat
    reg("system", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty()) {
            std::string cmd = args[0].toString();
            // "pause" — just wait for enter
            if (cmd == "pause") {
                std::cout << "Press any key to continue . . ." << std::flush;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            // "cls" / "clear" — emit newlines (terminal clear not safe in all envs)
            // Just silently ignore for interpreter use
        }
        return QuantumValue(0.0); });

    // ─── Math object (JavaScript compatibility) ──────────────────────────
    auto mathDict = std::make_shared<Dict>();

    auto mathNative = [&](const std::string &name, QuantumNativeFunc fn) -> QuantumValue
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = name;
        nat->fn = std::move(fn);
        return QuantumValue(nat);
    };

    (*mathDict)["PI"] = QuantumValue(M_PI);
    (*mathDict)["E"] = QuantumValue(M_E);
    (*mathDict)["LN2"] = QuantumValue(std::log(2.0));
    (*mathDict)["LN10"] = QuantumValue(std::log(10.0));
    (*mathDict)["LOG2E"] = QuantumValue(std::log2(M_E));
    (*mathDict)["LOG10E"] = QuantumValue(std::log10(M_E));
    (*mathDict)["SQRT2"] = QuantumValue(std::sqrt(2.0));
    (*mathDict)["Infinity"] = QuantumValue(std::numeric_limits<double>::infinity());

    (*mathDict)["floor"] = mathNative("Math.floor", [](std::vector<QuantumValue> a) -> QuantumValue
                                      { return QuantumValue(std::floor(toNum(a[0], "Math.floor"))); });
    (*mathDict)["ceil"] = mathNative("Math.ceil", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::ceil(toNum(a[0], "Math.ceil"))); });
    (*mathDict)["round"] = mathNative("Math.round", [](std::vector<QuantumValue> a) -> QuantumValue
                                      { return QuantumValue(std::round(toNum(a[0], "Math.round"))); });
    (*mathDict)["abs"] = mathNative("Math.abs", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::abs(toNum(a[0], "Math.abs"))); });
    (*mathDict)["sqrt"] = mathNative("Math.sqrt", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::sqrt(toNum(a[0], "Math.sqrt"))); });
    (*mathDict)["cbrt"] = mathNative("Math.cbrt", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::cbrt(toNum(a[0], "Math.cbrt"))); });
    (*mathDict)["pow"] = mathNative("Math.pow", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::pow(toNum(a[0], "Math.pow"), toNum(a[1], "Math.pow"))); });
    (*mathDict)["log"] = mathNative("Math.log", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::log(toNum(a[0], "Math.log"))); });
    (*mathDict)["log2"] = mathNative("Math.log2", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::log2(toNum(a[0], "Math.log2"))); });
    (*mathDict)["log10"] = mathNative("Math.log10", [](std::vector<QuantumValue> a) -> QuantumValue
                                      { return QuantumValue(std::log10(toNum(a[0], "Math.log10"))); });
    (*mathDict)["sin"] = mathNative("Math.sin", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::sin(toNum(a[0], "Math.sin"))); });
    (*mathDict)["cos"] = mathNative("Math.cos", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::cos(toNum(a[0], "Math.cos"))); });
    (*mathDict)["tan"] = mathNative("Math.tan", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::tan(toNum(a[0], "Math.tan"))); });
    (*mathDict)["asin"] = mathNative("Math.asin", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::asin(toNum(a[0], "Math.asin"))); });
    (*mathDict)["acos"] = mathNative("Math.acos", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::acos(toNum(a[0], "Math.acos"))); });
    (*mathDict)["atan"] = mathNative("Math.atan", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { return QuantumValue(std::atan(toNum(a[0], "Math.atan"))); });
    (*mathDict)["atan2"] = mathNative("Math.atan2", [](std::vector<QuantumValue> a) -> QuantumValue
                                      { return QuantumValue(std::atan2(toNum(a[0], "Math.atan2"), toNum(a[1], "Math.atan2"))); });
    (*mathDict)["exp"] = mathNative("Math.exp", [](std::vector<QuantumValue> a) -> QuantumValue
                                    { return QuantumValue(std::exp(toNum(a[0], "Math.exp"))); });
    (*mathDict)["sign"] = mathNative("Math.sign", [](std::vector<QuantumValue> a) -> QuantumValue
                                     { double v = toNum(a[0],"Math.sign"); return QuantumValue(v > 0 ? 1.0 : v < 0 ? -1.0 : 0.0); });
    (*mathDict)["trunc"] = mathNative("Math.trunc", [](std::vector<QuantumValue> a) -> QuantumValue
                                      { return QuantumValue(std::trunc(toNum(a[0], "Math.trunc"))); });
    (*mathDict)["hypot"] = mathNative("Math.hypot", [](std::vector<QuantumValue> a) -> QuantumValue
                                      {
        double s = 0; for (auto& x : a) { double v = toNum(x,"Math.hypot"); s += v*v; } return QuantumValue(std::sqrt(s)); });
    (*mathDict)["min"] = mathNative("Math.min", [](std::vector<QuantumValue> a) -> QuantumValue
                                    {
        if (a.empty()) return QuantumValue(std::numeric_limits<double>::infinity());
        double m = toNum(a[0],"Math.min"); for (size_t i=1;i<a.size();i++) m = std::min(m, toNum(a[i],"Math.min")); return QuantumValue(m); });
    (*mathDict)["max"] = mathNative("Math.max", [](std::vector<QuantumValue> a) -> QuantumValue
                                    {
        if (a.empty()) return QuantumValue(-std::numeric_limits<double>::infinity());
        double m = toNum(a[0],"Math.max"); for (size_t i=1;i<a.size();i++) m = std::max(m, toNum(a[i],"Math.max")); return QuantumValue(m); });
    (*mathDict)["random"] = mathNative("Math.random", [](std::vector<QuantumValue>) -> QuantumValue
                                       {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return QuantumValue(dist(rng)); });
    (*mathDict)["clamp"] = mathNative("Math.clamp", [](std::vector<QuantumValue> a) -> QuantumValue
                                      {
        double v = toNum(a[0],"Math.clamp"), lo = toNum(a[1],"Math.clamp"), hi = toNum(a[2],"Math.clamp");
        return QuantumValue(std::max(lo, std::min(hi, v))); });

    globals->define("Math", QuantumValue(mathDict));
    // console.log, console.warn, console.error all print space-joined args
    auto consolePrint = [](const std::string &prefix, std::ostream &out)
    {
        return [prefix, &out](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (!prefix.empty())
                out << prefix;
            for (size_t i = 0; i < args.size(); i++)
            {
                if (i)
                    out << " ";
                out << args[i].toString();
            }
            out << "\n";
            out.flush();
            return QuantumValue();
        };
    };

    auto consoleDict = std::make_shared<Dict>();

    auto makeNative = [&](const std::string &name, QuantumNativeFunc fn) -> QuantumValue
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = name;
        nat->fn = std::move(fn);
        return QuantumValue(nat);
    };

    (*consoleDict)["log"] = makeNative("console.log",
                                       [](std::vector<QuantumValue> args) -> QuantumValue
                                       {
            for (size_t i = 0; i < args.size(); i++) { if (i) std::cout << " "; std::cout << args[i].toString(); }
            std::cout << "\n"; std::cout.flush(); return QuantumValue(); });

    (*consoleDict)["warn"] = makeNative("console.warn",
                                        [](std::vector<QuantumValue> args) -> QuantumValue
                                        {
            std::cout << "[warn] ";
            for (size_t i = 0; i < args.size(); i++) { if (i) std::cout << " "; std::cout << args[i].toString(); }
            std::cout << "\n"; std::cout.flush(); return QuantumValue(); });

    (*consoleDict)["error"] = makeNative("console.error",
                                         [](std::vector<QuantumValue> args) -> QuantumValue
                                         {
            std::cerr << "[error] ";
            for (size_t i = 0; i < args.size(); i++) { if (i) std::cerr << " "; std::cerr << args[i].toString(); }
            std::cerr << "\n"; std::cerr.flush(); return QuantumValue(); });

    globals->define("console", QuantumValue(consoleDict));

    // ── Python-style exception constructors ────────────────────────────────
    // These allow  raise ValueError("msg")  to produce a meaningful string.
    // When used with raise/throw the interpreter converts to RuntimeError.
    auto makeExcClass = [&](const std::string &excName)
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = excName;
        nat->fn = [excName](std::vector<QuantumValue> args) -> QuantumValue
        {
            std::string msg = excName;
            if (!args.empty())
                msg += ": " + args[0].toString();
            return QuantumValue(msg);
        };
        globals->define(excName, QuantumValue(nat));
    };
    makeExcClass("ValueError");
    makeExcClass("TypeError");
    makeExcClass("RuntimeError");
    makeExcClass("IndexError");
    makeExcClass("KeyError");
    makeExcClass("AttributeError");
    makeExcClass("NotImplementedError");
    makeExcClass("StopIteration");
    makeExcClass("OverflowError");
    makeExcClass("ZeroDivisionError");
    makeExcClass("IOError");
    makeExcClass("FileNotFoundError");
    makeExcClass("PermissionError");
    makeExcClass("Exception");
    makeExcClass("Error");          // JS alias
    makeExcClass("RangeError");     // JS alias
    makeExcClass("ReferenceError"); // JS alias

    // ── Python built-in type tokens as first-class values ─────────────────
    // Allows:  isinstance(x, int)  isinstance(x, str)  isinstance(x, list)
    // These reserved keywords (TYPE_INT etc.) become Identifier{"int"} in the
    // AST; the interpreter looks them up here rather than throwing NameError.
    auto makeTypeVal = [&](const std::string &name)
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = name;
        // When called as a constructor/cast: int(x), str(x), list(x), etc.
        nat->fn = [name](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (args.empty())
                return QuantumValue();
            auto &v = args[0];
            if (name == "int" || name == "long" || name == "short")
            {
                if (v.isNumber())
                    return QuantumValue(std::floor(v.asNumber()));
                if (v.isString())
                {
                    try
                    {
                        return QuantumValue((double)(long long)std::stod(v.asString()));
                    }
                    catch (...)
                    {
                    }
                }
                if (v.isBool())
                    return QuantumValue(v.asBool() ? 1.0 : 0.0);
                throw TypeError("int() cannot convert " + v.typeName());
            }
            if (name == "float" || name == "double")
            {
                if (v.isNumber())
                    return v;
                if (v.isString())
                {
                    try
                    {
                        return QuantumValue(std::stod(v.asString()));
                    }
                    catch (...)
                    {
                    }
                }
                if (v.isBool())
                    return QuantumValue(v.asBool() ? 1.0 : 0.0);
                throw TypeError("float() cannot convert " + v.typeName());
            }
            if (name == "str" || name == "string")
                return QuantumValue(v.toString());
            if (name == "bool")
                return QuantumValue(v.isTruthy());
            if (name == "list" || name == "tuple")
            {
                if (v.isArray())
                    return v;
                auto arr = std::make_shared<Array>();
                if (v.isString())
                {
                    for (char c : v.asString())
                        arr->push_back(QuantumValue(std::string(1, c)));
                }
                else
                    arr->push_back(v);
                return QuantumValue(arr);
            }
            if (name == "dict")
            {
                if (v.isDict())
                    return v;
                return QuantumValue(std::make_shared<Dict>());
            }
            return v;
        };
        globals->define(name, QuantumValue(nat));
    };
    makeTypeVal("int");
    makeTypeVal("float");
    makeTypeVal("double");
    makeTypeVal("str");
    makeTypeVal("bool");
    makeTypeVal("list");
    makeTypeVal("tuple");
    makeTypeVal("dict");
    makeTypeVal("long");
    makeTypeVal("short");
    makeTypeVal("char");

    // ── isinstance(obj, klass) ──────────────────────────────────────────────
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = "isinstance";
        nat->fn = [this](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (args.size() < 2)
                throw RuntimeError("isinstance() requires 2 arguments");

            auto &val = args[0];
            auto &typeVal = args[1];

            std::function<bool(const QuantumValue &, const QuantumValue &)> checkType = [&](const QuantumValue &v, const QuantumValue &t) -> bool
            {
                if (t.isArray())
                {
                    for (auto &item : *t.asArray())
                    {
                        if (checkType(v, item))
                            return true;
                    }
                    return false;
                }
                if (t.isNative())
                {
                    std::string tname = t.asNative()->name;
                    if (tname == "int" || tname == "long" || tname == "short" || tname == "float" || tname == "double")
                        return v.isNumber();
                    if (tname == "str" || tname == "string" || tname == "char")
                        return v.isString();
                    if (tname == "list" || tname == "Array" || tname == "tuple")
                        return v.isArray();
                    if (tname == "dict" || tname == "Object")
                        return v.isDict();
                    if (tname == "bool")
                        return v.isBool();
                    if (tname == "function" || tname == "Callable")
                        return v.isFunction() || v.isNative();
                }

                if (!v.isInstance())
                    return false;
                if (!t.isClass())
                    return false;
                auto inst = v.asInstance();
                auto targetKlass = t.asClass();
                auto k = inst->klass.get();
                while (k)
                {
                    if (k == targetKlass.get())
                        return true;
                    k = k->base.get();
                }
                return false;
            };

            return QuantumValue(checkType(val, typeVal));
        };
        globals->define("isinstance", QuantumValue(nat));
    }

    // ── classname(obj) — get class name as string ───────────────────────────
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = "classname";
        nat->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (args.empty())
                return QuantumValue(std::string("nil"));
            if (args[0].isInstance())
                return QuantumValue(args[0].asInstance()->klass->name);
            return QuantumValue(args[0].typeName());
        };
        globals->define("classname", QuantumValue(nat));
    }

    // ── __format__(obj, specifier) — internal f-string format helper ────────
    {
        auto nat = std::make_shared<QuantumNative>();
        nat->name = "__format__";
        nat->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (args.size() < 2)
                return QuantumValue(std::string(""));
            std::string valStr = args[0].toString();
            std::string spec = args[1].asString();

            // Basic handling for Python format specifiers like .2f
            if (spec.size() >= 3 && spec[0] == '.' && spec.back() == 'f' && args[0].isNumber())
            {
                int precision = std::stoi(spec.substr(1, spec.size() - 2));
                std::ostringstream out;
                out.precision(precision);
                out << std::fixed << args[0].asNumber();
                return QuantumValue(out.str());
            }

            // Basic handling for alignment like *^30 or >10
            if (spec.size() >= 2)
            {
                char fill = ' ';
                char align = spec[0];
                size_t widthIdx = 1;

                if (spec.size() >= 3 && (spec[1] == '<' || spec[1] == '>' || spec[1] == '^'))
                {
                    fill = spec[0];
                    align = spec[1];
                    widthIdx = 2;
                }

                if (align == '<' || align == '>' || align == '^')
                {
                    try
                    {
                        int width = std::stoi(spec.substr(widthIdx));
                        int len = (int)valStr.size();
                        if (width > len)
                        {
                            if (align == '<')
                            {
                                valStr = valStr + std::string(width - len, fill);
                            }
                            else if (align == '>')
                            {
                                valStr = std::string(width - len, fill) + valStr;
                            }
                            else if (align == '^')
                            {
                                int padLeft = (width - len) / 2;
                                int padRight = width - len - padLeft;
                                valStr = std::string(padLeft, fill) + valStr + std::string(padRight, fill);
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }

            return QuantumValue(valStr);
        };
        globals->define("__format__", QuantumValue(nat));
    }

    // ---- print native (called as function: print("hello")) ----
    reg("print", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        auto pyStr = [](const QuantumValue &v) -> std::string {
            if (v.isBool()) return v.asBool() ? "True" : "False";
            if (v.isNil())  return "None";
            return v.toString();
        };
        if (args.empty()) { std::cout << "\n"; std::cout.flush(); return QuantumValue(); }
        bool isPrintf = false;
        if (args.size() > 1 && args[0].isString()) {
            const std::string &fmt = args[0].asString();
            for (size_t i = 0; i + 1 < fmt.size(); ++i)
                if (fmt[i] == '%' && fmt[i+1] != '%') { isPrintf = true; break; }
        }
        if (isPrintf) {
            std::cout << applyFormat(args[0].toString(), args, 1);
        } else {
            for (size_t i = 0; i < args.size(); i++) {
                if (i) std::cout << " ";
                std::cout << pyStr(args[i]);
            }
            std::cout << "\n";
        }
        std::cout.flush();
        return QuantumValue(); });

    // ---- String(x) native -- JS String constructor and String.fromCharCode ----
    {
        auto strNat = std::make_shared<QuantumNative>();
        strNat->name = "String";
        strNat->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        {
            if (args.empty())
                return QuantumValue(std::string(""));
            if (args[0].isBool())
                return QuantumValue(std::string(args[0].asBool() ? "true" : "false"));
            if (args[0].isNil())
                return QuantumValue(std::string("null"));
            if (args[0].isNumber())
            {
                double d = args[0].asNumber();
                if (d == (long long)d)
                {
                    return QuantumValue(std::to_string((long long)d));
                }
                return QuantumValue(args[0].toString());
            }
            return QuantumValue(args[0].toString());
        };
        globals->define("String", QuantumValue(strNat));
    }

    // ---- Array global (Array.from, Array.isArray) ----
    {
        auto arrNat = std::make_shared<QuantumNative>();
        arrNat->name = "Array";
        arrNat->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        {
            auto arr = std::make_shared<Array>();
            if (!args.empty() && args[0].isNumber())
            {
                int n = (int)args[0].asNumber();
                for (int i = 0; i < n; i++)
                    arr->push_back(QuantumValue());
            }
            return QuantumValue(arr);
        };
        globals->define("Array", QuantumValue(arrNat));
    }

    // ---- Object global (Object.keys/values/entries/fromEntries/assign) ----
    {
        auto makeON = [&](const std::string &nm, QuantumNativeFunc fn) -> QuantumValue
        {
            auto nat = std::make_shared<QuantumNative>();
            nat->name = nm;
            nat->fn = std::move(fn);
            return QuantumValue(nat);
        };
        auto objDict = std::make_shared<Dict>();
        (*objDict)["keys"] = makeON("Object.keys", [](std::vector<QuantumValue> args) -> QuantumValue
                                    {
            auto arr = std::make_shared<Array>();
            if (!args.empty() && args[0].isDict())
                for (auto &p : *args[0].asDict()) arr->push_back(QuantumValue(p.first));
            return QuantumValue(arr); });
        (*objDict)["values"] = makeON("Object.values", [](std::vector<QuantumValue> args) -> QuantumValue
                                      {
            auto arr = std::make_shared<Array>();
            if (!args.empty() && args[0].isDict())
                for (auto &p : *args[0].asDict()) arr->push_back(p.second);
            return QuantumValue(arr); });
        (*objDict)["entries"] = makeON("Object.entries", [](std::vector<QuantumValue> args) -> QuantumValue
                                       {
            auto arr = std::make_shared<Array>();
            if (!args.empty() && args[0].isDict())
                for (auto &p : *args[0].asDict()) {
                    auto pair = std::make_shared<Array>();
                    pair->push_back(QuantumValue(p.first)); pair->push_back(p.second);
                    arr->push_back(QuantumValue(pair));
                }
            return QuantumValue(arr); });
        (*objDict)["fromEntries"] = makeON("Object.fromEntries", [](std::vector<QuantumValue> args) -> QuantumValue
                                           {
            auto dict = std::make_shared<Dict>();
            if (!args.empty() && args[0].isArray())
                for (auto &e : *args[0].asArray())
                    if (e.isArray() && e.asArray()->size() >= 2)
                        (*dict)[(*e.asArray())[0].toString()] = (*e.asArray())[1];
            return QuantumValue(dict); });
        (*objDict)["assign"] = makeON("Object.assign", [](std::vector<QuantumValue> args) -> QuantumValue
                                      {
            if (args.empty()) return QuantumValue(std::make_shared<Dict>());
            auto target = args[0].isDict() ? args[0].asDict() : std::make_shared<Dict>();
            for (size_t i = 1; i < args.size(); i++)
                if (args[i].isDict()) for (auto &p : *args[i].asDict()) (*target)[p.first] = p.second;
            return QuantumValue(target); });
        globals->define("Object", QuantumValue(objDict));
    }

    // ---- Map constructor ----
    reg("Map", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        auto dict = std::make_shared<Dict>();
        if (!args.empty() && args[0].isArray())
            for (auto &e : *args[0].asArray())
                if (e.isArray() && e.asArray()->size() >= 2)
                    (*dict)[(*e.asArray())[0].toString()] = (*e.asArray())[1];
        return QuantumValue(dict); });

    // ---- Set constructor ----
    reg("Set", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        auto arr = std::make_shared<Array>();
        if (!args.empty() && args[0].isArray()) {
            std::unordered_map<std::string,bool> seen;
            for (auto &v : *args[0].asArray()) {
                std::string k = v.toString();
                if (!seen[k]) { seen[k] = true; arr->push_back(v); }
            }
        }
        return QuantumValue(arr); });

    // ---- parseFloat / parseInt ----
    reg("parseFloat", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) return QuantumValue(0.0);
        if (args[0].isNumber()) return args[0];
        try { return QuantumValue(std::stod(args[0].toString())); } catch (...) { return QuantumValue(0.0); } });
    reg("parseInt", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) return QuantumValue(0.0);
        if (args[0].isNumber()) return QuantumValue(std::floor(args[0].asNumber()));
        int base = (args.size() > 1 && args[1].isNumber()) ? (int)args[1].asNumber() : 10;
        try { return QuantumValue((double)std::stoll(args[0].toString(), nullptr, base)); } catch (...) { return QuantumValue(0.0); } });
    reg("isNaN", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) return QuantumValue(true);
        if (!args[0].isNumber()) return QuantumValue(true);
        return QuantumValue(std::isnan(args[0].asNumber())); });
    reg("isFinite", [](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (args.empty()) return QuantumValue(false);
        if (!args[0].isNumber()) return QuantumValue(false);
        return QuantumValue(std::isfinite(args[0].asNumber())); });

    // ---- process stub ----
    {
        auto makePN = [&](const std::string &nm, QuantumNativeFunc fn) -> QuantumValue
        {
            auto nat = std::make_shared<QuantumNative>();
            nat->name = nm;
            nat->fn = std::move(fn);
            return QuantumValue(nat);
        };
        auto stdoutDict = std::make_shared<Dict>();
        (*stdoutDict)["write"] = makePN("process.stdout.write",
                                        [](std::vector<QuantumValue> args) -> QuantumValue
                                        {
                                            if (!args.empty())
                                            {
                                                std::cout << args[0].toString();
                                                std::cout.flush();
                                            }
                                            return QuantumValue();
                                        });
        auto procDict = std::make_shared<Dict>();
        (*procDict)["stdout"] = QuantumValue(stdoutDict);
        (*procDict)["exit"] = makePN("process.exit", [](std::vector<QuantumValue>) -> QuantumValue
                                     { return QuantumValue(); });
        auto argvArr = std::make_shared<Array>();
        (*procDict)["argv"] = QuantumValue(argvArr);
        globals->define("process", QuantumValue(procDict));
    }

    // ---- setInterval/clearInterval/setTimeout/clearTimeout stubs ----
    reg("setInterval", [this](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty()) {
            try {
                if (args[0].isNative()) callNative(args[0].asNative(), {});
                else if (args[0].isFunction()) callFunction(args[0].asFunction(), {});
            } catch (...) {}
        }
        return QuantumValue(1.0); });
    reg("clearInterval", [](std::vector<QuantumValue>) -> QuantumValue
        { return QuantumValue(); });
    reg("setTimeout", [this](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty()) {
            try {
                if (args[0].isNative()) callNative(args[0].asNative(), {});
                else if (args[0].isFunction()) callFunction(args[0].asFunction(), {});
            } catch (...) {}
        }
        return QuantumValue(1.0); });
    reg("clearTimeout", [](std::vector<QuantumValue>) -> QuantumValue
        { return QuantumValue(); });

    // ---- sizeof, cin, cout ----
    reg("sizeof", [](std::vector<QuantumValue> args) -> QuantumValue
        { return QuantumValue(4.0); });

    {
        auto makePN = [&](const std::string &nm, QuantumNativeFunc fn) -> QuantumValue
        {
            auto nat = std::make_shared<QuantumNative>();
            nat->name = nm;
            nat->fn = std::move(fn);
            return QuantumValue(nat);
        };
        auto cinDict = std::make_shared<Dict>();
        (*cinDict)["get"] = makePN("cin.get", [](std::vector<QuantumValue> args) -> QuantumValue
                                   { return QuantumValue(false); }); // fail safely
        (*cinDict)["ignore"] = makePN("cin.ignore", [](std::vector<QuantumValue> args) -> QuantumValue
                                      { return QuantumValue(false); });
        globals->define("cin", QuantumValue(cinDict));

        auto coutDict = std::make_shared<Dict>();
        globals->define("cout", QuantumValue(coutDict));
    }

    // ── nullptr and NULL constants ─────────────────────────────────────────
    {
        auto nullPtr = std::make_shared<QuantumPointer>();
        nullPtr->cell = nullptr;
        nullPtr->varName = "nullptr";
        nullPtr->offset = 0;
        globals->define("nullptr", QuantumValue(nullPtr));
        globals->define("NULL", QuantumValue(nullPtr));
    }

    // ── Smart pointer factory functions ─────────────────────────────────────
    // make_unique<T>(args) and make_shared<T>(args) — treated as value constructors
    reg("make_unique", [this](std::vector<QuantumValue> args) -> QuantumValue
        {
        // make_unique<int[]>(n) → create array of n elements initialized to 0
        // make_unique<T>(args) → behave like calling T constructor
        if (!args.empty() && args[0].isNumber())
        {
            int n = std::max(1, (int)args[0].asNumber());
            auto arr = std::make_shared<Array>(n, QuantumValue(0.0));
            auto cell = std::make_shared<QuantumValue>(QuantumValue(arr));
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = cell;
            ptr->varName = "unique_ptr";
            return QuantumValue(ptr);
        }
        if (!args.empty())
        {
            auto cell = std::make_shared<QuantumValue>(args[0]);
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = cell;
            ptr->varName = "unique_ptr";
            return QuantumValue(ptr);
        }
        return QuantumValue(std::make_shared<QuantumPointer>()); });

    reg("make_shared", [this](std::vector<QuantumValue> args) -> QuantumValue
        {
        if (!args.empty())
        {
            auto cell = std::make_shared<QuantumValue>(args[0]);
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = cell;
            ptr->varName = "shared_ptr";
            return QuantumValue(ptr);
        }
        return QuantumValue(std::make_shared<QuantumPointer>()); });
}

// ─── Execute ─────────────────────────────────────────────────────────────────

void Interpreter::execute(ASTNode &node)
{
    std::visit([&](auto &n)
               {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, BlockStmt>)      execBlock(n, env); // same scope — no new env
        else if constexpr (std::is_same_v<T, VarDecl>)       execVarDecl(n);
        else if constexpr (std::is_same_v<T, FunctionDecl>)  execFunctionDecl(n);
        else if constexpr (std::is_same_v<T, ClassDecl>)     execClassDecl(n);
        else if constexpr (std::is_same_v<T, IfStmt>)        execIf(n);
        else if constexpr (std::is_same_v<T, WhileStmt>)     execWhile(n);
        else if constexpr (std::is_same_v<T, ForStmt>)       execFor(n);
        else if constexpr (std::is_same_v<T, ReturnStmt>)    execReturn(n);
        else if constexpr (std::is_same_v<T, PrintStmt>)     execPrint(n);
        else if constexpr (std::is_same_v<T, InputStmt>)     execInput(n);
        else if constexpr (std::is_same_v<T, ImportStmt>)    execImport(n);
        else if constexpr (std::is_same_v<T, ExprStmt>)      execExprStmt(n);
        else if constexpr (std::is_same_v<T, BreakStmt>)     throw BreakSignal{};
        else if constexpr (std::is_same_v<T, ContinueStmt>)  throw ContinueSignal{};
        else if constexpr (std::is_same_v<T, RaiseStmt>) {
            std::string msg = "Exception raised";
            std::string kind = "RuntimeError";
            if (n.value) {
                // Check if it's a call like ValueError("msg") — extract type name and message
                if (n.value->template is<CallExpr>()) {
                    auto &call = n.value->template as<CallExpr>();
                    if (call.callee->template is<Identifier>()) {
                        kind = call.callee->template as<Identifier>().name;
                    }
                }
                auto val = evaluate(*n.value);
                msg = val.toString();
                if (msg.empty() || msg == "nil") msg = kind;
            }
            throw QuantumError(kind, msg, node.line);
        }
        else if constexpr (std::is_same_v<T, TryStmt>) {
            auto runFinally = [&]() {
                if (n.finallyBody) execute(*n.finallyBody);
            };
            try {
                execute(*n.body);
            }
            catch (ReturnSignal&)  { runFinally(); throw; }
            catch (BreakSignal&)   { runFinally(); throw; }
            catch (ContinueSignal&){ runFinally(); throw; }
            catch (QuantumError &e) {
                bool handled = false;
                for (auto &h : n.handlers) {
                    // Match if no type specified (bare except) or type name matches
                    bool matches = h.errorType.empty() ||
                                   h.errorType == e.kind ||
                                   h.errorType == "Exception" ||
                                   h.errorType == "Error" ||
                                   h.errorType == "BaseException";
                    if (matches) {
                        if (!h.alias.empty()) {
                            if (!env->has(h.alias)) env->define(h.alias, QuantumValue(std::string(e.what())));
                            else env->set(h.alias, QuantumValue(std::string(e.what())));
                        }
                        execute(*h.body);
                        handled = true;
                        break;
                    }
                }
                if (!handled) { runFinally(); throw; }
            }
            catch (std::exception &e) {
                bool handled = false;
                for (auto &h : n.handlers) {
                    if (h.errorType.empty() || h.errorType == "Exception" || h.errorType == "Error") {
                        if (!h.alias.empty()) {
                            if (!env->has(h.alias)) env->define(h.alias, QuantumValue(std::string(e.what())));
                            else env->set(h.alias, QuantumValue(std::string(e.what())));
                        }
                        execute(*h.body);
                        handled = true;
                        break;
                    }
                }
                if (!handled) { runFinally(); throw; }
            }
            runFinally();
        }
        else {
            // Could be an expression node used as statement
            evaluate(node);
        } }, node.node);
}

void Interpreter::execBlock(BlockStmt &s, std::shared_ptr<Environment> scope)
{
    auto prev = env;
    env = scope ? scope : std::make_shared<Environment>(prev);
    // Top-level execution (globals scope): be fault-tolerant for NameError
    // so that files with missing variable definitions don't kill the whole run
    bool isTopLevel = (scope == globals);
    try
    {
        for (auto &stmt : s.statements)
        {
            if (isTopLevel)
            {
                try
                {
                    execute(*stmt);
                }
                catch (NameError &e)
                {
                    // Print to stderr (suppressed in test mode) and continue
                    std::cerr << "[NameError] " << e.what() << "\n";
                }
            }
            else
                execute(*stmt);
        }
    }
    catch (...)
    {
        env = prev;
        throw;
    }
    env = prev;
}

void Interpreter::execVarDecl(VarDecl &s)
{
    QuantumValue val;
    if (s.initializer)
        val = evaluate(*s.initializer);

    // ── C-style type coercion ────────────────────────────────────────────────
    if (!s.typeHint.empty())
    {
        const std::string &h = s.typeHint;
        // Integer types: int, short, long, unsigned, unsigned int, long long …
        if (h == "int" || h == "short" || h == "long" ||
            h == "long long" || h == "unsigned" || h == "unsigned int" ||
            h == "unsigned long" || h == "unsigned long long" || h == "unsigned short")
        {
            if (val.isNumber())
                val = QuantumValue((double)(long long)val.asNumber());
            else if (val.isString())
            {
                try
                {
                    val = QuantumValue((double)std::stoll(val.asString()));
                }
                catch (...)
                {
                    val = QuantumValue(0.0);
                }
            }
            else if (val.isBool())
                val = QuantumValue(val.asBool() ? 1.0 : 0.0);
            else if (val.isNil())
                val = QuantumValue(0.0);
        }
        // Floating point types: float, double, long double
        else if (h == "float" || h == "double" || h == "long double")
        {
            if (val.isNumber())
                ; // already double — keep it
            else if (val.isString())
            {
                try
                {
                    val = QuantumValue(std::stod(val.asString()));
                }
                catch (...)
                {
                    val = QuantumValue(0.0);
                }
            }
            else if (val.isBool())
                val = QuantumValue(val.asBool() ? 1.0 : 0.0);
            else if (val.isNil())
                val = QuantumValue(0.0);
        }
        // char — single character stored as string
        else if (h == "char")
        {
            if (val.isString())
                val = QuantumValue(val.asString().empty() ? std::string("") : std::string(1, val.asString()[0]));
            else if (val.isNumber())
            {
                char c = (char)(int)val.asNumber();
                val = QuantumValue(std::string(1, c));
            }
            else if (val.isNil())
                val = QuantumValue(std::string("\0", 1));
        }
        // string / std::string — convert anything to string
        else if (h == "string")
        {
            val = QuantumValue(val.toString());
        }
        // bool — truthy coercion
        else if (h == "bool")
        {
            val = QuantumValue(val.isTruthy());
        }
        // void — just nil; assigning void x = ... is unusual but handled gracefully
        else if (h == "void")
        {
            if (!s.isPointer)
                val = QuantumValue(); // nil
        }
    }

    // ── Pointer variable: int* p = new int(100) / int* p = &a ────────────────
    // If the VarDecl is declared as a pointer (int* p) and the initializer
    // produced a plain value (not already a pointer), wrap it in a heap cell.
    if (s.isPointer && !val.isPointer())
    {
        if (val.isNumber() && val.asNumber() == 0.0)
        {
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = nullptr;
            ptr->varName = s.name;
            val = QuantumValue(ptr);
        }
        else if (!val.isNil())
        {
            auto cell = std::make_shared<QuantumValue>(val);
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = cell;
            ptr->varName = s.name;
            val = QuantumValue(ptr);
        }
    }

    env->define(s.name, std::move(val), s.isConst);
}

void Interpreter::execFunctionDecl(FunctionDecl &s)
{
    auto fn = std::make_shared<QuantumFunction>();
    fn->name = s.name;
    fn->params = s.params;
    fn->paramIsRef = s.paramIsRef;
    for (auto &da : s.defaultArgs)
        fn->defaultArgs.push_back(da.get());
    fn->body = s.body.get();
    fn->closure = env;
    env->define(s.name, QuantumValue(fn));
}

void Interpreter::execClassDecl(ClassDecl &s)
{
    auto klass = std::make_shared<QuantumClass>();
    klass->name = s.name;

    // Inherit from base class if specified
    if (!s.base.empty())
    {
        try
        {
            auto baseVal = env->get(s.base);
            if (baseVal.isClass())
                klass->base = baseVal.asClass();
            // If base is a native stub (e.g. ABC from our import stubs), silently skip —
            // we don't need true abstract-class enforcement at runtime.
        }
        catch (NameError &)
        {
            // Base not defined at all — treat as a rootless class rather than crashing.
            // This mirrors Python's lenient behaviour when stubs are missing.
        }
    }

    // Compile instance methods
    for (auto &m : s.methods)
    {
        if (m->is<FunctionDecl>())
        {
            auto &fd = m->as<FunctionDecl>();
            auto fn = std::make_shared<QuantumFunction>();
            fn->name = fd.name;
            fn->params = fd.params;
            fn->body = fd.body.get();
            fn->closure = env;

            // Support method overloading: if name already exists, also store under name#argcount
            if (klass->methods.count(fd.name))
            {
                // Store the existing one under name#argcount if not already done
                auto existing = klass->methods[fd.name];
                size_t existingParamStart = (!existing->params.empty() &&
                                             (existing->params[0] == "self" || existing->params[0] == "this"))
                                                ? 1
                                                : 0;
                std::string existingKey = fd.name + "#" + std::to_string(existing->params.size() - existingParamStart);
                if (!klass->methods.count(existingKey))
                    klass->methods[existingKey] = existing;
            }
            klass->methods[fd.name] = fn;

            // Always store an overload-specific version
            size_t paramStart = (!fd.params.empty() &&
                                 (fd.params[0] == "self" || fd.params[0] == "this"))
                                    ? 1
                                    : 0;
            std::string overloadKey = fd.name + "#" + std::to_string(fd.params.size() - paramStart);
            klass->methods[overloadKey] = fn;
        }
    }

    // Compile static methods
    for (auto &m : s.staticMethods)
    {
        if (m->is<FunctionDecl>())
        {
            auto &fd = m->as<FunctionDecl>();
            auto fn = std::make_shared<QuantumFunction>();
            fn->name = fd.name;
            fn->params = fd.params;
            fn->body = fd.body.get();
            fn->closure = env;
            klass->staticMethods[fd.name] = fn;
        }
    }

    // Evaluate class-level field declarations and store as static fields
    for (auto &f : s.fields)
    {
        if (f->is<VarDecl>())
        {
            auto &vd = f->as<VarDecl>();
            QuantumValue val;
            if (vd.initializer)
            {
                try
                {
                    val = evaluate(*vd.initializer);
                }
                catch (...)
                {
                    val = QuantumValue();
                }
            }
            klass->staticFields[vd.name] = val;
        }
        else if (f->is<ClassDecl>())
        {
            auto &nestedCd = f->as<ClassDecl>();
            auto tempEnv = std::make_shared<Environment>(env);
            auto oldEnv = env;
            env = tempEnv;
            execClassDecl(nestedCd); // this defines the class in tempEnv
            env = oldEnv;
            klass->staticFields[nestedCd.name] = tempEnv->get(nestedCd.name);
        }
    }

    // Store the class as a first-class value
    env->define(s.name, QuantumValue(klass));
}

void Interpreter::execIf(IfStmt &s)
{
    if (evaluate(*s.condition).isTruthy())
        execute(*s.thenBranch);
    else if (s.elseBranch)
        execute(*s.elseBranch);
}

void Interpreter::execWhile(WhileStmt &s)
{
    while (evaluate(*s.condition).isTruthy())
    {
        if (++stepCount_ > MAX_STEPS)
            return; // silent termination — infinite loop from no-input program
        try
        {
            execute(*s.body);
        }
        catch (BreakSignal &)
        {
            break;
        }
        catch (ContinueSignal &)
        {
            continue;
        }
    }
}

void Interpreter::execFor(ForStmt &s)
{
    auto iter = evaluate(*s.iterable);
    bool hasTuple = !s.var2.empty();

    auto loop = [&](QuantumValue item)
    {
        auto scope = std::make_shared<Environment>(env);
        if (hasTuple)
        {
            // Tuple unpacking: for k, v in dict.items() or array of pairs
            if (item.isArray() && item.asArray()->size() >= 2)
            {
                scope->define(s.var, (*item.asArray())[0]);
                scope->define(s.var2, (*item.asArray())[1]);
            }
            else
            {
                scope->define(s.var, item);
                scope->define(s.var2, QuantumValue());
            }
        }
        else if (s.var.size() >= 2 && s.var.front() == '[' && s.var.back() == ']')
        {
            std::string inner = s.var.substr(1, s.var.size() - 2);
            size_t comma = inner.find(',');
            if (comma != std::string::npos && item.isArray() && item.asArray()->size() >= 2)
            {
                std::string p1 = inner.substr(0, comma);
                std::string p2 = inner.substr(comma + 1);
                p1.erase(0, p1.find_first_not_of(" \t"));
                p1.erase(p1.find_last_not_of(" \t") + 1);
                p2.erase(0, p2.find_first_not_of(" \t"));
                p2.erase(p2.find_last_not_of(" \t") + 1);
                auto arr = item.asArray();
                scope->define(p1, (*arr)[0]);
                scope->define(p2, (*arr)[1]);
            }
            else
                scope->define(s.var, std::move(item));
        }
        else
            scope->define(s.var, std::move(item));
        try
        {
            execBlock(s.body->as<BlockStmt>(), scope);
        }
        catch (BreakSignal &)
        {
            throw;
        }
        catch (ContinueSignal &)
        {
        }
    };

    if (iter.isArray())
    {
        for (auto &item : *iter.asArray())
        {
            try
            {
                loop(item);
            }
            catch (BreakSignal &)
            {
                break;
            }
        }
    }
    else if (iter.isString())
    {
        for (char c : iter.asString())
        {
            try
            {
                loop(QuantumValue(std::string(1, c)));
            }
            catch (BreakSignal &)
            {
                break;
            }
        }
    }
    else if (iter.isDict())
    {
        for (auto &[k, v] : *iter.asDict())
        {
            try
            {
                loop(QuantumValue(k));
            }
            catch (BreakSignal &)
            {
                break;
            }
        }
    }
    else
    {
        throw TypeError("Cannot iterate over " + iter.typeName());
    }
}

void Interpreter::execReturn(ReturnStmt &s)
{
    QuantumValue val;
    if (s.value)
        val = evaluate(*s.value);
    throw ReturnSignal(std::move(val));
}

void Interpreter::execPrint(PrintStmt &s)
{
    if (s.args.empty())
    {
        std::cout << s.end;
        std::cout.flush();
        return;
    }

    // Evaluate all args first
    std::vector<QuantumValue> vals;
    vals.reserve(s.args.size());
    for (auto &a : s.args)
        vals.push_back(evaluate(*a));

    // Printf-mode: first arg is a string containing a % format specifier
    // e.g. printf("x = %d\n", x)  OR  print("%d items", count)
    bool isPrintf = false;
    if (vals.size() > 1 && vals[0].isString())
    {
        const std::string &fmt = vals[0].asString();
        for (size_t i = 0; i + 1 < fmt.size(); ++i)
        {
            if (fmt[i] == '%' && fmt[i + 1] != '%')
            {
                isPrintf = true;
                break;
            }
        }
    }

    // Helper: Python-style value repr for print output
    auto pyStr = [](const QuantumValue &v) -> std::string
    {
        if (v.isBool())
            return v.asBool() ? "True" : "False";
        if (v.isNil())
            return "None";
        return v.toString();
    };

    if (isPrintf)
    {
        std::cout << applyFormat(vals[0].toString(), vals, 1);
    }
    else
    {
        for (size_t i = 0; i < vals.size(); i++)
        {
            if (i)
                std::cout << s.sep;
            if (vals[i].isInstance())
            {
                auto inst = vals[i].asInstance();
                auto k = inst->klass.get();
                bool found = false;
                while (k)
                {
                    auto it = k->methods.find("__str__");
                    if (it != k->methods.end())
                    {
                        auto result = callInstanceMethod(inst, it->second, {});
                        std::cout << result.toString();
                        found = true;
                        break;
                    }
                    k = k->base.get();
                }
                if (!found)
                    std::cout << pyStr(vals[i]);
            }
            else
            {
                std::cout << pyStr(vals[i]);
            }
        }
        std::cout << s.end;
    }
    std::cout.flush();
}

void Interpreter::execInput(InputStmt &s)
{
    // Evaluate the prompt string exactly once
    std::string promptStr;
    if (s.prompt)
        promptStr = evaluate(*s.prompt).toString();

    // Extract the format specifier (if any) from the prompt string
    // e.g. "%d" -> 'd',  "Enter a number: " -> 0 (no spec)
    char spec = 0;
    if (!promptStr.empty())
    {
        for (size_t i = 0; i + 1 < promptStr.size(); ++i)
        {
            if (promptStr[i] == '%' && promptStr[i + 1] != '%')
            {
                size_t j = i + 1;
                while (j < promptStr.size() && (promptStr[j] == '-' || promptStr[j] == '+' || promptStr[j] == ' ' || promptStr[j] == '0' || promptStr[j] == '#'))
                    ++j;
                while (j < promptStr.size() && std::isdigit(promptStr[j]))
                    ++j;
                if (j < promptStr.size() && promptStr[j] == '.')
                {
                    ++j;
                    while (j < promptStr.size() && std::isdigit(promptStr[j]))
                        ++j;
                }
                if (j < promptStr.size())
                    spec = promptStr[j];
                break;
            }
        }
    }

    // Strip all %specifier sequences from the prompt and print only the
    // human-readable text. e.g. "Enter number: %d" -> prints "Enter number: "
    // and "%d" alone -> prints nothing.
    if (!promptStr.empty())
    {
        std::string display;
        for (size_t i = 0; i < promptStr.size();)
        {
            if (promptStr[i] == '%')
            {
                ++i;
                if (i < promptStr.size() && promptStr[i] == '%')
                {
                    display += '%';
                    ++i; // %% -> literal %
                    continue;
                }
                // skip flags
                while (i < promptStr.size() && (promptStr[i] == '-' || promptStr[i] == '+' || promptStr[i] == ' ' || promptStr[i] == '0' || promptStr[i] == '#'))
                    ++i;
                // skip width
                while (i < promptStr.size() && std::isdigit(promptStr[i]))
                    ++i;
                // skip precision
                if (i < promptStr.size() && promptStr[i] == '.')
                {
                    ++i;
                    while (i < promptStr.size() && std::isdigit(promptStr[i]))
                        ++i;
                }
                // skip conversion char — the specifier is fully consumed, print nothing
                if (i < promptStr.size())
                    ++i;
            }
            else
            {
                display += promptStr[i++];
            }
        }
        if (!display.empty())
        {
            std::cout << display;
            std::cout.flush();
        }
    }

    // Read next whitespace-delimited token, buffering the rest of the line.
    // This matches real C++ cin >> behaviour: "5 10\n" gives "5" then "10".
    std::string line;
    while (s_cinBuffer.empty())
    {
        if (!std::getline(std::cin, line))
            break; // EOF
        // Trim trailing \r
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        // Split by whitespace into tokens
        std::istringstream iss(line);
        std::string tok;
        while (iss >> tok)
            s_cinBuffer.push_back(tok);
        // If the line was blank, loop and read another line
    }
    // Pop the front token
    std::string token;
    if (!s_cinBuffer.empty())
    {
        token = s_cinBuffer.front();
        s_cinBuffer.erase(s_cinBuffer.begin());
    }

    if (s.target.empty() && !s.lvalueTarget)
        return;

    // EOF / no input: default numeric variables to 0, string variables to ""
    if (token.empty())
    {
        // Check the declared type of the target to pick the right default
        QuantumValue defaultVal(0.0); // numeric default
        if (!s.target.empty() && env->has(s.target))
        {
            auto existing = env->get(s.target);
            if (existing.isString())
                defaultVal = QuantumValue(std::string(""));
        }
        if (s.lvalueTarget)
            setLValue(*s.lvalueTarget, defaultVal, "=");
        else if (env->has(s.target))
            env->set(s.target, defaultVal);
        else
            env->define(s.target, defaultVal);
        return;
    }

    QuantumValue val;
    try
    {
        switch (spec)
        {
        case 'd':
        case 'i':
        case 'u':
        {
            // Force integer
            long long n = std::stoll(token);
            val = QuantumValue((double)n);
            break;
        }
        case 'f':
        case 'e':
        case 'g':
        case 'F':
        case 'E':
        case 'G':
        {
            // Force float
            val = QuantumValue(std::stod(token));
            break;
        }
        case 's':
        case 'c':
        {
            // Force string
            val = QuantumValue(token);
            break;
        }
        default:
        {
            // Auto-detect: try number first, fall back to string.
            std::string trimmed = token;
            while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == ' ' || trimmed.back() == '\t'))
                trimmed.pop_back();
            try
            {
                size_t idx = 0;
                double d = std::stod(trimmed, &idx);
                if (idx == trimmed.size())
                    val = QuantumValue(d);
                else
                    val = QuantumValue(trimmed);
            }
            catch (...)
            {
                val = QuantumValue(trimmed);
            }
            // If the target variable already holds a numeric value, force numeric coercion
            if (val.isString() && !s.target.empty() && env->has(s.target))
            {
                auto existing = env->get(s.target);
                if (existing.isNumber())
                {
                    try
                    {
                        val = QuantumValue((double)std::stoll(val.asString()));
                    }
                    catch (...)
                    {
                        val = QuantumValue(0.0);
                    } // unparseable → 0
                }
            }
            break;
        }
        }
    }
    catch (...)
    {
        // Forced numeric conversion (stoll/stod) failed — default to 0
        val = QuantumValue(0.0);
    }

    if (s.lvalueTarget)
    {
        // Complex lvalue: arr[i], obj.field, *ptr, etc.
        setLValue(*s.lvalueTarget, std::move(val), "=");
        return;
    }

    if (env->has(s.target))
        env->set(s.target, std::move(val));
    else
        env->define(s.target, std::move(val));
}

void Interpreter::execImport(ImportStmt &s)
{
    // ── Stub registry for common Python standard-library symbols ──────────────
    // Any symbol not listed here is silently injected as a no-op native stub.

    auto makeStubClass = [&](const std::string &name) -> QuantumValue
    {
        auto klass = std::make_shared<QuantumClass>();
        klass->name = name;
        return QuantumValue(klass);
    };

    auto makeStubNative = [&](const std::string &name) -> QuantumValue
    {
        auto fn = std::make_shared<QuantumNative>();
        fn->name = name;
        fn->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        {
            return args.empty() ? QuantumValue() : args[0];
        };
        return QuantumValue(fn);
    };

    auto defGlobal = [&](const std::string &sym, QuantumValue val)
    {
        if (!globals->has(sym))
            globals->define(sym, std::move(val), false);
    };

    // Per-module registration helpers
    auto registerAbc = [&]()
    {
        defGlobal("ABC", makeStubClass("ABC"));
        defGlobal("abstractmethod", makeStubNative("abstractmethod"));
        defGlobal("ABCMeta", makeStubClass("ABCMeta"));
    };
    auto registerTyping = [&]()
    {
        for (auto &sym : {"List", "Dict", "Set", "Tuple", "Optional", "Union",
                          "Any", "Callable", "Type", "Iterable", "Iterator",
                          "Generator", "Sequence", "Mapping", "FrozenSet",
                          "ClassVar", "Final", "Literal", "TypeVar", "Generic",
                          "Protocol", "NamedTuple", "TypedDict", "overload",
                          "cast", "no_type_check", "get_type_hints"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerCollections = [&]()
    {
        for (auto &sym : {"defaultdict", "OrderedDict", "Counter", "deque",
                          "namedtuple", "ChainMap"})
            defGlobal(sym, makeStubClass(sym));
    };
    auto registerDataclasses = [&]()
    {
        for (auto &sym : {"dataclass", "field", "fields", "asdict", "astuple",
                          "make_dataclass", "replace", "is_dataclass"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerEnum = [&]()
    {
        for (auto &sym : {"Enum", "IntEnum", "Flag", "IntFlag", "auto", "unique"})
            defGlobal(sym, makeStubClass(sym));
    };
    auto registerFunctools = [&]()
    {
        for (auto &sym : {"reduce", "partial", "wraps", "lru_cache",
                          "cached_property", "total_ordering", "singledispatch"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerItertools = [&]()
    {
        for (auto &sym : {"chain", "cycle", "repeat", "count", "accumulate",
                          "combinations", "permutations", "product", "groupby",
                          "islice", "starmap", "takewhile", "dropwhile", "zip_longest"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerOs = [&]()
    {
        for (auto &sym : {"getcwd", "listdir", "path", "environ", "getenv",
                          "makedirs", "remove", "rename", "walk", "sep"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerSys = [&]()
    {
        for (auto &sym : {"argv", "exit", "path", "version", "platform",
                          "stdin", "stdout", "stderr", "modules", "maxsize"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerRe = [&]()
    {
        for (auto &sym : {"compile", "match", "search", "findall", "finditer",
                          "sub", "subn", "split", "fullmatch", "escape",
                          "IGNORECASE", "MULTILINE", "DOTALL", "VERBOSE"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerJson = [&]()
    {
        for (auto &sym : {"dumps", "loads", "dump", "load",
                          "JSONDecodeError", "JSONEncoder", "JSONDecoder"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerMath = [&]()
    {
        auto mod = std::make_shared<QuantumClass>();
        mod->name = "math";
        for (auto &sym : {"sqrt", "floor", "ceil", "log", "log2", "log10",
                          "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
                          "pow", "exp", "fabs", "factorial", "gcd", "lcm",
                          "pi", "e", "inf", "nan", "isnan", "isinf", "isfinite",
                          "degrees", "radians", "hypot", "trunc", "comb", "perm"})
        {
            defGlobal(sym, makeStubNative(sym));
            mod->staticFields.insert({sym, makeStubNative(sym)});
        }
        defGlobal("math", QuantumValue(mod));
    };
    auto registerRandom = [&]()
    {
        for (auto &sym : {"random", "randint", "choice", "choices", "shuffle",
                          "sample", "uniform", "gauss", "seed", "randrange"})
            defGlobal(sym, makeStubNative(sym));
    };
    auto registerDatetime = [&]()
    {
        auto dtClass = std::make_shared<QuantumClass>();
        dtClass->name = "datetime";
        dtClass->staticFields["datetime"] = QuantumValue(dtClass);
        auto nowFn = std::make_shared<QuantumNative>();
        nowFn->name = "now";
        nowFn->fn = [](std::vector<QuantumValue> args) -> QuantumValue
        { return QuantumValue(); };
        dtClass->staticFields["now"] = QuantumValue(nowFn);
        defGlobal("datetime", QuantumValue(dtClass));
        defGlobal("date", makeStubClass("date"));
        defGlobal("time", makeStubClass("time"));
        defGlobal("timedelta", makeStubClass("timedelta"));
        defGlobal("timezone", makeStubClass("timezone"));
        defGlobal("MINYEAR", makeStubClass("MINYEAR"));
        defGlobal("MAXYEAR", makeStubClass("MAXYEAR"));
    };
    auto registerPathlib = [&]()
    {
        defGlobal("Path", makeStubClass("Path"));
        defGlobal("PurePath", makeStubClass("PurePath"));
    };
    auto registerIo = [&]()
    {
        for (auto &sym : {"StringIO", "BytesIO", "TextIOWrapper", "BufferedReader"})
            defGlobal(sym, makeStubClass(sym));
    };
    auto registerCopy = [&]()
    {
        defGlobal("copy", makeStubNative("copy"));
        defGlobal("deepcopy", makeStubNative("deepcopy"));
    };

    auto registerModule = [&](const std::string &name)
    {
        if (name == "abc")
            registerAbc();
        else if (name == "typing")
            registerTyping();
        else if (name == "collections")
            registerCollections();
        else if (name == "dataclasses")
            registerDataclasses();
        else if (name == "enum")
            registerEnum();
        else if (name == "functools")
            registerFunctools();
        else if (name == "itertools")
            registerItertools();
        else if (name == "os" || name == "os.path")
            registerOs();
        else if (name == "sys")
            registerSys();
        else if (name == "re")
            registerRe();
        else if (name == "json")
            registerJson();
        else if (name == "math")
            registerMath();
        else if (name == "random")
            registerRandom();
        else if (name == "datetime")
            registerDatetime();
        else if (name == "pathlib")
            registerPathlib();
        else if (name == "io")
            registerIo();
        else if (name == "copy")
            registerCopy();
        // Unknown module: silently ignored
    };

    if (!s.module.empty())
    {
        // "from X import a, b, c"
        registerModule(s.module);
        // Any symbol still undefined after module registration gets a generic stub
        for (auto &item : s.imports)
        {
            std::string sym = item.alias.empty() ? item.name : item.alias;
            if (!globals->has(sym) && !env->has(sym))
                globals->define(sym, makeStubNative(item.name), false);
        }
    }
    else
    {
        // "import X"  or  "import X as Y"
        for (auto &item : s.imports)
        {
            registerModule(item.name);
            std::string alias = item.alias.empty() ? item.name : item.alias;
            if (!globals->has(alias))
                globals->define(alias, makeStubNative(alias), false);
        }
    }
}

void Interpreter::execExprStmt(ExprStmt &s)
{
    evaluate(*s.expr);
}

// ─── Evaluate ────────────────────────────────────────────────────────────────

QuantumValue Interpreter::evaluate(ASTNode &node)
{
    return std::visit([&](auto &n) -> QuantumValue
                      {
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
        else if constexpr (std::is_same_v<T, SliceExpr>)
        {
            auto obj = evaluate(*n.object);
            // Resolve start / stop / step with Python semantics
            auto applySlice = [&](int len) -> std::tuple<int,int,int>
            {
                int step  = n.step  ? (int)toNum(evaluate(*n.step),  "slice step")  : 1;
                int start, stop;
                if (!n.start) start = (step > 0) ? 0 : len - 1;
                else {
                    start = (int)toNum(evaluate(*n.start), "slice start");
                    if (start < 0) start += len;
                    if (start < 0) start = 0;
                    if (start > len) start = len;
                }
                if (!n.stop) stop = (step > 0) ? len : -1;
                else {
                    stop = (int)toNum(evaluate(*n.stop), "slice stop");
                    if (stop < 0) stop += len;
                    if (step > 0) { if (stop < 0) stop = 0; if (stop > len) stop = len; }
                    else          { if (stop < -1) stop = -1; }
                }
                return {start, stop, step};
            };

            if (obj.isString())
            {
                const auto &s = obj.asString();
                int len = (int)s.size();
                auto [start, stop, step] = applySlice(len);
                std::string result;
                if (step > 0) { for (int i = start; i < stop;  i += step) if (i >= 0 && i < len) result += s[i]; }
                else          { for (int i = start; i > stop;  i += step) if (i >= 0 && i < len) result += s[i]; }
                return QuantumValue(result);
            }
            if (obj.isArray())
            {
                auto &arr = *obj.asArray();
                int len = (int)arr.size();
                auto [start, stop, step] = applySlice(len);
                auto result = std::make_shared<Array>();
                if (step > 0) { for (int i = start; i < stop;  i += step) if (i >= 0 && i < len) result->push_back(arr[i]); }
                else          { for (int i = start; i > stop;  i += step) if (i >= 0 && i < len) result->push_back(arr[i]); }
                return QuantumValue(result);
            }
            throw TypeError("Cannot slice " + obj.typeName());
        }
        else if constexpr (std::is_same_v<T, MemberExpr>)    return evalMember(n);
        else if constexpr (std::is_same_v<T, ArrayLiteral>)  return evalArray(n);
        else if constexpr (std::is_same_v<T, DictLiteral>)   return evalDict(n);
        else if constexpr (std::is_same_v<T, LambdaExpr>)    return evalLambda(n);
        else if constexpr (std::is_same_v<T, ListComp>)      return evalListComp(n);
        else if constexpr (std::is_same_v<T, TupleLiteral>)
        {
            // Tuples are arrays at runtime
            auto arr = std::make_shared<Array>();
            for (auto &el : n.elements)
                arr->push_back(evaluate(*el));
            return QuantumValue(arr);
        }
        else if constexpr (std::is_same_v<T, TernaryExpr>)   {
            return evaluate(*n.condition).isTruthy()
                ? evaluate(*n.thenExpr)
                : evaluate(*n.elseExpr);
        }
        else if constexpr (std::is_same_v<T, SuperExpr>) {
            // SuperExpr is handled directly in evalCall for super() and super.method()
            // If we get here, it's a bare super reference
            throw RuntimeError("Cannot use 'super' outside of a method call");
        }
        else if constexpr (std::is_same_v<T, AddressOfExpr>) return evalAddressOf(n);
        else if constexpr (std::is_same_v<T, DerefExpr>)     return evalDeref(n);
        else if constexpr (std::is_same_v<T, ArrowExpr>)     return evalArrow(n);
        else if constexpr (std::is_same_v<T, NewExpr>)       return evalNewExpr(n);
        else {
            execute(node);
            return QuantumValue();
        } }, node.node);
}

QuantumValue Interpreter::evalIdentifier(Identifier &e)
{
    // Try regular env lookup first
    try
    {
        return env->get(e.name);
    }
    catch (NameError &)
    {
        // Fall back: if "self" is in scope and has this field or method, return it
        try
        {
            auto selfVal = env->get("self");
            if (selfVal.isInstance())
            {
                auto inst = selfVal.asInstance();
                // Check instance fields
                auto fit = inst->fields.find(e.name);
                if (fit != inst->fields.end())
                    return fit->second;
                // Check class methods (for C++ style bare method calls like defeatVillain())
                auto k = inst->klass.get();
                while (k)
                {
                    auto mit = k->methods.find(e.name);
                    if (mit != k->methods.end())
                    {
                        // Return as a bound-method-like function
                        // We wrap it so callFunction works, binding self
                        return QuantumValue(mit->second);
                    }
                    k = k->base.get();
                }
            }
        }
        catch (...)
        {
        }
        throw; // re-throw original NameError
    }
}

QuantumValue Interpreter::evalBinary(BinaryExpr &e)
{
    // Short-circuit for and/or/??
    if (e.op == "and")
    {
        auto lv = evaluate(*e.left);
        if (!lv.isTruthy())
            return lv;
        return evaluate(*e.right);
    }
    if (e.op == "or")
    {
        auto lv = evaluate(*e.left);
        if (lv.isTruthy())
            return lv;
        return evaluate(*e.right);
    }
    if (e.op == "??")
    {
        auto lv = evaluate(*e.left);
        if (!lv.isNil())
            return lv;
        return evaluate(*e.right);
    }

    auto lv = evaluate(*e.left);
    auto rv = evaluate(*e.right);
    const std::string &op = e.op;

    // ── Pointer arithmetic (must be checked before generic +/-) ─────────────
    if (lv.isPointer() && rv.isNumber())
    {
        auto ptr = lv.asPointer();
        auto newPtr = std::make_shared<QuantumPointer>(*ptr);
        if (op == "+")
            newPtr->offset += (int)rv.asNumber();
        else if (op == "-")
            newPtr->offset -= (int)rv.asNumber();
        else
            throw RuntimeError("Unsupported pointer operator: " + op);
        return QuantumValue(newPtr);
    }
    if (rv.isPointer() && lv.isNumber() && op == "+")
    {
        auto ptr = rv.asPointer();
        auto newPtr = std::make_shared<QuantumPointer>(*ptr);
        newPtr->offset += (int)lv.asNumber();
        return QuantumValue(newPtr);
    }
    // Pointer difference
    if (lv.isPointer() && rv.isPointer() && op == "-")
        return QuantumValue((double)(lv.asPointer()->offset - rv.asPointer()->offset));
    // Pointer comparison (ptr == nullptr, ptr == ptr, ptr != nullptr)
    if (lv.isPointer() || rv.isPointer())
    {
        if (op == "==" || op == "!=")
        {
            bool eq = false;
            if (lv.isPointer() && rv.isNil())
                eq = lv.asPointer()->isNull();
            else if (rv.isPointer() && lv.isNil())
                eq = rv.asPointer()->isNull();
            else if (lv.isPointer() && rv.isPointer())
                eq = (lv.asPointer()->cell == rv.asPointer()->cell &&
                      lv.asPointer()->offset == rv.asPointer()->offset);
            return QuantumValue(op == "==" ? eq : !eq);
        }
    }

    if (op == "+")
    {
        // C-style char arithmetic: 'a' + 32 → ASCII arithmetic → returns number
        if (lv.isString() && lv.asString().size() == 1 && rv.isNumber())
            return QuantumValue((double)(unsigned char)lv.asString()[0] + rv.asNumber());
        if (rv.isString() && rv.asString().size() == 1 && lv.isNumber())
            return QuantumValue(lv.asNumber() + (double)(unsigned char)rv.asString()[0]);
        if (lv.isString() || rv.isString())
            return QuantumValue(lv.toString() + rv.toString());
        if (lv.isNumber() && rv.isNumber())
            return QuantumValue(lv.asNumber() + rv.asNumber());
        if (lv.isArray() && rv.isArray())
        {
            auto arr = std::make_shared<Array>(*lv.asArray());
            for (auto &x : *rv.asArray())
                arr->push_back(x);
            return QuantumValue(arr);
        }
        throw TypeError("Cannot add " + lv.typeName() + " and " + rv.typeName());
    }
    if (op == "-")
    {
        // C-style char arithmetic: 'A' - 32 etc.
        if (lv.isString() && lv.asString().size() == 1 && rv.isNumber())
            return QuantumValue((double)(unsigned char)lv.asString()[0] - rv.asNumber());
        if (rv.isString() && rv.asString().size() == 1 && lv.isNumber())
            return QuantumValue(lv.asNumber() - (double)(unsigned char)rv.asString()[0]);
        return QuantumValue(toNum(lv, "-") - toNum(rv, "-"));
    }
    if (op == "*")
    {
        if (lv.isNumber() && rv.isNumber())
            return QuantumValue(lv.asNumber() * rv.asNumber());
        if (lv.isString() && rv.isNumber())
        {
            std::string s;
            int n = (int)rv.asNumber();
            for (int i = 0; i < n; i++)
                s += lv.asString();
            return QuantumValue(s);
        }
        // Python-style list repetition: [x]*n or n*[x]
        if (lv.isArray() && rv.isNumber())
        {
            auto res = std::make_shared<Array>();
            int n = (int)rv.asNumber();
            for (int i = 0; i < n; i++)
                for (auto &item : *lv.asArray())
                    res->push_back(item);
            return QuantumValue(res);
        }
        if (rv.isArray() && lv.isNumber())
        {
            auto res = std::make_shared<Array>();
            int n = (int)lv.asNumber();
            for (int i = 0; i < n; i++)
                for (auto &item : *rv.asArray())
                    res->push_back(item);
            return QuantumValue(res);
        }
        throw TypeError("Cannot multiply " + lv.typeName() + " and " + rv.typeName());
    }
    if (op == "/")
    {
        double d = toNum(rv, "/");
        if (d == 0)
            return QuantumValue(0.0); // C float: 0/0 = NaN → treat as 0; avoids crash on empty-stdin n=0
        return QuantumValue(toNum(lv, "/") / d);
    }
    if (op == "//")
    {
        double d = toNum(rv, "//");
        if (d == 0)
            return QuantumValue(0.0);
        return QuantumValue(std::floor(toNum(lv, "//") / d));
    }
    if (op == "%")
    {
        long long b = toInt(rv, "%");
        if (b == 0)
            throw RuntimeError("Modulo by zero");
        return QuantumValue((double)(toInt(lv, "%") % b));
    }
    if (op == "**")
        return QuantumValue(std::pow(toNum(lv, "**"), toNum(rv, "**")));
    if (op == "==")
    {
        if (lv.isNil() && rv.isNil())
            return QuantumValue(true);
        if (lv.typeName() != rv.typeName())
            return QuantumValue(false);
        if (lv.isNumber())
            return QuantumValue(lv.asNumber() == rv.asNumber());
        if (lv.isBool())
            return QuantumValue(lv.asBool() == rv.asBool());
        if (lv.isString())
            return QuantumValue(lv.asString() == rv.asString());
        return QuantumValue(false);
    }
    if (op == "!=")
    {
        // inline evaluate
        bool eq_res = false;
        if (lv.isNil() && rv.isNil())
            eq_res = true;
        else if (lv.typeName() == rv.typeName())
        {
            if (lv.isNumber())
                eq_res = lv.asNumber() == rv.asNumber();
            else if (lv.isBool())
                eq_res = lv.asBool() == rv.asBool();
            else if (lv.isString())
                eq_res = lv.asString() == rv.asString();
        }
        return QuantumValue(!eq_res);
    }
    if (op == "is" || op == "is not")
    {
        bool eq_res = false;
        if (lv.isNil() && rv.isNil())
            eq_res = true;
        else if (lv.typeName() == rv.typeName())
        {
            if (lv.isNumber())
                eq_res = lv.asNumber() == rv.asNumber();
            else if (lv.isBool())
                eq_res = lv.asBool() == rv.asBool();
            else if (lv.isString())
                eq_res = lv.asString() == rv.asString();
            else if (lv.isPointer())
                eq_res = (lv.asPointer()->cell == rv.asPointer()->cell && lv.asPointer()->offset == rv.asPointer()->offset);
            else if (lv.isArray())
                eq_res = lv.asArray() == rv.asArray();
            else if (lv.isDict())
                eq_res = lv.asDict() == rv.asDict();
            else if (lv.isInstance())
                eq_res = lv.asInstance() == rv.asInstance();
            else if (lv.isClass())
                eq_res = lv.asClass() == rv.asClass();
            else if (lv.isFunction())
            {
                if (lv.isNative() && rv.isNative())
                    eq_res = lv.asNative() == rv.asNative();
                else if (!lv.isNative() && !rv.isNative())
                    eq_res = lv.asFunction() == rv.asFunction();
            }
        }
        return QuantumValue(op == "is" ? eq_res : !eq_res);
    }
    // Numeric comparisons — coerce bool to number so chained comparisons
    // like (0 <= start) < n work correctly (True/False become 1/0).
    auto toNumOrBool = [](const QuantumValue &v, const std::string &ctx) -> double
    {
        if (v.isNumber())
            return v.asNumber();
        if (v.isBool())
            return v.asBool() ? 1.0 : 0.0;
        // C-style: single-char string → ASCII value (for char comparisons like c >= 'a')
        if (v.isString() && v.asString().size() == 1)
            return (double)(unsigned char)v.asString()[0];
        throw TypeError("Expected number in " + ctx + ", got " + v.typeName());
    };
    if (op == "<")
        return QuantumValue(toNumOrBool(lv, "<") < toNumOrBool(rv, "<"));
    if (op == ">")
        return QuantumValue(toNumOrBool(lv, ">") > toNumOrBool(rv, ">"));
    if (op == "<=")
        return QuantumValue(toNumOrBool(lv, "<=") <= toNumOrBool(rv, "<="));
    if (op == ">=")
        return QuantumValue(toNumOrBool(lv, ">=") >= toNumOrBool(rv, ">="));

    // Membership: x in collection,  x not in collection
    if (op == "in" || op == "not in")
    {
        bool found = false;
        if (rv.isArray())
        {
            for (auto &elem : *rv.asArray())
            {
                if (elem.isNumber() && lv.isNumber() && elem.asNumber() == lv.asNumber())
                {
                    found = true;
                    break;
                }
                if (elem.isString() && lv.isString() && elem.asString() == lv.asString())
                {
                    found = true;
                    break;
                }
                if (elem.isBool() && lv.isBool() && elem.asBool() == lv.asBool())
                {
                    found = true;
                    break;
                }
                if (elem.isNil() && lv.isNil())
                {
                    found = true;
                    break;
                }
            }
        }
        else if (rv.isString())
            found = rv.asString().find(lv.toString()) != std::string::npos;
        else if (rv.isDict())
            found = rv.asDict()->count(lv.toString()) > 0;
        return QuantumValue(op == "in" ? found : !found);
    }

    // Bitwise
    if (op == "&")
        return QuantumValue((double)(toInt(lv, "&") & toInt(rv, "&")));
    if (op == "|")
        return QuantumValue((double)(toInt(lv, "|") | toInt(rv, "|")));
    if (op == "^")
        return QuantumValue((double)(toInt(lv, "^") ^ toInt(rv, "^")));
    if (op == "<<")
        return QuantumValue((double)(toInt(lv, "<<") << toInt(rv, "<<")));
    if (op == ">>")
        return QuantumValue((double)(toInt(lv, ">>") >> toInt(rv, ">>")));

    throw RuntimeError("Unknown operator: " + op);
}

QuantumValue Interpreter::evalUnary(UnaryExpr &e)
{
    auto v = evaluate(*e.operand);
    if (e.op == "-")
        return QuantumValue(-toNum(v, "unary -"));
    if (e.op == "not" || e.op == "!")
        return QuantumValue(!v.isTruthy());
    if (e.op == "~")
        return QuantumValue((double)~toInt(v, "~"));
    // Spread: ...arr -- return value; evalArray does actual flattening
    if (e.op == "...")
        return v;
    // Pointer-aware increment/decrement (operand already evaluated above as 'v')
    // The orignal parsePostfix already converts p++ into AssignExpr(+=, p, 1)
    // so we only need to handle pointer arithmetic for any raw unary ++ that might arrive
    throw RuntimeError("Unknown unary op: " + e.op);
}

void Interpreter::setLValue(ASTNode &target, QuantumValue val, const std::string &op)
{
    auto applyOp = [&](QuantumValue old) -> QuantumValue
    {
        if (op == "=")
            return val;
        if (op == "+=")
        {
            if (old.isString() && val.isNumber())
            {
                // C-style: char* s; s++ advances the pointer (substring)
                const std::string &str = old.asString();
                int n = (int)val.asNumber();
                if (n >= 0 && n <= (int)str.size())
                    return QuantumValue(str.substr(n));
                return QuantumValue(std::string("")); // past end = null terminator
            }
            if (old.isString())
                return QuantumValue(old.asString() + val.toString());
            return QuantumValue(toNum(old, op) + toNum(val, op));
        }
        if (op == "-=")
        {
            if (old.isString() && val.isNumber())
            {
                // C-style: char* s; s-- goes back
                const std::string &str = old.asString();
                int n = (int)val.asNumber();
                // Can't go before string start; just return original
                return old;
            }
            return QuantumValue(toNum(old, op) - toNum(val, op));
        }
        if (op == "*=")
            return QuantumValue(toNum(old, op) * toNum(val, op));
        if (op == "/=")
        {
            double d = toNum(val, op);
            if (!d)
                throw RuntimeError("Div by 0");
            return QuantumValue(toNum(old, op) / d);
        }
        if (op == ">>")
            return QuantumValue((double)((long long)toNum(old, op) >> (long long)toNum(val, op)));
        if (op == "<<")
            return QuantumValue((double)((long long)toNum(old, op) << (long long)toNum(val, op)));
        if (op == "&=")
            return QuantumValue((double)((long long)toNum(old, op) & (long long)toNum(val, op)));
        if (op == "|=")
            return QuantumValue((double)((long long)toNum(old, op) | (long long)toNum(val, op)));
        if (op == "^=")
            return QuantumValue((double)((long long)toNum(old, op) ^ (long long)toNum(val, op)));
        if (op == "%=")
        {
            long long d = (long long)toNum(val, op);
            if (!d)
                throw RuntimeError("Mod by 0");
            return QuantumValue((double)((long long)toNum(old, op) % d));
        }
        return val;
    };

    if (target.is<Identifier>())
    {
        auto &name = target.as<Identifier>().name;
        if (op == "=")
        {
            // If assigning a plain value to an existing pointer variable (e.g. c = new int(300)),
            // wrap the value in a new heap cell so the pointer stays valid.
            if (env->has(name) && !val.isPointer() && !val.isNil())
            {
                try
                {
                    auto existing = env->get(name);
                    if (existing.isPointer())
                    {
                        if (val.isNumber() && val.asNumber() == 0.0)
                        {
                            auto ptr = std::make_shared<QuantumPointer>();
                            ptr->cell = nullptr;
                            ptr->varName = name;
                            val = QuantumValue(ptr);
                        }
                        else
                        {
                            auto cell = std::make_shared<QuantumValue>(val);
                            auto ptr = std::make_shared<QuantumPointer>();
                            ptr->cell = cell;
                            ptr->varName = name;
                            val = QuantumValue(ptr);
                        }
                    }
                }
                catch (...)
                {
                }
            }
            // Python-style: if variable doesn't exist anywhere, define it in current scope
            // But if we're inside a method (self exists) and variable not yet in any scope,
            // store as instance field (C++ style: heroName = value is equivalent to self.heroName = value)
            if (!env->has(name))
            {
                // Check if self exists — if so, store as instance field (C++ method style)
                try
                {
                    auto selfVal = env->get("self");
                    if (selfVal.isInstance())
                    {
                        selfVal.asInstance()->fields[name] = std::move(val);
                        return;
                    }
                }
                catch (...)
                {
                }
                env->define(name, std::move(val));
            }
            else
                env->set(name, std::move(val));
        }
        else
        {
            // Compound assignment: try env first, then self fields
            try
            {
                env->set(name, applyOp(env->get(name)));
            }
            catch (NameError &)
            {
                try
                {
                    auto selfVal = env->get("self");
                    if (selfVal.isInstance())
                    {
                        auto inst = selfVal.asInstance();
                        auto it = inst->fields.find(name);
                        QuantumValue old = (it != inst->fields.end()) ? it->second : QuantumValue();
                        inst->fields[name] = applyOp(old);
                        return;
                    }
                }
                catch (...)
                {
                }
                throw;
            }
        }
    }
    else if (target.is<IndexExpr>())
    {
        auto &ie = target.as<IndexExpr>();

        // Helper: get or create a shared array at a variable, ensuring it's an Array
        // This supports auto-creation for uninitialized C-style arrays: int C[N][N]; C[i][j] = v
        auto ensureArray = [&](ASTNode &objExpr, int minSize) -> std::shared_ptr<Array>
        {
            auto v = evaluate(objExpr);
            if (v.isArray())
                return v.asArray();
            // Create new array and write back
            auto arr = std::make_shared<Array>();
            arr->resize(minSize + 1, QuantumValue());
            QuantumValue arrVal(arr);
            if (objExpr.is<Identifier>())
            {
                const std::string &n = objExpr.as<Identifier>().name;
                if (env->has(n))
                    env->set(n, arrVal);
                else
                    env->define(n, arrVal);
            }
            else if (objExpr.is<IndexExpr>())
            {
                // Recursive: write the new array back through the outer index
                setLValue(objExpr, arrVal, "=");
            }
            return arr;
        };

        auto idx = evaluate(*ie.index);
        int idxI = idx.isNumber() ? (int)idx.asNumber() : 0;
        auto obj = evaluate(*ie.object);

        // Unwrap pointer-to-array: float* arr = new float[n]; arr[i] = v
        if (obj.isPointer())
        {
            auto ptr = obj.asPointer();
            if (!ptr->isNull() && ptr->cell->isArray())
                obj = *ptr->cell;
        }
        // Auto-create array when obj is nil (uninitialized C array)
        if (obj.isNil())
        {
            auto arr = ensureArray(*ie.object, idxI);
            if (idxI >= (int)arr->size())
                arr->resize(idxI + 1, QuantumValue());
            (*arr)[idxI] = applyOp((*arr)[idxI]);
            return;
        }
        if (obj.isArray())
        {
            int i = (int)toNum(idx, "index");
            auto arr = obj.asArray();
            if (i < 0)
                i += (int)arr->size();
            // Auto-extend on out-of-bounds WRITE (C dynamic array pattern: arr[count++]=val)
            if (i < 0)
                throw IndexError("Array index out of range");
            if (i >= (int)arr->size())
                arr->resize(i + 1, QuantumValue(0.0));
            (*arr)[i] = applyOp((*arr)[i]);
        }
        else if (obj.isString())
        {
            // C-style mutable char array: dest[i] = ch
            // Must write back through the variable binding
            int i = (int)toNum(idx, "index");
            if (ie.object->is<Identifier>())
            {
                const std::string &varName = ie.object->as<Identifier>().name;
                QuantumValue cur = obj;
                std::string s = cur.asString();
                // Extend string if needed (C allows writing beyond current length up to buffer)
                if (i >= (int)s.size())
                    s.resize(i + 1, '\0');
                QuantumValue newChar = applyOp(i < (int)s.size() ? QuantumValue(std::string(1, s[i])) : QuantumValue(std::string(1, '\0')));
                char c = newChar.isString() && !newChar.asString().empty() ? newChar.asString()[0]
                         : newChar.isNumber()                              ? (char)(int)newChar.asNumber()
                                                                           : '\0';
                s[i] = c;
                if (env->has(varName))
                    env->set(varName, QuantumValue(s));
            }
        }
        else if (obj.isDict())
        {
            std::string key = idx.toString();
            auto dict = obj.asDict();
            (*dict)[key] = applyOp(dict->count(key) ? (*dict)[key] : QuantumValue());
        }
        else if (obj.isInstance() && idx.isString())
        {
            // Pointer-to-member write: b1.*ptrtest = value
            auto inst = obj.asInstance();
            std::string fieldName = idx.asString();
            auto cur = inst->fields.count(fieldName) ? inst->fields[fieldName] : QuantumValue();
            inst->setField(fieldName, applyOp(cur));
        }
    }
    else if (target.is<MemberExpr>())
    {
        auto &me = target.as<MemberExpr>();
        auto obj = evaluate(*me.object);
        if (obj.isInstance())
        {
            auto inst = obj.asInstance();
            auto cur = inst->fields.count(me.member) ? inst->fields[me.member] : QuantumValue();
            inst->setField(me.member, applyOp(cur));
        }
        else if (obj.isDict())
        {
            (*obj.asDict())[me.member] = applyOp(obj.asDict()->count(me.member) ? (*obj.asDict())[me.member] : QuantumValue());
        }
    }
    else if (target.is<DerefExpr>())
    {
        // *ptr = value  /  *(ptr + i) = value
        auto &de = target.as<DerefExpr>();
        auto ptrVal = evaluate(*de.operand);
        // C-style: *dst++ = ch where dst is a string — write char at position 0 then advance
        // We can't mutate a string-value in place; this is a no-op for interpreter purposes
        // (the copy is already handled by the += advancement). Just silently skip.
        if (ptrVal.isString())
        {
            // Advance dst by 1 (like *dst++ side effect)
            if (de.operand->is<Identifier>())
            {
                const std::string &varName = de.operand->as<Identifier>().name;
                if (env->has(varName))
                {
                    std::string s = ptrVal.asString();
                    if (!s.empty())
                        env->set(varName, QuantumValue(s.substr(1)));
                }
            }
            return; // ignore the write — can't mutate a C-style char* string in place
        }
        // Nil dereference write: *p = val where p is uninitialized char array
        // Auto-init as an empty string buffer and treat as string append
        if (ptrVal.isNil() && de.operand->is<Identifier>())
        {
            const std::string &varName = de.operand->as<Identifier>().name;
            QuantumValue newVal = applyOp(QuantumValue(std::string("")));
            // Convert number to char if needed
            std::string buf;
            if (newVal.isNumber())
                buf = std::string(1, (char)(int)newVal.asNumber());
            else if (newVal.isString())
                buf = newVal.asString();
            if (env->has(varName))
                env->set(varName, QuantumValue(buf));
            else
                env->define(varName, QuantumValue(buf));
            return;
        }
        if (!ptrVal.isPointer())
            throw RuntimeError("Cannot dereference non-pointer");
        auto ptr = ptrVal.asPointer();
        if (ptr->isNull())
            throw RuntimeError("Null pointer dereference");
        // Array-pointer with offset: *(ptr + i) = value
        // Unwrap one level if cell itself holds a pointer (double-indirection)
        auto cell = ptr->cell;
        while (cell->isPointer())
            cell = cell->asPointer()->cell;
        if (cell->isArray())
        {
            auto arr = cell->asArray();
            int i = ptr->offset;
            if (i < 0)
                i += (int)arr->size();
            if (i < 0 || i >= (int)arr->size())
                throw IndexError("Pointer write out of bounds (offset " + std::to_string(ptr->offset) + ")");
            (*arr)[i] = applyOp((*arr)[i]);
        }
        else
        {
            *cell = applyOp(*cell);
        }
    }
    else if (target.is<ArrowExpr>())
    {
        // ptr->member = value
        auto &ae = target.as<ArrowExpr>();
        auto ptrVal = evaluate(*ae.object);
        if (!ptrVal.isPointer())
            throw RuntimeError("Arrow operator requires pointer");
        auto ptr = ptrVal.asPointer();
        if (ptr->isNull())
            throw RuntimeError("Null pointer dereference via ->");
        auto &cell = *ptr->cell;
        if (cell.isInstance())
        {
            auto inst = cell.asInstance();
            auto cur = inst->fields.count(ae.member) ? inst->fields[ae.member] : QuantumValue();
            inst->setField(ae.member, applyOp(cur));
        }
        else if (cell.isDict())
        {
            (*cell.asDict())[ae.member] = applyOp(cell.asDict()->count(ae.member) ? (*cell.asDict())[ae.member] : QuantumValue());
        }
        else
            throw RuntimeError("Cannot use -> on non-struct type");
    }
}

QuantumValue Interpreter::evalAssign(AssignExpr &e)
{
    // Tuple unpacking: a, b, c = someIterable
    if (e.op == "unpack")
    {
        auto val = evaluate(*e.value);
        auto &lhs = e.target->as<TupleLiteral>();
        // val should be array/tuple
        std::shared_ptr<Array> arr;
        if (val.isArray())
            arr = val.asArray();
        else
        {
            // wrap scalar in single-element array
            arr = std::make_shared<Array>();
            arr->push_back(val);
        }
        for (size_t i = 0; i < lhs.elements.size(); ++i)
        {
            QuantumValue item = (i < arr->size()) ? (*arr)[i] : QuantumValue();
            auto &target = *lhs.elements[i];
            if (target.is<Identifier>())
            {
                auto &name = target.as<Identifier>().name;
                if (!env->has(name))
                    env->define(name, item);
                else
                    env->set(name, item);
            }
        }
        return val;
    }
    auto val = evaluate(*e.value);
    // Post-increment/decrement: return OLD value, then modify
    if (e.op == "post+=" || e.op == "post-=")
    {
        QuantumValue oldVal;
        try
        {
            oldVal = evaluate(*e.target);
        }
        catch (...)
        {
        }
        std::string realOp = (e.op == "post+=") ? "+=" : "-=";
        setLValue(*e.target, val, realOp);
        return oldVal;
    }
    setLValue(*e.target, val, e.op);
    return val;
}

QuantumValue Interpreter::evalCall(CallExpr &e)
{
    // Special: method call via MemberExpr callee
    if (e.callee->is<MemberExpr>())
    {
        auto &me = e.callee->as<MemberExpr>();

        // Special case: super().__init__(args) or super().__str__(args)
        // super() returns nil; we need to treat this as super.method(args) instead
        if (me.object->is<CallExpr>() && me.object->as<CallExpr>().callee->is<SuperExpr>())
        {
            std::string methodName = me.member;
            // Normalize Python dunder names to internal names
            if (methodName == "__init__")
                methodName = "init";
            else if (methodName == "__str__" || methodName == "__repr__")
                methodName = "__str__";
            std::vector<QuantumValue> args;
            for (auto &a : e.args)
                args.push_back(evaluate(*a));
            // Call as super.method(args)
            auto selfVal = env->get("self");
            auto inst = selfVal.asInstance();
            auto parentClass = inst->klass->base;
            if (parentClass)
            {
                auto k = parentClass.get();
                while (k)
                {
                    auto it = k->methods.find(methodName);
                    if (it != k->methods.end())
                        return callInstanceMethod(inst, it->second, std::move(args));
                    k = k->base.get();
                }
            }
            return QuantumValue();
        }

        auto obj = evaluate(*me.object);
        std::vector<QuantumValue> args;
        for (auto &a : e.args)
            args.push_back(evaluate(*a));
        return callMethod(obj, me.member, std::move(args));
    }

    // Super constructor call: super(args)
    if (e.callee->is<SuperExpr>())
    {
        auto &se = e.callee->as<SuperExpr>();
        std::vector<QuantumValue> args;
        for (auto &a : e.args)
            args.push_back(evaluate(*a));

        if (!se.method.empty())
        {
            // super.method(args) — find method in parent class and call with current self
            auto selfVal = env->get("self");
            auto inst = selfVal.asInstance();
            auto parentClass = inst->klass->base;
            if (!parentClass)
                throw RuntimeError("No parent class for super call");
            // Walk parent chain to find the method
            auto k = parentClass.get();
            while (k)
            {
                auto it = k->methods.find(se.method);
                if (it != k->methods.end())
                    return callInstanceMethod(inst, it->second, std::move(args));
                k = k->base.get();
            }
            throw RuntimeError("Method '" + se.method + "' not found in parent class");
        }
        else
        {
            // super(args) — call parent constructor
            auto selfVal = env->get("self");
            auto inst = selfVal.asInstance();
            auto parentClass = inst->klass->base;
            if (!parentClass)
                throw RuntimeError("No parent class for super() call");
            // Find init in parent chain
            auto k = parentClass.get();
            while (k)
            {
                auto it = k->methods.find("init");
                if (it != k->methods.end())
                    return callInstanceMethod(inst, it->second, std::move(args));
                k = k->base.get();
            }
            return QuantumValue(); // no parent init
        }
    }

    // C++ pointer-to-member call: (b1.*fp)() where fp holds method name as string
    // Parsed as CallExpr{ IndexExpr{b1, fp}, args }
    if (e.callee->is<IndexExpr>())
    {
        auto &ie = e.callee->as<IndexExpr>();
        auto obj = evaluate(*ie.object);
        auto idx = evaluate(*ie.index);
        if (obj.isInstance() && idx.isString())
        {
            std::string methodName = idx.asString();
            std::vector<QuantumValue> args;
            for (auto &a : e.args)
                args.push_back(evaluate(*a));
            return callMethod(obj, methodName, std::move(args));
        }
    }
    // C++ pointer-to-member call via pointer: (bp->*fp)()
    // bp->*fp is parsed as ArrowExpr{bp}.*fp or similar
    // Handle: evaluate the callee; if nil and we came from an IndexExpr, try method lookup
    auto callee = evaluate(*e.callee);
    std::vector<QuantumValue> args;

    for (size_t i = 0; i < e.args.size(); i++)
    {
        if (e.args[i]->is<UnaryExpr>() && e.args[i]->as<UnaryExpr>().op == "**")
        {
            auto dictVal = evaluate(*e.args[i]->as<UnaryExpr>().operand);
            if (dictVal.isDict())
            {
                auto map = dictVal.asDict();
                if (callee.isFunction() && std::holds_alternative<std::shared_ptr<QuantumFunction>>(callee.data))
                {
                    auto fn = callee.asFunction();
                    size_t provided = args.size();
                    for (size_t p = provided; p < fn->params.size(); p++)
                    {
                        auto it = map->find(fn->params[p]);
                        if (it != map->end())
                            args.push_back(it->second);
                        else
                            args.push_back(QuantumValue());
                    }
                }
                else
                {
                    for (auto &pair : *map)
                        args.push_back(pair.second);
                }
            }
            continue;
        }

        // For QuantumFunctions with reference params, pass live cells for ref args
        // so the callee can write back through them
        if (callee.isFunction() && std::holds_alternative<std::shared_ptr<QuantumFunction>>(callee.data))
        {
            auto fn = callee.asFunction();
            bool isRef = (i < fn->paramIsRef.size()) && fn->paramIsRef[i];
            if (isRef && e.args[i]->is<Identifier>())
            {
                const std::string &varName = e.args[i]->as<Identifier>().name;
                auto cell = env->getCell(varName);
                if (cell)
                {
                    auto ptr = std::make_shared<QuantumPointer>();
                    ptr->cell = cell;
                    ptr->varName = varName;
                    args.push_back(QuantumValue(ptr));
                    continue;
                }
            }
        }
        args.push_back(evaluate(*e.args[i]));
    }

    // Class construction: ClassName(args)
    if (callee.isClass())
    {
        auto klass = callee.asClass();
        auto inst = std::make_shared<QuantumInstance>();
        inst->klass = klass;
        inst->env = std::make_shared<Environment>(env);
        QuantumValue instVal(inst);

        // Find init in class hierarchy — prefer exact arg-count match for overloading
        auto k = klass.get();
        std::shared_ptr<QuantumFunction> initFn;
        std::string overloadKey = "init#" + std::to_string(args.size());
        while (k && !initFn)
        {
            // Try exact overload match first
            auto oit = k->methods.find(overloadKey);
            if (oit != k->methods.end())
            {
                initFn = oit->second;
                break;
            }
            // Fall back to generic init
            auto it = k->methods.find("init");
            if (it != k->methods.end())
                initFn = it->second;
            k = k->base.get();
        }

        if (initFn)
        {
            auto scope = std::make_shared<Environment>(initFn->closure);
            scope->define("self", instVal);
            scope->define("this", instVal);
            // Bind params (skip leading "self"/"this" if present)
            size_t paramStart = (!initFn->params.empty() && (initFn->params[0] == "self" || initFn->params[0] == "this")) ? 1 : 0;
            for (size_t i = paramStart; i < initFn->params.size(); i++)
            {
                size_t ai = i - paramStart;
                scope->define(initFn->params[i], ai < args.size() ? args[ai] : QuantumValue());
            }
            try
            {
                execBlock(initFn->body->as<BlockStmt>(), scope);
            }
            catch (ReturnSignal &)
            {
            }
        }
        return instVal;
    }

    if (callee.isFunction())
    {
        if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(callee.data))
        {
            // C++ style: if we're inside a method and called a bare function name,
            // and that function is a class method (not in regular env), call it as instance method
            if (e.callee->is<Identifier>())
            {
                const std::string &fname = e.callee->as<Identifier>().name;
                try
                {
                    env->get(fname); // if this succeeds, it's a regular function — call normally
                    return callFunction(callee.asFunction(), std::move(args));
                }
                catch (NameError &)
                {
                    // It came from self — call as instance method
                    try
                    {
                        auto selfVal = env->get("self");
                        if (selfVal.isInstance())
                            return callInstanceMethod(selfVal.asInstance(), callee.asFunction(), std::move(args));
                    }
                    catch (...)
                    {
                    }
                    return callFunction(callee.asFunction(), std::move(args));
                }
            }
            return callFunction(callee.asFunction(), std::move(args));
        }
        if (std::holds_alternative<std::shared_ptr<QuantumNative>>(callee.data))
            return callNative(callee.asNative(), std::move(args));
    }
    throw TypeError("Cannot call " + callee.typeName());
}

QuantumValue Interpreter::callFunction(std::shared_ptr<QuantumFunction> fn, std::vector<QuantumValue> args)
{
    auto scope = std::make_shared<Environment>(fn->closure);
    for (size_t i = 0; i < fn->params.size(); i++)
    {
        bool isRef = (i < fn->paramIsRef.size()) && fn->paramIsRef[i];
        QuantumValue v = i < args.size() ? args[i] : (fn->defaultArgs.size() > i && fn->defaultArgs[i] ? evaluate(*(fn->defaultArgs[i])) : QuantumValue());

        if (isRef)
        {
            // Pass-by-reference: the arg must be a QuantumPointer (from &var or the var's cell).
            // If the caller already passed a pointer, use its cell directly in scope.
            // If the caller passed a plain value (e.g. cubeByRef(b) without explicit &),
            // we need the caller's env to have a live cell for 'b'. We wrap the value
            // in a new shared cell and bind it — then sync back after the call.
            if (v.isPointer())
            {
                // Already a pointer — bind param name to a local that shares the cell
                auto ptr = v.asPointer();
                if (!ptr->isNull())
                {
                    scope->defineRef(fn->params[i], ptr->cell);
                    continue;
                }
            }
            // Plain value passed for a ref param: create a cell, bind it, sync after call
            auto cell = std::make_shared<QuantumValue>(v);
            scope->defineRef(fn->params[i], cell);
            // After the call we'll write *cell back to the caller arg — but we can't
            // reach caller's env here. The cell IS shared though, so if the caller
            // passed via getCell the sync is automatic.
            continue;
        }

        if (fn->params[i].front() == '[' && fn->params[i].back() == ']')
        {
            // destructuring: "[dr,dc]"
            std::string inner = fn->params[i].substr(1, fn->params[i].size() - 2);
            std::vector<std::string> keys;
            size_t start = 0, found;
            while ((found = inner.find(',', start)) != std::string::npos)
            {
                keys.push_back(inner.substr(start, found - start));
                start = found + 1;
            }
            if (start < inner.size())
                keys.push_back(inner.substr(start));

            if (v.isArray())
            {
                auto arr = v.asArray();
                for (size_t j = 0; j < keys.size(); j++)
                {
                    if (j < arr->size())
                        scope->define(keys[j], (*arr)[j]);
                    else
                        scope->define(keys[j], QuantumValue());
                }
            }
            else
            {
                for (auto &k : keys)
                    scope->define(k, QuantumValue());
            }
            continue;
        }

        scope->define(fn->params[i], std::move(v));
    }
    QuantumValue result;
    try
    {
        execBlock(fn->body->as<BlockStmt>(), scope);
    }
    catch (ReturnSignal &r)
    {
        result = std::move(r.value);
    }

    // C++ destructor simulation: call __del__ on any instances defined in this scope
    // that have a __del__ method (e.g. objects going out of scope)
    for (auto &[varName, val] : scope->getVars())
    {
        if (val.isInstance())
        {
            auto inst = val.asInstance();
            auto k = inst->klass.get();
            while (k)
            {
                auto it = k->methods.find("__del__");
                if (it != k->methods.end())
                {
                    try
                    {
                        callInstanceMethod(inst, it->second, {});
                    }
                    catch (...)
                    {
                    }
                    break;
                }
                k = k->base.get();
            }
        }
    }

    return result;
}

QuantumValue Interpreter::callNative(std::shared_ptr<QuantumNative> fn, std::vector<QuantumValue> args)
{
    return fn->fn(std::move(args));
}

QuantumValue Interpreter::callInstanceMethod(std::shared_ptr<QuantumInstance> inst, std::shared_ptr<QuantumFunction> fn, std::vector<QuantumValue> args)
{
    QuantumValue instVal(inst);
    auto scope = std::make_shared<Environment>(fn->closure);
    scope->define("self", instVal);
    scope->define("this", instVal);
    size_t paramStart = (!fn->params.empty() && (fn->params[0] == "self" || fn->params[0] == "this")) ? 1 : 0;
    for (size_t i = paramStart; i < fn->params.size(); i++)
    {
        size_t ai = i - paramStart;
        scope->define(fn->params[i], ai < args.size() ? args[ai] : QuantumValue());
    }
    try
    {
        execBlock(fn->body->as<BlockStmt>(), scope);
    }
    catch (ReturnSignal &r)
    {
        return r.value;
    }
    return QuantumValue();
}

QuantumValue Interpreter::evalIndex(IndexExpr &e)
{
    auto obj = evaluate(*e.object);
    auto idx = evaluate(*e.index);

    // Pointer-to-array indexing: float* arr = new float[n];  arr[i]
    if (obj.isPointer())
    {
        auto ptr = obj.asPointer();
        if (!ptr->isNull() && ptr->cell->isArray())
        {
            obj = *ptr->cell; // unwrap to the array
        }
        else
        {
            // Pointer arithmetic: ptr[0] == *ptr
            int i = idx.isNumber() ? (int)idx.asNumber() : 0;
            if (i == 0 && !ptr->isNull())
                return *ptr->cell;
            throw TypeError("Cannot index pointer with offset " + std::to_string(i));
        }
    }

    if (obj.isArray())
    {
        int i;
        if (idx.isNumber())
            i = (int)idx.asNumber();
        else if (idx.isString())
        {
            try
            {
                i = std::stoi(idx.asString());
            }
            catch (...)
            {
                throw TypeError("Expected number in index, got " + idx.typeName());
            }
        }
        else
            throw TypeError("Expected number in index, got " + idx.typeName());
        auto arr = obj.asArray();
        if (i < 0)
            i += (int)arr->size();
        if (i < 0 || i >= (int)arr->size())
            return QuantumValue(); // JS: arr[outOfBounds] → undefined (nil)
        return (*arr)[i];
    }
    if (obj.isDict())
    {
        auto dict = obj.asDict();
        auto key = idx.toString();
        auto it = dict->find(key);
        if (it == dict->end())
            return QuantumValue();
        return it->second;
    }
    if (obj.isString())
    {
        int i = (int)toNum(idx, "index");
        const auto &s = obj.asString();
        if (i < 0)
            i += (int)s.size();
        // C-style: index == length returns the null terminator '\0'
        if (i == (int)s.size())
            return QuantumValue(std::string(1, '\0'));
        if (i < 0 || i >= (int)s.size())
            throw IndexError("String index out of range");
        return QuantumValue(std::string(1, s[i]));
    }
    // Pointer-to-member: obj.*ptrMember where ptrMember holds the member name as string
    if (obj.isInstance() && idx.isString())
    {
        auto inst = obj.asInstance();
        return inst->getField(idx.asString());
    }
    // Nil indexing: uninitialized C arrays — return nil (auto-create on write via setLValue)
    if (obj.isNil())
        return QuantumValue();
    std::cout << "[DEBUG] evalIndex fail. obj=" << obj.typeName() << " idx=" << idx.typeName();
    if (idx.isString())
        std::cout << " val=" << idx.asString();
    std::cout << "\n";
    throw TypeError("Cannot index " + obj.typeName());
}

QuantumValue Interpreter::evalMember(MemberExpr &e)
{
    auto obj = evaluate(*e.object);
    if (obj.isPointer())
    {
        auto ptr = obj.asPointer();
        if (!ptr->isNull())
            obj = *ptr->cell;
    }
    // Optional chaining: nil?.member → nil (handles t?.val where t is null/nil)
    if (obj.isNil())
        return QuantumValue();
    if (obj.isInstance())
    {
        auto inst = obj.asInstance();
        // Check for __str__ when toString is called
        try
        {
            return inst->getField(e.member);
        }
        catch (NameError &)
        {
            auto k = inst->klass.get();
            while (k)
            {
                auto sit = k->staticFields.find(e.member);
                if (sit != k->staticFields.end())
                    return sit->second;
                k = k->base.get();
            }
            throw TypeError("No member '" + e.member + "' on instance of " + inst->klass->name);
        }
    }
    if (obj.isClass())
    {
        // Static method/field access: ClassName.method or ClassName.field
        auto klass = obj.asClass();
        auto it = klass->staticMethods.find(e.member);
        if (it != klass->staticMethods.end())
            return QuantumValue(it->second);
        auto fit = klass->staticFields.find(e.member);
        if (fit != klass->staticFields.end())
            return fit->second;
        throw TypeError("No static member '" + e.member + "' on class " + klass->name);
    }
    if (obj.isDict())
    {
        auto dict = obj.asDict();
        auto it = dict->find(e.member);
        return it != dict->end() ? it->second : QuantumValue();
    }
    // String properties
    if (obj.isString() && e.member == "length")
        return QuantumValue((double)obj.asString().size());
    if (obj.isArray() && e.member == "length")
        return QuantumValue((double)obj.asArray()->size());
    // Native object member access: String.fromCharCode, Array.from, Object.keys, etc.
    // Return a bound callable that captures obj and member name.
    if (obj.isNative())
    {
        auto natObjCopy = obj; // copy to capture
        std::string memCopy = e.member;
        auto bound = std::make_shared<QuantumNative>();
        bound->name = obj.asNative()->name + "." + e.member;
        bound->fn = [this, natObjCopy, memCopy](std::vector<QuantumValue> args) mutable -> QuantumValue
        {
            QuantumValue o = natObjCopy;
            return callMethod(o, memCopy, std::move(args));
        };
        return QuantumValue(bound);
    }
    // String/array method access as bound callable
    if (obj.isString() || obj.isArray())
    {
        auto objCopy = obj;
        std::string memCopy = e.member;
        auto bound = std::make_shared<QuantumNative>();
        bound->name = memCopy;
        bound->fn = [this, objCopy, memCopy](std::vector<QuantumValue> args) mutable -> QuantumValue
        {
            QuantumValue o = objCopy;
            return callMethod(o, memCopy, std::move(args));
        };
        return QuantumValue(bound);
    }
    throw TypeError("No member '" + e.member + "' on " + obj.typeName());
}

QuantumValue Interpreter::evalArray(ArrayLiteral &e)
{
    auto arr = std::make_shared<Array>();
    for (auto &el : e.elements)
    {
        if (el->is<UnaryExpr>() && el->as<UnaryExpr>().op == "...")
        {
            auto spreadVal = evaluate(*el->as<UnaryExpr>().operand);
            if (spreadVal.isArray())
            {
                for (auto &sv : *spreadVal.asArray())
                    arr->push_back(sv);
            }
            else if (spreadVal.isString())
            {
                for (char c : spreadVal.asString())
                    arr->push_back(QuantumValue(std::string(1, c)));
            }
            else
            {
                arr->push_back(spreadVal);
            }
        }
        else
        {
            arr->push_back(evaluate(*el));
        }
    }
    return QuantumValue(arr);
}

QuantumValue Interpreter::evalDict(DictLiteral &e)
{
    auto dict = std::make_shared<Dict>();
    for (auto &[k, v] : e.pairs)
    {
        if (!k)
        {
            // Spread: {...obj} — copy all fields from obj into dict
            auto spreadVal = evaluate(*v);
            // unwrap UnaryExpr "..." if needed (the value is already the operand)
            if (spreadVal.isDict())
                for (auto &p : *spreadVal.asDict())
                    (*dict)[p.first] = p.second;
            else if (spreadVal.isInstance())
                for (auto &p : spreadVal.asInstance()->fields)
                    (*dict)[p.first] = p.second;
            continue;
        }
        auto key = evaluate(*k);
        auto val = evaluate(*v);
        (*dict)[key.toString()] = std::move(val);
    }
    return QuantumValue(dict);
}

QuantumValue Interpreter::evalListComp(ListComp &e)
{
    auto result = std::make_shared<Array>();
    auto iter = evaluate(*e.iterable);
    bool hasTuple = e.vars.size() >= 2;

    auto processItem = [&](QuantumValue item)
    {
        auto scope = std::make_shared<Environment>(env);
        if (hasTuple)
        {
            if (item.isArray() && item.asArray()->size() >= 2)
            {
                scope->define(e.vars[0], (*item.asArray())[0]);
                scope->define(e.vars[1], (*item.asArray())[1]);
            }
            else
            {
                scope->define(e.vars[0], item);
                if (e.vars.size() > 1)
                    scope->define(e.vars[1], QuantumValue());
            }
        }
        else
            scope->define(e.vars[0], item);

        // Evaluate optional filter condition
        if (e.condition)
        {
            auto saved = env;
            env = scope;
            bool pass = evaluate(*e.condition).isTruthy();
            env = saved;
            if (!pass)
                return;
        }

        // Evaluate the expression in the loop scope
        auto saved = env;
        env = scope;
        auto val = evaluate(*e.expr);
        env = saved;
        result->push_back(std::move(val));
    };

    if (iter.isArray())
        for (auto &item : *iter.asArray())
            processItem(item);
    else if (iter.isString())
        for (char c : iter.asString())
            processItem(QuantumValue(std::string(1, c)));
    else if (iter.isDict())
        for (auto &[k, v] : *iter.asDict())
        {
            auto pair = std::make_shared<Array>();
            pair->push_back(QuantumValue(k));
            pair->push_back(v);
            processItem(QuantumValue(pair));
        }

    return QuantumValue(result);
}

QuantumValue Interpreter::evalLambda(LambdaExpr &e)
{
    auto fn = std::make_shared<QuantumFunction>();
    fn->name = "<lambda>";
    fn->params = e.params;
    for (auto &da : e.defaultArgs)
        fn->defaultArgs.push_back(da.get());
    fn->body = e.body.get();
    fn->closure = env;
    return QuantumValue(fn);
}

// ─── C++ Pointer Operators ────────────────────────────────────────────────────

QuantumValue Interpreter::evalAddressOf(AddressOfExpr &e)
{
    // &var — returns a live shared pointer to the variable's cell
    if (e.operand->is<Identifier>())
    {
        const std::string &name = e.operand->as<Identifier>().name;

        // Ensure variable exists
        if (!env->has(name))
            env->define(name, QuantumValue());

        auto cell = env->getCell(name);
        if (!cell)
            throw RuntimeError("Could not get address of '" + name + "'");

        auto ptr = std::make_shared<QuantumPointer>();
        ptr->cell = cell;
        ptr->varName = name;
        ptr->offset = 0;
        return QuantumValue(ptr);
    }

    // &ClassName::memberName — C++ pointer-to-member: return member name as string value
    if (e.operand->is<MemberExpr>())
    {
        auto &me = e.operand->as<MemberExpr>();
        // Return the member name as a string so b1.*ptrtest works via IndexExpr
        return QuantumValue(me.member);
    }
    // &ClassName::memberName where parsePrimary combined them into "ClassName::memberName"
    if (e.operand->is<Identifier>())
    {
        const std::string &name = e.operand->as<Identifier>().name;
        auto sep = name.rfind("::");
        if (sep != std::string::npos)
            return QuantumValue(name.substr(sep + 2)); // return just "memberName"
    }

    // &arr[i] or &obj.field — evaluate the expression, wrap result in a temporary cell
    // This covers scanf("%d", &arr[i]) and similar patterns where C passes a pointer
    // to a sub-object. We wrap the current value; writes through this pointer won't
    // reflect back, but for most interpreter uses (scanf, pass-by-pointer) this is fine.
    auto val = evaluate(*e.operand);
    auto cell = std::make_shared<QuantumValue>(std::move(val));
    auto ptr = std::make_shared<QuantumPointer>();
    ptr->cell = cell;
    ptr->varName = "";
    ptr->offset = 0;
    return QuantumValue(ptr);
}

QuantumValue Interpreter::evalDeref(DerefExpr &e)
{
    // *ptr — read the value the pointer points to
    auto ptrVal = evaluate(*e.operand);

    // C-style: if dereferencing a string directly (e.g. const char* s passed as string value),
    // treat it as reading the first character (offset 0). This supports while(*s) patterns.
    if (ptrVal.isString())
    {
        const std::string &s = ptrVal.asString();
        if (s.empty())
            return QuantumValue(0.0); // null terminator
        return QuantumValue((double)(unsigned char)s[0]);
    }

    // Nil dereference: uninitialized pointer — return 0 (null terminator / falsy)
    if (ptrVal.isNil())
        return QuantumValue(0.0);

    if (!ptrVal.isPointer())
        throw RuntimeError("Cannot dereference non-pointer (type: " + ptrVal.typeName() + ")");
    auto ptr = ptrVal.asPointer();
    if (ptr->isNull())
        throw RuntimeError("Null pointer dereference");
    // Unwrap one level if cell itself holds a pointer (double-indirection)
    auto cell = ptr->cell;
    while (cell->isPointer())
        cell = cell->asPointer()->cell;
    // If the cell holds an array, index into it using offset: *(ptr + i)
    if (cell->isArray())
    {
        auto arr = cell->asArray();
        int i = ptr->offset;
        if (i < 0)
            i += (int)arr->size();
        if (i < 0 || i >= (int)arr->size())
            throw IndexError("Pointer dereference out of bounds (offset " + std::to_string(ptr->offset) + ")");
        return (*arr)[i];
    }
    // If the cell holds a string, index into it by offset (C-style char pointer)
    if (cell->isString())
    {
        const std::string &s = cell->toString();
        int i = ptr->offset;
        if (i >= 0 && i < (int)s.size())
            return QuantumValue((double)(unsigned char)s[i]);
        return QuantumValue(0.0); // null terminator
    }
    return *cell;
}

QuantumValue Interpreter::evalArrow(ArrowExpr &e)
{
    // ptr->member — dereference pointer then access member
    auto ptrVal = evaluate(*e.object);
    if (!ptrVal.isPointer())
        throw RuntimeError("Arrow operator requires a pointer (type: " + ptrVal.typeName() + ")");
    auto ptr = ptrVal.asPointer();
    if (ptr->isNull())
        throw RuntimeError("Null pointer dereference via ->");
    auto &cell = *ptr->cell;
    if (cell.isInstance())
    {
        auto inst = cell.asInstance();
        try
        {
            return inst->getField(e.member);
        }
        catch (...)
        {
            throw TypeError("No member '" + e.member + "' on pointed-to instance");
        }
    }
    if (cell.isDict())
    {
        auto dict = cell.asDict();
        auto it = dict->find(e.member);
        return it != dict->end() ? it->second : QuantumValue();
    }
    throw RuntimeError("Cannot use -> on type " + cell.typeName());
}

QuantumValue Interpreter::evalNewExpr(NewExpr &e)
{
    static const std::unordered_set<std::string> primitives = {
        "int", "long", "short", "unsigned", "float", "double", "char", "bool"};

    // ── new T[n] — allocate array of n zero-initialised elements ─────────────
    if (e.isArray)
    {
        int n = 0;
        if (e.sizeExpr)
        {
            auto sv = evaluate(*e.sizeExpr);
            n = sv.isNumber() ? (int)sv.asNumber() : 0;
        }
        if (n < 0)
            n = 0;
        // Allocate at least 1 element: prevents immediate IndexError when n=0
        // (e.g. empty stdin → n defaults to 0 → cumulative[0] would crash otherwise)
        int allocN = n < 1 ? 1 : n;
        auto arr = std::make_shared<Array>();
        arr->resize(allocN, QuantumValue(0.0));
        auto cell = std::make_shared<QuantumValue>(QuantumValue(arr));
        auto ptr = std::make_shared<QuantumPointer>();
        ptr->cell = cell;
        ptr->varName = e.typeName + "[]";
        return QuantumValue(ptr);
    }

    // ── new T(args) — single-value heap allocation ────────────────────────────
    std::vector<QuantumValue> args;
    for (auto &a : e.args)
        args.push_back(evaluate(*a));

    QuantumValue val;
    if (primitives.count(e.typeName))
    {
        val = args.empty() ? QuantumValue(0.0) : args[0];
        if (e.typeName == "int" || e.typeName == "long" || e.typeName == "short" || e.typeName == "unsigned")
        {
            if (val.isNumber())
                val = QuantumValue((double)(long long)val.asNumber());
        }
        else if (e.typeName == "char")
        {
            if (val.isNumber())
                val = QuantumValue(std::string(1, (char)(int)val.asNumber()));
            else if (val.isString())
                val = QuantumValue(val.asString().empty() ? std::string("") : std::string(1, val.asString()[0]));
        }
        else if (e.typeName == "bool")
            val = QuantumValue(val.isTruthy());
    }
    else
    {
        // Class type — construct instance and wrap in pointer
        QuantumValue classVal;
        try
        {
            classVal = env->get(e.typeName);
        }
        catch (...)
        {
        }
        if (classVal.isClass())
        {
            auto klass = classVal.asClass();
            auto inst = std::make_shared<QuantumInstance>();
            inst->klass = klass;
            inst->env = std::make_shared<Environment>(env);
            QuantumValue instVal(inst);
            auto k = klass.get();
            std::shared_ptr<QuantumFunction> initFn;
            std::string overloadKey = "init#" + std::to_string(args.size());
            while (k && !initFn)
            {
                auto oit = k->methods.find(overloadKey);
                if (oit != k->methods.end())
                {
                    initFn = oit->second;
                    break;
                }
                auto it = k->methods.find("init");
                if (it != k->methods.end())
                    initFn = it->second;
                k = k->base.get();
            }
            if (initFn)
            {
                auto scope = std::make_shared<Environment>(initFn->closure);
                scope->define("self", instVal);
                scope->define("this", instVal);
                size_t paramStart = (!initFn->params.empty() && (initFn->params[0] == "self" || initFn->params[0] == "this")) ? 1 : 0;
                for (size_t i = paramStart; i < initFn->params.size(); i++)
                {
                    size_t ai = i - paramStart;
                    scope->define(initFn->params[i], ai < args.size() ? args[ai] : QuantumValue());
                }
                try
                {
                    execBlock(initFn->body->as<BlockStmt>(), scope);
                }
                catch (ReturnSignal &)
                {
                }
            }
            auto cell = std::make_shared<QuantumValue>(instVal);
            auto ptr = std::make_shared<QuantumPointer>();
            ptr->cell = cell;
            ptr->varName = e.typeName;
            return QuantumValue(ptr);
        }
        val = args.empty() ? QuantumValue() : args[0];
    }

    auto cell = std::make_shared<QuantumValue>(val);
    auto ptr = std::make_shared<QuantumPointer>();
    ptr->cell = cell;
    ptr->varName = e.typeName;
    return QuantumValue(ptr);
}

// ─── Built-in Method Dispatch ────────────────────────────────────────────────

QuantumValue Interpreter::callMethod(QuantumValue &obj, const std::string &method, std::vector<QuantumValue> args)
{
    if (obj.isArray())
        return callArrayMethod(obj.asArray(), method, std::move(args));
    if (obj.isString())
        return callStringMethod(obj.asString(), method, std::move(args));
    if (obj.isNumber())
    {
        if (method == "toFixed")
        {
            int precision = args.empty() ? 0 : (int)args[0].asNumber();
            double num = obj.asNumber();
            std::ostringstream out;
            out << std::fixed << std::setprecision(precision) << num;
            return QuantumValue(out.str());
        }
        throw TypeError("Number has no method '" + method + "'");
    }
    // Native static method dispatch: Array.from, String.fromCharCode, etc.
    if (obj.isNative())
    {
        auto nat = obj.asNative();
        if (nat->name == "Array")
        {
            if (method == "from")
            {
                auto arr = std::make_shared<Array>();
                if (!args.empty())
                {
                    QuantumValue src = args[0];
                    QuantumValue mapFn = args.size() > 1 ? args[1] : QuantumValue();
                    bool hasMap = mapFn.isFunction() || mapFn.isNative();
                    if (src.isArray())
                    {
                        for (size_t i = 0; i < src.asArray()->size(); i++)
                        {
                            QuantumValue item = (*src.asArray())[i];
                            if (hasMap)
                                item = mapFn.isNative()
                                           ? callNative(mapFn.asNative(), {item, QuantumValue((double)i)})
                                           : callFunction(mapFn.asFunction(), {item, QuantumValue((double)i)});
                            arr->push_back(item);
                        }
                    }
                    else if (src.isDict())
                    {
                        auto d = src.asDict();
                        auto lit = d->find("length");
                        int len = (lit != d->end() && lit->second.isNumber()) ? (int)lit->second.asNumber() : 0;
                        for (int i = 0; i < len; i++)
                        {
                            QuantumValue item = QuantumValue();
                            if (hasMap)
                                item = mapFn.isNative()
                                           ? callNative(mapFn.asNative(), {QuantumValue(), QuantumValue((double)i)})
                                           : callFunction(mapFn.asFunction(), {QuantumValue(), QuantumValue((double)i)});
                            arr->push_back(item);
                        }
                    }
                    else if (src.isString())
                    {
                        for (char c : src.asString())
                            arr->push_back(QuantumValue(std::string(1, c)));
                    }
                }
                return QuantumValue(arr);
            }
            if (method == "isArray")
                return QuantumValue(!args.empty() && args[0].isArray());
            if (method == "of")
            {
                auto arr = std::make_shared<Array>();
                for (auto &a : args)
                    arr->push_back(a);
                return QuantumValue(arr);
            }
        }
        if (nat->name == "String")
        {
            if (method == "fromCharCode")
            {
                std::string result;
                for (auto &a : args)
                    result += (char)(int)(a.isNumber() ? a.asNumber() : 0.0);
                return QuantumValue(result);
            }
            if (method == "raw")
                return args.empty() ? QuantumValue(std::string("")) : QuantumValue(args[0].toString());
        }
        if (nat->name == "Number")
        {
            if (method == "isNaN")
                return QuantumValue(!args.empty() && args[0].isNumber() && std::isnan(args[0].asNumber()));
            if (method == "isFinite")
                return QuantumValue(!args.empty() && args[0].isNumber() && std::isfinite(args[0].asNumber()));
            if (method == "parseInt")
            {
                try
                {
                    return QuantumValue((double)std::stoll(args.empty() ? "0" : args[0].toString()));
                }
                catch (...)
                {
                    return QuantumValue(0.0);
                }
            }
            if (method == "parseFloat")
            {
                try
                {
                    return QuantumValue(std::stod(args.empty() ? "0" : args[0].toString()));
                }
                catch (...)
                {
                    return QuantumValue(0.0);
                }
            }
        }
    }
    // str.maketrans(from, to) — called on the str type native itself
    if (obj.isFunction() && std::holds_alternative<std::shared_ptr<QuantumNative>>(obj.data))
    {
        auto nat = obj.asNative();
        if ((nat->name == "str" || nat->name == "string") && method == "maketrans")
        {
            // maketrans(from_str, to_str) → dict mapping each char in from to corresponding in to
            if (args.size() < 2 || !args[0].isString() || !args[1].isString())
                throw RuntimeError("str.maketrans() requires two string arguments");
            const std::string &from = args[0].asString();
            const std::string &to = args[1].asString();
            auto table = std::make_shared<Dict>();
            size_t len = std::min(from.size(), to.size());
            for (size_t i = 0; i < len; i++)
                (*table)[std::string(1, from[i])] = QuantumValue(std::string(1, to[i]));
            return QuantumValue(table);
        }
    }
    if (obj.isDict())
    {
        // First check if the dict has a callable stored under that key
        // e.g. console.log(...) where console["log"] is a native function
        auto dict = obj.asDict();
        auto it = dict->find(method);
        if (it != dict->end() && it->second.isFunction())
        {
            auto &fn = it->second;
            if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                return callNative(fn.asNative(), std::move(args));
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                return callFunction(fn.asFunction(), std::move(args));
        }
        return callDictMethod(dict, method, std::move(args));
    }
    if (obj.isInstance())
    {
        auto inst = obj.asInstance();
        // Look up method in class hierarchy — prefer exact arg-count match for overloading
        std::string overloadKey = method + "#" + std::to_string(args.size());
        auto k = inst->klass.get();
        while (k)
        {
            // Try exact overload match first
            auto oit = k->methods.find(overloadKey);
            if (oit != k->methods.end())
                return callInstanceMethod(inst, oit->second, std::move(args));
            auto it = k->methods.find(method);
            if (it != k->methods.end())
                return callInstanceMethod(inst, it->second, std::move(args));
            k = k->base.get();
        }
        // Check instance fields for callable
        if (inst->fields.count(method))
        {
            auto &field = inst->fields[method];
            if (field.isFunction())
            {
                if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(field.data))
                    return callFunction(field.asFunction(), std::move(args));
                if (std::holds_alternative<std::shared_ptr<QuantumNative>>(field.data))
                    return callNative(field.asNative(), std::move(args));
            }
        }
        // Check static fields (for nested classes or static callables)
        k = inst->klass.get();
        while (k)
        {
            auto fit = k->staticFields.find(method);
            if (fit != k->staticFields.end())
            {
                auto &field = fit->second;
                if (field.isFunction())
                    return callFunction(field.asFunction(), std::move(args));
                if (field.isNative())
                    return callNative(field.asNative(), std::move(args));
                if (field.isClass())
                {
                    auto klass = field.asClass();
                    auto newInst = std::make_shared<QuantumInstance>();
                    newInst->klass = klass;
                    newInst->fields = klass->staticFields; // ensure static fields act as initial instance fields
                    auto ik = klass.get();
                    bool calledInit = false;
                    while (ik)
                    {
                        auto initIt = ik->methods.find("init");
                        if (initIt != ik->methods.end())
                        {
                            callInstanceMethod(newInst, initIt->second, std::move(args));
                            calledInit = true;
                            break;
                        }
                        ik = ik->base.get();
                    }
                    if (!calledInit && !args.empty())
                        throw TypeError(klass->name + "() takes no arguments");
                    return QuantumValue(newInst);
                }
            }
            k = k->base.get();
        }
        throw TypeError("No method '" + method + "' on instance of " + inst->klass->name);
    }
    if (obj.isClass())
    {
        auto klass = obj.asClass();
        auto it = klass->staticMethods.find(method);
        if (it != klass->staticMethods.end())
            return callFunction(it->second, std::move(args));
        // Fallback to static fields (e.g. math.sqrt where sqrt is a QuantumNative stored as a field)
        auto fit = klass->staticFields.find(method);
        if (fit != klass->staticFields.end())
        {
            if (fit->second.isNative())
                return callNative(fit->second.asNative(), args);
            if (fit->second.isFunction())
                return callFunction(fit->second.asFunction(), args);
        }
        throw TypeError("No static method '" + method + "' on class " + klass->name);
    }
    // smart-pointer / raw-pointer method stubs for weak_ptr, shared_ptr, unique_ptr
    if (obj.isPointer())
    {
        auto ptr = obj.asPointer();
        if (method == "lock")
            // weak_ptr::lock() — returns the pointer if valid, nil if expired
            return ptr->isNull() ? QuantumValue() : obj;
        if (method == "use_count")
        {
            // sp.use_count() includes: env slot(s) + obj copy in evalCall + ptr local + sp local = overhead 3
            // Subtract 3 to get the Quantum-visible reference count (how many named vars share this pointer)
            auto sp = std::get<std::shared_ptr<QuantumPointer>>(obj.data);
            long count = (long)sp.use_count() - 3;
            if (count < 1)
                count = 1;
            return QuantumValue((double)count);
        }
        if (method == "expired")
            return QuantumValue(ptr->isNull()); // null pointer → expired
        if (method == "get")
            return obj; // return the pointer itself
        if (method == "reset")
        {
            ptr->cell = nullptr; // nullify the pointer
            return QuantumValue();
        }
        if (method == "release")
            return obj;
        // Unknown pointer method — return nil silently (e.g. delete[])
        return QuantumValue();
    }
    // nil value: weak_ptr that was never assigned — .lock() returns nil, etc.
    if (obj.isNil())
    {
        if (method == "lock")
            return QuantumValue();
        if (method == "expired")
            return QuantumValue(true);
        if (method == "use_count")
            return QuantumValue(0.0);
        return QuantumValue();
    }
    throw TypeError("No method '" + method + "' on " + obj.typeName());
}

QuantumValue Interpreter::callArrayMethod(std::shared_ptr<Array> arr, const std::string &m, std::vector<QuantumValue> args)
{
    if (m == "push" || m == "append")
    {
        for (auto &a : args)
            arr->push_back(a);
        return QuantumValue();
    }
    if (m == "pop")
    {
        if (arr->empty())
            throw IndexError("pop() on empty array");
        // pop() → remove last (JS/default), pop(i) → remove at index i (Python)
        if (!args.empty() && args[0].isNumber())
        {
            int i = (int)args[0].asNumber();
            if (i < 0)
                i += (int)arr->size();
            if (i < 0 || i >= (int)arr->size())
                throw IndexError("pop() index out of range");
            auto v = (*arr)[i];
            arr->erase(arr->begin() + i);
            return v;
        }
        auto v = arr->back();
        arr->pop_back();
        return v;
    }
    if (m == "shift")
    {
        if (arr->empty())
            throw IndexError("shift() on empty array");
        auto v = arr->front();
        arr->erase(arr->begin());
        return v;
    }
    if (m == "unshift")
    {
        arr->insert(arr->begin(), args.begin(), args.end());
        return QuantumValue();
    }
    if (m == "length")
        return QuantumValue((double)arr->size());
    if (m == "copy")
    {
        return QuantumValue(std::make_shared<Array>(*arr));
    }
    if (m == "fill")
    {
        if (args.empty())
            return QuantumValue(arr);
        QuantumValue val = args[0];
        int start = args.size() > 1 && args[1].isNumber() ? (int)args[1].asNumber() : 0;
        int end = args.size() > 2 && args[2].isNumber() ? (int)args[2].asNumber() : (int)arr->size();
        if (start < 0)
            start += (int)arr->size();
        if (end < 0)
            end += (int)arr->size();
        if (start < 0)
            start = 0;
        if (end > (int)arr->size())
            end = (int)arr->size();
        for (int i = start; i < end; i++)
            (*arr)[i] = val;
        return QuantumValue(arr);
    }
    if (m == "reverse")
    {
        std::reverse(arr->begin(), arr->end());
        return QuantumValue();
    }
    if (m == "contains" || m == "includes")
    {
        for (auto &v : *arr)
        {
            if (v.toString() == args[0].toString())
                return QuantumValue(true);
        }
        return QuantumValue(false);
    }
    if (m == "join")
    {
        std::string sep = args.empty() ? "," : args[0].toString();
        std::string res;
        for (size_t i = 0; i < arr->size(); i++)
        {
            if (i)
                res += sep;
            res += (*arr)[i].toString();
        }
        return QuantumValue(res);
    }
    if (m == "slice")
    {
        int start = args.empty() ? 0 : (int)toNum(args[0], "slice");
        int end_ = args.size() > 1 ? (int)toNum(args[1], "slice") : (int)arr->size();
        if (start < 0)
            start += (int)arr->size();
        if (end_ < 0)
            end_ += (int)arr->size();
        start = std::max(0, std::min(start, (int)arr->size()));
        end_ = std::max(0, std::min(end_, (int)arr->size()));
        auto res = std::make_shared<Array>(arr->begin() + start, arr->begin() + end_);
        return QuantumValue(res);
    }
    if (m == "map")
    {
        if (args.empty())
            throw RuntimeError("map() requires function argument");
        auto res = std::make_shared<Array>();
        for (auto &item : *arr)
        {
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
    if (m == "filter")
    {
        if (args.empty())
            throw RuntimeError("filter() requires function argument");
        auto res = std::make_shared<Array>();
        for (auto &item : *arr)
        {
            std::vector<QuantumValue> callArgs = {item};
            QuantumValue fn = args[0];
            QuantumValue r;
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                r = callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                r = callNative(fn.asNative(), callArgs);
            if (r.isTruthy())
                res->push_back(item);
        }
        return QuantumValue(res);
    }
    if (m == "some")
    {
        if (args.empty())
            throw RuntimeError("some() requires function argument");
        for (auto &item : *arr)
        {
            std::vector<QuantumValue> callArgs = {item};
            QuantumValue fn = args[0];
            QuantumValue r;
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                r = callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                r = callNative(fn.asNative(), callArgs);
            if (r.isTruthy())
                return QuantumValue(true);
        }
        return QuantumValue(false);
    }
    if (m == "reduce")
    {
        if (args.empty())
            throw RuntimeError("reduce() requires function argument");

        QuantumValue acc;
        size_t startIdx = 0;
        if (args.size() > 1)
        {
            acc = args[1];
        }
        else if (!arr->empty())
        {
            acc = (*arr)[0];
            startIdx = 1;
        }
        else
        {
            throw RuntimeError("Reduce of empty array with no initial value");
        }

        QuantumValue fn = args[0];
        for (size_t i = startIdx; i < arr->size(); i++)
        {
            std::vector<QuantumValue> callArgs = {acc, (*arr)[i]};
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                acc = callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                acc = callNative(fn.asNative(), callArgs);
        }
        return acc;
    }
    if (m == "forEach")
    {
        if (args.empty())
            throw RuntimeError("forEach() requires function argument");
        for (auto &item : *arr)
        {
            std::vector<QuantumValue> callArgs = {item};
            QuantumValue fn = args[0];
            if (std::holds_alternative<std::shared_ptr<QuantumFunction>>(fn.data))
                callFunction(fn.asFunction(), callArgs);
            else if (std::holds_alternative<std::shared_ptr<QuantumNative>>(fn.data))
                callNative(fn.asNative(), callArgs);
        }
        return QuantumValue();
    }
    if (m == "sort")
    {
        std::sort(arr->begin(), arr->end(), [](const QuantumValue &a, const QuantumValue &b)
                  {
            if (a.isNumber() && b.isNumber()) return a.asNumber() < b.asNumber();
            return a.toString() < b.toString(); });
        return QuantumValue(arr);
    }
    throw TypeError("Array has no method '" + m + "'");
}

QuantumValue Interpreter::callStringMethod(const std::string &str, const std::string &m, std::vector<QuantumValue> args)
{
    if (m == "length" || m == "len")
        return QuantumValue((double)str.size());
    if (m == "upper")
    {
        std::string r = str;
        std::transform(r.begin(), r.end(), r.begin(), ::toupper);
        return QuantumValue(r);
    }
    if (m == "lower")
    {
        std::string r = str;
        std::transform(r.begin(), r.end(), r.begin(), ::tolower);
        return QuantumValue(r);
    }
    if (m == "trim")
    {
        size_t s = str.find_first_not_of(" \t\n\r");
        size_t e = str.find_last_not_of(" \t\n\r");
        if (s == std::string::npos)
            return QuantumValue(std::string(""));
        return QuantumValue(str.substr(s, e - s + 1));
    }
    if (m == "split")
    {
        std::string sep = args.empty() ? " " : args[0].toString();
        auto arr = std::make_shared<Array>();
        // Detect regex pattern: /pattern/flags
        bool isRegex = sep.size() >= 2 && sep.front() == '/' && sep.back() != '\\';
        if (isRegex)
        {
            // Find closing /
            size_t rpos = sep.rfind('/');
            if (rpos > 0)
            {
                std::string pattern = sep.substr(1, rpos - 1);
                // Handle common patterns: \s+ means whitespace split
                bool whitespace = (pattern == "\\s+" || pattern == "\\s*" || pattern == " +" || pattern == " *");
                if (whitespace || pattern.empty())
                {
                    // Split on whitespace
                    size_t i = 0;
                    while (i < str.size())
                    {
                        while (i < str.size() && std::isspace((unsigned char)str[i]))
                            i++;
                        if (i >= str.size())
                            break;
                        size_t start = i;
                        while (i < str.size() && !std::isspace((unsigned char)str[i]))
                            i++;
                        arr->push_back(QuantumValue(str.substr(start, i - start)));
                    }
                    return QuantumValue(arr);
                }
            }
        }
        if (sep.empty())
        {
            for (char c : str)
                arr->push_back(QuantumValue(std::string(1, c)));
            return QuantumValue(arr);
        }
        size_t pos = 0, found;
        while ((found = str.find(sep, pos)) != std::string::npos)
        {
            arr->push_back(QuantumValue(str.substr(pos, found - pos)));
            pos = found + sep.size();
        }
        arr->push_back(QuantumValue(str.substr(pos)));
        return QuantumValue(arr);
    }
    if (m == "join")
    {
        if (args.empty() || !args[0].isArray())
            throw TypeError("join() requires an array argument");
        std::string result;
        auto arr = args[0].asArray();
        for (size_t i = 0; i < arr->size(); i++)
        {
            if (i > 0)
                result += str;
            result += (*arr)[i].toString();
        }
        return QuantumValue(result);
    }
    if (m == "contains")
    {
        return QuantumValue(str.find(args[0].toString()) != std::string::npos);
    }
    if (m == "starts_with")
    {
        auto p = args[0].toString();
        return QuantumValue(str.size() >= p.size() && str.substr(0, p.size()) == p);
    }
    if (m == "ends_with")
    {
        auto p = args[0].toString();
        return QuantumValue(str.size() >= p.size() && str.substr(str.size() - p.size()) == p);
    }
    if (m == "replace")
    {
        if (args.size() < 2)
            throw RuntimeError("replace() requires 2 args");
        std::string from = args[0].toString(), to = args[1].toString();
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos)
        {
            result.replace(pos, from.size(), to);
            pos += to.size();
        }
        return QuantumValue(result);
    }
    if (m == "slice" || m == "substr")
    {
        int start = args.empty() ? 0 : (int)toNum(args[0], "slice");
        int len_ = args.size() > 1 ? (int)toNum(args[1], "slice") : (int)str.size() - start;
        if (start < 0)
            start += (int)str.size();
        return QuantumValue(str.substr(std::max(0, start), len_));
    }
    if (m == "index")
    {
        auto sub = args[0].toString();
        auto p = str.find(sub);
        return QuantumValue(p == std::string::npos ? QuantumValue(-1.0) : QuantumValue((double)p));
    }
    if (m == "repeat")
    {
        int n = (int)toNum(args[0], "repeat");
        std::string res;
        for (int i = 0; i < n; i++)
            res += str;
        return QuantumValue(res);
    }
    if (m == "chars")
    {
        auto arr = std::make_shared<Array>();
        for (char c : str)
            arr->push_back(QuantumValue(std::string(1, c)));
        return QuantumValue(arr);
    }

    // ── JavaScript-style aliases ──────────────────────────────────────────
    if (m == "toLowerCase")
        return callStringMethod(str, "lower", std::move(args));
    if (m == "toUpperCase")
        return callStringMethod(str, "upper", std::move(args));
    if (m == "includes")
        return callStringMethod(str, "contains", std::move(args));
    if (m == "startsWith")
        return callStringMethod(str, "starts_with", std::move(args));
    if (m == "endsWith")
        return callStringMethod(str, "ends_with", std::move(args));
    if (m == "indexOf")
        return callStringMethod(str, "index", std::move(args));
    if (m == "substring" || m == "subString")
        return callStringMethod(str, "slice", std::move(args));
    if (m == "trimStart" || m == "trimEnd")
        return callStringMethod(str, "trim", std::move(args));
    if (m == "toLocaleLowerCase")
        return callStringMethod(str, "lower", std::move(args));
    if (m == "toLocaleUpperCase")
        return callStringMethod(str, "upper", std::move(args));
    if (m == "padStart")
    {
        int targetLen = args.empty() ? 0 : (int)toNum(args[0], "padStart");
        std::string pad = args.size() > 1 ? args[1].toString() : " ";
        std::string result = str;
        while ((int)result.size() < targetLen)
            result = pad + result;
        return QuantumValue(result.substr(result.size() - std::max((int)str.size(), targetLen)));
    }
    if (m == "padEnd")
    {
        int targetLen = args.empty() ? 0 : (int)toNum(args[0], "padEnd");
        std::string pad = args.size() > 1 ? args[1].toString() : " ";
        std::string result = str;
        while ((int)result.size() < targetLen)
            result += pad;
        return QuantumValue(result.substr(0, std::max((int)str.size(), targetLen)));
    }
    if (m == "charAt")
    {
        int idx = args.empty() ? 0 : (int)toNum(args[0], "charAt");
        if (idx < 0 || idx >= (int)str.size())
            return QuantumValue(std::string(""));
        return QuantumValue(std::string(1, str[idx]));
    }
    if (m == "charCodeAt")
    {
        int idx = args.empty() ? 0 : (int)toNum(args[0], "charCodeAt");
        if (idx < 0 || idx >= (int)str.size())
            return QuantumValue(std::numeric_limits<double>::quiet_NaN());
        return QuantumValue((double)(unsigned char)str[idx]);
    }
    if (m == "at")
    {
        int idx = args.empty() ? 0 : (int)toNum(args[0], "at");
        if (idx < 0)
            idx += (int)str.size();
        if (idx < 0 || idx >= (int)str.size())
            return QuantumValue();
        return QuantumValue(std::string(1, str[idx]));
    }

    // translate(table) — apply a char-mapping dict produced by maketrans
    if (m == "translate")
    {
        if (args.empty() || !args[0].isDict())
            throw RuntimeError("translate() requires a dict translation table");
        auto &table = *args[0].asDict();
        std::string result;
        for (char c : str)
        {
            std::string key(1, c);
            auto it = table.find(key);
            if (it != table.end())
                result += it->second.toString();
            else
                result += c;
        }
        return QuantumValue(result);
    }
    if (m == "isdigit" || m == "isnumeric")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (!std::isdigit((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "isalpha")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (!std::isalpha((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "isalnum")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (!std::isalnum((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "isspace")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (!std::isspace((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "isupper")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (std::isalpha((unsigned char)c) && !std::isupper((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "islower")
    {
        if (str.empty())
            return QuantumValue(false);
        for (char c : str)
            if (std::isalpha((unsigned char)c) && !std::islower((unsigned char)c))
                return QuantumValue(false);
        return QuantumValue(true);
    }
    if (m == "strip")
    {
        std::string chars = args.empty() ? " \t\n\r" : args[0].asString();
        std::string r = str;
        auto isChars = [&](char c)
        { return chars.find(c) != std::string::npos; };
        size_t s = 0, e = r.size();
        while (s < e && isChars(r[s]))
            s++;
        while (e > s && isChars(r[e - 1]))
            e--;
        return QuantumValue(r.substr(s, e - s));
    }
    if (m == "lstrip")
    {
        std::string chars = args.empty() ? " \t\n\r" : args[0].asString();
        std::string r = str;
        size_t s = 0;
        while (s < r.size() && chars.find(r[s]) != std::string::npos)
            s++;
        return QuantumValue(r.substr(s));
    }
    if (m == "rstrip")
    {
        std::string chars = args.empty() ? " \t\n\r" : args[0].asString();
        std::string r = str;
        size_t e = r.size();
        while (e > 0 && chars.find(r[e - 1]) != std::string::npos)
            e--;
        return QuantumValue(r.substr(0, e));
    }

    // Regex test() — called on a regex string like /\s/ or /\d/
    if (m == "test")
    {
        if (args.empty())
            return QuantumValue(false);
        std::string target = args[0].toString();
        // str is the regex pattern, possibly wrapped in /.../flags
        std::string pattern = str;
        std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
        if (pattern.size() >= 2 && pattern.front() == '/')
        {
            size_t rpos = pattern.rfind('/');
            if (rpos > 0)
            {
                std::string fstr = pattern.substr(rpos + 1);
                pattern = pattern.substr(1, rpos - 1);
                if (fstr.find('i') != std::string::npos)
                    flags |= std::regex_constants::icase;
            }
        }
        // Convert JS regex shortcuts to ECMAScript equivalents
        // \s \d \w are already ECMAScript compatible
        try
        {
            std::regex re(pattern, flags);
            return QuantumValue(std::regex_search(target, re));
        }
        catch (...)
        {
            // If regex is invalid, fall back to substring match
            return QuantumValue(target.find(pattern) != std::string::npos);
        }
    }
    // match() — like test but returns an array of matches
    if (m == "match")
    {
        if (args.empty())
            return QuantumValue();
        std::string src = args[0].toString();
        // Called as str.match(regex) where str is the string, but also
        // called as regex.match(str) — handle both by checking if str looks like a regex
        std::string text = str, pattern2 = src;
        bool strIsRegex = (str.size() >= 2 && str.front() == '/');
        if (!strIsRegex)
        {
            text = str;
            pattern2 = src;
        }
        else
        {
            text = src;
            pattern2 = str;
        }
        std::regex_constants::syntax_option_type fl2 = std::regex_constants::ECMAScript;
        if (pattern2.size() >= 2 && pattern2.front() == '/')
        {
            size_t rp = pattern2.rfind('/');
            if (rp > 0)
            {
                if (pattern2.substr(rp + 1).find('i') != std::string::npos)
                    fl2 |= std::regex_constants::icase;
                pattern2 = pattern2.substr(1, rp - 1);
            }
        }
        try
        {
            std::regex re2(pattern2, fl2);
            std::smatch sm;
            auto arr2 = std::make_shared<Array>();
            if (std::regex_search(text, sm, re2))
                for (auto &s : sm)
                    arr2->push_back(QuantumValue(s.str()));
            return arr2->empty() ? QuantumValue() : QuantumValue(arr2);
        }
        catch (...)
        {
            return QuantumValue();
        }
    }

    throw TypeError("String has no method '" + m + "'");
}

QuantumValue Interpreter::callDictMethod(std::shared_ptr<Dict> dict, const std::string &m, std::vector<QuantumValue> args)
{
    // Map-style methods
    if (m == "has")
        return QuantumValue(dict->find(args.empty() ? std::string() : args[0].toString()) != dict->end());
    if (m == "get")
    {
        if (args.empty())
            return QuantumValue();
        auto it = dict->find(args[0].toString());
        if (it != dict->end())
            return it->second;
        return args.size() >= 2 ? args[1] : QuantumValue();
    }
    if (m == "set")
    {
        if (args.size() >= 2)
            (*dict)[args[0].toString()] = args[1];
        return QuantumValue(dict);
    }
    if (m == "delete")
    {
        if (!args.empty())
            dict->erase(args[0].toString());
        return QuantumValue(true);
    }
    if (m == "clear")
    {
        dict->clear();
        return QuantumValue();
    }
    if (m == "add")
    {
        if (!args.empty())
        {
            (*dict)[args[0].toString()] = args[0];
        }
        return QuantumValue(dict);
    }
    if (m == "size")
        return QuantumValue((double)dict->size());
    if (m == "values")
    {
        auto a = std::make_shared<Array>();
        for (auto &p : *dict)
            a->push_back(p.second);
        return QuantumValue(a);
    }
    if (m == "keys")
    {
        auto a = std::make_shared<Array>();
        for (auto &p : *dict)
            a->push_back(QuantumValue(p.first));
        return QuantumValue(a);
    }
    if (m == "entries")
    {
        auto a = std::make_shared<Array>();
        for (auto &p : *dict)
        {
            auto pr = std::make_shared<Array>();
            pr->push_back(QuantumValue(p.first));
            pr->push_back(p.second);
            a->push_back(QuantumValue(pr));
        }
        return QuantumValue(a);
    }
    if (m == "forEach")
    {
        if (!args.empty())
            for (auto &p : *dict)
            {
                std::vector<QuantumValue> cb = {p.second, QuantumValue(p.first)};
                if (args[0].isNative())
                    callNative(args[0].asNative(), cb);
                else if (args[0].isFunction())
                    callFunction(args[0].asFunction(), cb);
            }
        return QuantumValue();
    }

    if (m == "has" || m == "contains" || m == "hasOwnProperty")
        return QuantumValue(dict->count(args[0].toString()) > 0);
    // .items() → array of [key, value] pairs (Python/JS compatibility)
    if (m == "items" || m == "entries")
    {
        auto arr = std::make_shared<Array>();
        for (auto &[k, v] : *dict)
        {
            auto pair = std::make_shared<Array>();
            pair->push_back(QuantumValue(k));
            pair->push_back(v);
            arr->push_back(QuantumValue(pair));
        }
        return QuantumValue(arr);
    }
    if (m == "get")
    {
        auto key = args[0].toString();
        auto it = dict->find(key);
        if (it != dict->end())
            return it->second;
        return args.size() > 1 ? args[1] : QuantumValue();
    }
    if (m == "set")
    {
        (*dict)[args[0].toString()] = args[1];
        return QuantumValue();
    }
    if (m == "delete")
    {
        dict->erase(args[0].toString());
        return QuantumValue();
    }
    if (m == "keys")
    {
        auto arr = std::make_shared<Array>();
        for (auto &[k, v] : *dict)
            arr->push_back(QuantumValue(k));
        return QuantumValue(arr);
    }
    if (m == "values")
    {
        auto arr = std::make_shared<Array>();
        for (auto &[k, v] : *dict)
            arr->push_back(v);
        return QuantumValue(arr);
    }
    if (m == "length" || m == "size")
        return QuantumValue((double)dict->size());
    throw TypeError("Dict has no method '" + m + "'");
}