#include "../include/Value.h"
#include "../include/Error.h"
#include <sstream>
#include <cmath>
#include <iomanip>

// ─── QuantumValue ─────────────────────────────────────────────────────────────

bool QuantumValue::isTruthy() const {
    return std::visit([](const auto& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, QuantumNil>)    return false;
        if constexpr (std::is_same_v<T, bool>)          return v;
        if constexpr (std::is_same_v<T, double>)        return v != 0.0;
        if constexpr (std::is_same_v<T, std::string>)   return !v.empty();
        if constexpr (std::is_same_v<T, std::shared_ptr<Array>>) return !v->empty();
        return true;
    }, data);
}

std::string QuantumValue::toString() const {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, QuantumNil>)  return "nil";
        if constexpr (std::is_same_v<T, bool>)        return v ? "true" : "false";
        if constexpr (std::is_same_v<T, double>) {
            if (std::floor(v) == v && std::abs(v) < 1e15)
                return std::to_string((long long)v);
            std::ostringstream oss;
            oss << std::setprecision(10) << v;
            return oss.str();
        }
        if constexpr (std::is_same_v<T, std::string>) return v;
        if constexpr (std::is_same_v<T, std::shared_ptr<Array>>) {
            std::string s = "[";
            for (size_t i = 0; i < v->size(); i++) {
                if (i) s += ", ";
                if ((*v)[i].isString()) s += "\"" + (*v)[i].toString() + "\"";
                else s += (*v)[i].toString();
            }
            return s + "]";
        }
        if constexpr (std::is_same_v<T, std::shared_ptr<Dict>>) {
            std::string s = "{";
            bool first = true;
            for (auto& [k, val] : *v) {
                if (!first) s += ", ";
                s += "\"" + k + "\": ";
                if (val.isString()) s += "\"" + val.toString() + "\"";
                else s += val.toString();
                first = false;
            }
            return s + "}";
        }
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumFunction>>) return "<fn:" + v->name + ">";
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumNative>>)   return "<native:" + v->name + ">";
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumInstance>>) {
            // Call __str__ if defined
            auto k = v->klass.get();
            while (k) {
                auto mit = k->methods.find("__str__");
                if (mit != k->methods.end()) {
                    // __str__ found — we can't call it here without an interpreter,
                    // so fall back to default representation
                    break;
                }
                k = k->base.get();
            }
            return "<instance:" + v->klass->name + ">";
        }
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumClass>>)    return "<class:" + v->name + ">";
        return "?";
    }, data);
}

std::string QuantumValue::typeName() const {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, QuantumNil>)   return "nil";
        if constexpr (std::is_same_v<T, bool>)         return "bool";
        if constexpr (std::is_same_v<T, double>)       return "number";
        if constexpr (std::is_same_v<T, std::string>)  return "string";
        if constexpr (std::is_same_v<T, std::shared_ptr<Array>>)           return "array";
        if constexpr (std::is_same_v<T, std::shared_ptr<Dict>>)            return "dict";
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumFunction>>) return "function";
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumNative>>)   return "native";
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumInstance>>) return v->klass->name;
        if constexpr (std::is_same_v<T, std::shared_ptr<QuantumClass>>)    return "class";
        return "unknown";
    }, data);
}

// ─── Environment ─────────────────────────────────────────────────────────────

Environment::Environment(std::shared_ptr<Environment> p) : parent(std::move(p)) {}

void Environment::define(const std::string& name, QuantumValue val, bool isConst) {
    vars[name] = std::move(val);
    if (isConst) constants[name] = true;
}

QuantumValue Environment::get(const std::string& name) const {
    auto it = vars.find(name);
    if (it != vars.end()) return it->second;
    if (parent) return parent->get(name);
    throw NameError("Undefined variable: '" + name + "'");
}

void Environment::set(const std::string& name, QuantumValue val) {
    auto it = vars.find(name);
    if (it != vars.end()) {
        if (constants.count(name))
            throw RuntimeError("Cannot reassign constant '" + name + "'");
        it->second = std::move(val);
        return;
    }
    if (parent) { parent->set(name, std::move(val)); return; }
    throw NameError("Undefined variable: '" + name + "'");
}

bool Environment::has(const std::string& name) const {
    if (vars.count(name)) return true;
    if (parent) return parent->has(name);
    return false;
}

// ─── QuantumInstance ─────────────────────────────────────────────────────────

QuantumValue QuantumInstance::getField(const std::string& name) const {
    auto it = fields.find(name);
    if (it != fields.end()) return it->second;
    // Check methods
    auto k = klass.get();
    while (k) {
        auto mit = k->methods.find(name);
        if (mit != k->methods.end())
            return QuantumValue(mit->second);
        k = k->base.get();
    }
    throw NameError("No field/method '" + name + "' on instance of " + klass->name);
}

void QuantumInstance::setField(const std::string& name, QuantumValue val) {
    fields[name] = std::move(val);
}
