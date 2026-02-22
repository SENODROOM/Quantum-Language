#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <stdexcept>

class Environment;
struct ASTNode;
class Interpreter;

// ─── Value Types ──────────────────────────────────────────────────────────────

struct QuantumNil {};

struct QuantumFunction {
    std::string name;
    std::vector<std::string> params;
    ASTNode* body;                          // non-owning ptr
    std::shared_ptr<Environment> closure;
};

struct QuantumClass;
struct QuantumInstance;

using QuantumNativeFunc = std::function<struct QuantumValue(std::vector<struct QuantumValue>)>;

struct QuantumNative {
    std::string name;
    QuantumNativeFunc fn;
};

struct QuantumValue;

using Array   = std::vector<QuantumValue>;
using Dict    = std::unordered_map<std::string, QuantumValue>;

struct QuantumValue {
    using Data = std::variant<
        QuantumNil,
        bool,
        double,
        std::string,
        std::shared_ptr<Array>,
        std::shared_ptr<Dict>,
        std::shared_ptr<QuantumFunction>,
        std::shared_ptr<QuantumNative>,
        std::shared_ptr<QuantumInstance>
    >;

    Data data;

    // Constructors
    QuantumValue() : data(QuantumNil{}) {}
    explicit QuantumValue(bool b)  : data(b) {}
    explicit QuantumValue(double d): data(d) {}
    explicit QuantumValue(const std::string& s) : data(s) {}
    explicit QuantumValue(std::string&& s)       : data(std::move(s)) {}
    explicit QuantumValue(std::shared_ptr<Array> a) : data(std::move(a)) {}
    explicit QuantumValue(std::shared_ptr<Dict>  d) : data(std::move(d)) {}
    explicit QuantumValue(std::shared_ptr<QuantumFunction> f) : data(std::move(f)) {}
    explicit QuantumValue(std::shared_ptr<QuantumNative>   n) : data(std::move(n)) {}
    explicit QuantumValue(std::shared_ptr<QuantumInstance> i) : data(std::move(i)) {}

    // Type checks
    bool isNil()      const { return std::holds_alternative<QuantumNil>(data); }
    bool isBool()     const { return std::holds_alternative<bool>(data); }
    bool isNumber()   const { return std::holds_alternative<double>(data); }
    bool isString()   const { return std::holds_alternative<std::string>(data); }
    bool isArray()    const { return std::holds_alternative<std::shared_ptr<Array>>(data); }
    bool isDict()     const { return std::holds_alternative<std::shared_ptr<Dict>>(data); }
    bool isFunction() const { return std::holds_alternative<std::shared_ptr<QuantumFunction>>(data)
                                  || std::holds_alternative<std::shared_ptr<QuantumNative>>(data); }
    bool isInstance() const { return std::holds_alternative<std::shared_ptr<QuantumInstance>>(data); }

    // Accessors
    bool        asBool()   const { return std::get<bool>(data); }
    double      asNumber() const { return std::get<double>(data); }
    std::string asString() const { return std::get<std::string>(data); }
    std::shared_ptr<Array>    asArray()    const { return std::get<std::shared_ptr<Array>>(data); }
    std::shared_ptr<Dict>     asDict()     const { return std::get<std::shared_ptr<Dict>>(data); }
    std::shared_ptr<QuantumFunction> asFunction() const { return std::get<std::shared_ptr<QuantumFunction>>(data); }
    std::shared_ptr<QuantumNative>   asNative()   const { return std::get<std::shared_ptr<QuantumNative>>(data); }
    std::shared_ptr<QuantumInstance> asInstance() const { return std::get<std::shared_ptr<QuantumInstance>>(data); }

    bool isTruthy() const;
    std::string toString() const;
    std::string typeName() const;
};

// ─── Environment ──────────────────────────────────────────────────────────────

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr);

    void define(const std::string& name, QuantumValue val, bool isConst = false);
    QuantumValue get(const std::string& name) const;
    void set(const std::string& name, QuantumValue val);
    bool has(const std::string& name) const;

    std::shared_ptr<Environment> parent;

private:
    std::unordered_map<std::string, QuantumValue> vars;
    std::unordered_map<std::string, bool> constants;
};

// ─── Class & Instance ────────────────────────────────────────────────────────

struct QuantumClass {
    std::string name;
    std::shared_ptr<QuantumClass> base;
    std::unordered_map<std::string, std::shared_ptr<QuantumFunction>> methods;
};

struct QuantumInstance {
    std::shared_ptr<QuantumClass> klass;
    std::unordered_map<std::string, QuantumValue> fields;
    std::shared_ptr<Environment> env;

    QuantumValue getField(const std::string& name) const;
    void setField(const std::string& name, QuantumValue val);
};

// ─── Control Flow Signals ────────────────────────────────────────────────────

struct ReturnSignal {
    QuantumValue value;
    explicit ReturnSignal(QuantumValue v) : value(std::move(v)) {}
};

struct BreakSignal  {};
struct ContinueSignal {};
