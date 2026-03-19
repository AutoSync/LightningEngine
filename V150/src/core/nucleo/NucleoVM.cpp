#include "../../include/nucleo/NucleoVM.h"
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace Nucleo {

// ─────────────────────────────────────────────────────────────────────────────
// Stack helpers
// ─────────────────────────────────────────────────────────────────────────────

void VM::push(Value v)  { stack.push_back(std::move(v)); }
Value VM::pop()         { Value v = std::move(stack.back()); stack.pop_back(); return v; }
Value& VM::top()        { return stack.back(); }

void VM::err(const std::string& msg)
{
    lastError = msg;
    throw std::runtime_error(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lookup helpers
// ─────────────────────────────────────────────────────────────────────────────

const CompiledClass* VM::findClass(const std::string& n) const
{
    if (!program) return nullptr;
    return program->FindClass(n);
}

const CompiledMethod* VM::findMethod(const CompiledClass* cls,
                                      const std::string& name) const
{
    if (!cls) return nullptr;
    const CompiledMethod* m = cls->FindMethod(name);
    if (m) return m;
    // Walk base class chain
    if (!cls->baseName.empty()) {
        const CompiledClass* base = findClass(cls->baseName);
        if (base) return findMethod(base, name);
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Instantiate
// ─────────────────────────────────────────────────────────────────────────────

ObjectRef VM::Instantiate(const std::string& className)
{
    const CompiledClass* cls = findClass(className);
    auto obj = std::make_shared<NucleoObject>();
    obj->className = className;
    if (cls) {
        for (size_t i = 0; i < cls->fieldNames.size(); ++i) {
            obj->fields[cls->fieldNames[i]] =
                i < cls->fieldDefaults.size() ? cls->fieldDefaults[i] : Value::Null();
        }
    }
    return obj;
}

// ─────────────────────────────────────────────────────────────────────────────
// Call
// ─────────────────────────────────────────────────────────────────────────────

VMResult VM::Call(ObjectRef obj, const std::string& method,
                  const std::vector<Value>& args)
{
    lastError = "";
    VMResult res;

    if (!program) {
        res.ok    = false;
        res.error = "No program loaded";
        return res;
    }

    try {
        const CompiledClass*  cls = obj ? findClass(obj->className) : nullptr;
        const CompiledMethod* m   = cls ? findMethod(cls, method)   : nullptr;
        if (!m) {
            // Method not found — not an error for lifecycle methods
            res.value = Value::Null();
            return res;
        }

        size_t stackBase = stack.size();
        // Push "this" slot placeholder (0xFFFF convention)
        push(Value::Object(obj));
        // Push params
        for (const auto& a : args) push(a);
        // Reserve remaining local slots
        int extras = m->localCount - (int)m->paramNames.size();
        for (int i = 0; i < extras; ++i) push(Value::Null());

        frames.push_back({ &m->chunk, 0, stackBase, obj });
        res.value = run().value;
        res.ok    = lastError.empty();
        res.error = lastError;
    } catch (const std::exception& ex) {
        res.ok    = false;
        res.error = ex.what();
    }

    return res;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main execution loop
// ─────────────────────────────────────────────────────────────────────────────

bool VM::callMethod(ObjectRef obj, const std::string& name, int argc, bool pushNull)
{
    const CompiledClass*  cls = obj ? findClass(obj->className) : nullptr;
    const CompiledMethod* m   = cls ? findMethod(cls, name) : nullptr;

    if (!m) {
        // Check native registry by name too
        if (program) {
            for (size_t i = 0; i < program->natives.size(); ++i) {
                if (program->natives[i].name == name) {
                    return callNative((int)i, argc);
                }
            }
        }
        // Pop args + obj
        for (int i = 0; i <= argc; ++i) if (!stack.empty()) stack.pop_back();
        push(Value::Null());
        return true;
    }

    // Collect args (they're already on stack above obj)
    size_t stackBase = stack.size() - argc - 1; // -1 for obj
    int extras = m->localCount - (int)m->paramNames.size();
    for (int i = 0; i < extras; ++i) push(Value::Null());
    frames.push_back({ &m->chunk, 0, stackBase, obj });
    return true;
}

bool VM::callNative(int nativeIdx, int argc)
{
    if (!program || nativeIdx < 0 || nativeIdx >= (int)program->natives.size())
        return false;

    std::vector<Value> args;
    args.resize(argc);
    for (int i = argc - 1; i >= 0; --i)
        args[i] = pop();

    Value result = program->natives[nativeIdx].fn(args);
    push(std::move(result));
    return true;
}

VMResult VM::run()
{
    VMResult res;

    while (!frames.empty()) {
        CallFrame& f = frames.back();
        const Chunk& ch = *f.chunk;

        if (f.ip >= ch.code.size()) {
            frames.pop_back();
            if (!frames.empty()) push(Value::Null()); // implicit void return
            continue;
        }

        Op op = (Op)ch.code[f.ip++];

        auto readU16 = [&]() -> uint16_t {
            uint16_t v = (uint16_t)ch.code[f.ip] | ((uint16_t)ch.code[f.ip+1] << 8);
            f.ip += 2;
            return v;
        };
        auto readS16 = [&]() -> int16_t { return (int16_t)readU16(); };

        switch (op) {

        // ── Push constants ────────────────────────────────────────────────
        case Op::PUSH_CONST:  { uint16_t ci = readU16(); push(ch.constants[ci]); break; }
        case Op::PUSH_NULL:   push(Value::Null());        break;
        case Op::PUSH_TRUE:   push(Value::Bool(true));    break;
        case Op::PUSH_FALSE:  push(Value::Bool(false));   break;

        // ── Stack ─────────────────────────────────────────────────────────
        case Op::POP:  pop();  break;
        case Op::DUP:  push(stack.back()); break;

        // ── Locals ───────────────────────────────────────────────────────
        case Op::LOAD_LOCAL: {
            uint16_t slot = readU16();
            if (slot == 0xFFFF) push(Value::Object(f.self));  // this
            else push(stack[f.stackBase + slot]);
            break;
        }
        case Op::STORE_LOCAL: {
            uint16_t slot = readU16();
            stack[f.stackBase + slot] = pop();
            break;
        }

        // ── This fields ───────────────────────────────────────────────────
        case Op::LOAD_THIS_FIELD: {
            uint16_t ni = readU16();
            if (f.self) push(f.self->Field(ch.names[ni]));
            else push(Value::Null());
            break;
        }
        case Op::STORE_THIS_FIELD: {
            uint16_t ni = readU16();
            if (f.self) f.self->Field(ch.names[ni]) = pop();
            else pop();
            break;
        }

        // ── Field on object ───────────────────────────────────────────────
        case Op::LOAD_FIELD: {
            uint16_t ni = readU16();
            Value obj   = pop();
            if (obj.isObject() && obj.obj)
                push(obj.obj->Field(ch.names[ni]));
            else
                push(Value::Null());
            break;
        }
        case Op::STORE_FIELD: {
            uint16_t ni  = readU16();
            Value    val = pop();
            Value    obj = pop();
            if (obj.isObject() && obj.obj)
                obj.obj->Field(ch.names[ni]) = std::move(val);
            break;
        }

        // ── Globals ───────────────────────────────────────────────────────
        case Op::LOAD_GLOBAL: {
            uint16_t ni = readU16();
            auto it = globals.find(ch.names[ni]);
            push(it != globals.end() ? it->second : Value::Null());
            break;
        }
        case Op::STORE_GLOBAL: {
            uint16_t ni = readU16();
            globals[ch.names[ni]] = pop();
            break;
        }

        // ── Arithmetic ────────────────────────────────────────────────────
        case Op::ADD: {
            Value b = pop(), a = pop();
            if (a.isString() || b.isString())
                push(Value::String(a.toString() + b.toString()));
            else if (a.isFloat() || b.isFloat())
                push(Value::Float(a.toFloat() + b.toFloat()));
            else
                push(Value::Int(a.toInt() + b.toInt()));
            break;
        }
        case Op::SUB: { Value b=pop(), a=pop();
            push(a.isFloat()||b.isFloat() ? Value::Float(a.toFloat()-b.toFloat()) : Value::Int(a.toInt()-b.toInt())); break; }
        case Op::MUL: { Value b=pop(), a=pop();
            push(a.isFloat()||b.isFloat() ? Value::Float(a.toFloat()*b.toFloat()) : Value::Int(a.toInt()*b.toInt())); break; }
        case Op::DIV: { Value b=pop(), a=pop();
            if (b.toFloat()==0.f) err("Division by zero");
            push(a.isFloat()||b.isFloat() ? Value::Float(a.toFloat()/b.toFloat()) : Value::Int(a.toInt()/b.toInt())); break; }
        case Op::MOD: { Value b=pop(), a=pop();
            if (b.toInt()==0) err("Modulo by zero");
            push(Value::Int(a.toInt() % b.toInt())); break; }
        case Op::NEGATE: { Value a=pop();
            push(a.isFloat() ? Value::Float(-a.toFloat()) : Value::Int(-a.toInt())); break; }

        // ── Comparison ────────────────────────────────────────────────────
        case Op::EQ:  { Value b=pop(),a=pop(); push(Value::Bool(a==b));  break; }
        case Op::NEQ: { Value b=pop(),a=pop(); push(Value::Bool(a!=b));  break; }
        case Op::LT:  { Value b=pop(),a=pop(); push(Value::Bool(a.toFloat()<b.toFloat()));  break; }
        case Op::LTE: { Value b=pop(),a=pop(); push(Value::Bool(a.toFloat()<=b.toFloat())); break; }
        case Op::GT:  { Value b=pop(),a=pop(); push(Value::Bool(a.toFloat()>b.toFloat()));  break; }
        case Op::GTE: { Value b=pop(),a=pop(); push(Value::Bool(a.toFloat()>=b.toFloat())); break; }

        // ── Logical ───────────────────────────────────────────────────────
        case Op::LAND: { Value b=pop(),a=pop(); push(Value::Bool(a.toBool()&&b.toBool())); break; }
        case Op::LOR:  { Value b=pop(),a=pop(); push(Value::Bool(a.toBool()||b.toBool())); break; }
        case Op::LNOT: { push(Value::Bool(!pop().toBool())); break; }
        case Op::CONCAT: { Value b=pop(),a=pop(); push(Value::String(a.toString()+b.toString())); break; }

        // ── Jumps ─────────────────────────────────────────────────────────
        case Op::JMP:       { int16_t off = readS16(); f.ip += off; break; }
        case Op::JMP_FALSE: { int16_t off = readS16(); if (!pop().toBool()) f.ip += off; break; }
        case Op::JMP_TRUE:  { int16_t off = readS16(); if ( pop().toBool()) f.ip += off; break; }

        // ── Method call ───────────────────────────────────────────────────
        case Op::CALL_METHOD: {
            uint16_t argc = readU16();
            uint16_t ni   = readU16();
            // Stack: [obj, arg0, arg1, ..., argN-1]
            // obj is at stack[size - argc - 1]
            Value& objVal = stack[stack.size() - argc - 1];
            ObjectRef objRef = objVal.isObject() ? objVal.obj : f.self;
            const std::string& mname = ch.names[ni];

            // Check natives first
            bool handled = false;
            if (program) {
                for (size_t i = 0; i < program->natives.size(); ++i) {
                    if (program->natives[i].name == mname) {
                        std::vector<Value> nargs;
                        nargs.resize(argc);
                        for (int j = argc-1; j >= 0; --j) nargs[j] = pop();
                        pop(); // obj
                        push(program->natives[i].fn(nargs));
                        handled = true;
                        break;
                    }
                }
            }
            if (!handled) callMethod(objRef, mname, (int)argc);
            break;
        }

        // ── Native call ───────────────────────────────────────────────────
        case Op::CALL_NATIVE: {
            uint16_t ni   = readU16();
            uint16_t argc = readU16();
            callNative((int)ni, (int)argc);
            break;
        }

        // ── Return ────────────────────────────────────────────────────────
        case Op::RETURN: {
            Value retval = pop();
            // Unwind locals
            while (stack.size() > f.stackBase) pop();
            frames.pop_back();
            push(std::move(retval));
            break;
        }
        case Op::RETURN_VOID: {
            while (stack.size() > f.stackBase) pop();
            frames.pop_back();
            push(Value::Null());
            break;
        }

        // ── Object creation ───────────────────────────────────────────────
        case Op::NEW_OBJ: {
            uint16_t ni = readU16();
            ObjectRef obj = Instantiate(ch.names[ni]);
            push(Value::Object(std::move(obj)));
            break;
        }

        case Op::HALT:
            frames.clear();
            break;

        default: err("Unknown opcode"); break;
        }
    }

    res.value = stack.empty() ? Value::Null() : pop();
    return res;
}

} // namespace Nucleo
