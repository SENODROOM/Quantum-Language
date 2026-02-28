#pragma once
#include "AST.h"
#include "Value.h"
#include <memory>

class Interpreter
{
public:
    Interpreter();
    void execute(ASTNode &node);
    QuantumValue evaluate(ASTNode &node);

    std::shared_ptr<Environment> globals;

private:
    std::shared_ptr<Environment> env;

    void registerNatives();

    // Statement executors
    void execBlock(BlockStmt &s, std::shared_ptr<Environment> scope = nullptr);
    void execVarDecl(VarDecl &s);
    void execFunctionDecl(FunctionDecl &s);
    void execClassDecl(ClassDecl &s);
    void execIf(IfStmt &s);
    void execWhile(WhileStmt &s);
    void execFor(ForStmt &s);
    void execReturn(ReturnStmt &s);
    void execPrint(PrintStmt &s);
    void execInput(InputStmt &s);
    void execImport(ImportStmt &s);
    void execExprStmt(ExprStmt &s);

    // Expression evaluators
    QuantumValue evalBinary(BinaryExpr &e);
    QuantumValue evalUnary(UnaryExpr &e);
    QuantumValue evalAssign(AssignExpr &e);
    QuantumValue evalCall(CallExpr &e);
    QuantumValue evalIndex(IndexExpr &e);
    QuantumValue evalMember(MemberExpr &e);
    QuantumValue evalArray(ArrayLiteral &e);
    QuantumValue evalDict(DictLiteral &e);
    QuantumValue evalLambda(LambdaExpr &e);
    QuantumValue evalIdentifier(Identifier &e);

    QuantumValue callFunction(std::shared_ptr<QuantumFunction> fn, std::vector<QuantumValue> args);
    QuantumValue callNative(std::shared_ptr<QuantumNative> fn, std::vector<QuantumValue> args);

    // Built-in method dispatch
    QuantumValue callMethod(QuantumValue &obj, const std::string &method, std::vector<QuantumValue> args);
    QuantumValue callArrayMethod(std::shared_ptr<Array> arr, const std::string &method, std::vector<QuantumValue> args);
    QuantumValue callStringMethod(const std::string &str, const std::string &method, std::vector<QuantumValue> args);
    QuantumValue callDictMethod(std::shared_ptr<Dict> dict, const std::string &method, std::vector<QuantumValue> args);

    void setLValue(ASTNode &target, QuantumValue val, const std::string &op);
};