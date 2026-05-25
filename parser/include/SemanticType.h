#ifndef SEMANTIC_TYPE_H
#define SEMANTIC_TYPE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

enum class BaseType {
    ST_VOID,
    ST_I32,
    ST_BOOL,
    ST_REF,
    ST_MUT_REF,
    ST_ARRAY,
    ST_TUPLE,
    ST_UNIT,
    ST_FUNCTION,
    ST_UNKNOWN
};

class SType;
using STypePtr = shared_ptr<SType>;

class SType {
public:
    BaseType base = BaseType::ST_UNKNOWN;

    // REF / MUT_REF / ARRAY: inner type
    STypePtr inner;

    // ARRAY: size
    int array_size = 0;

    // TUPLE: element types
    vector<STypePtr> tuple_types;

    // FUNCTION
    vector<STypePtr> param_types;
    STypePtr return_type;

    SType() = default;
    explicit SType(BaseType b) : base(b) {}

    static STypePtr makeVoid() { return make_shared<SType>(BaseType::ST_VOID); }
    static STypePtr makeI32() { return make_shared<SType>(BaseType::ST_I32); }
    static STypePtr makeBool() { return make_shared<SType>(BaseType::ST_BOOL); }
    static STypePtr makeUnknown() { return make_shared<SType>(BaseType::ST_UNKNOWN); }
    static STypePtr makeUnit() { return make_shared<SType>(BaseType::ST_UNIT); }

    static STypePtr makeRef(STypePtr inner) {
        auto t = make_shared<SType>(BaseType::ST_REF);
        t->inner = inner;
        return t;
    }

    static STypePtr makeMutRef(STypePtr inner) {
        auto t = make_shared<SType>(BaseType::ST_MUT_REF);
        t->inner = inner;
        return t;
    }

    static STypePtr makeArray(STypePtr elem, int size) {
        auto t = make_shared<SType>(BaseType::ST_ARRAY);
        t->inner = elem;
        t->array_size = size;
        return t;
    }

    static STypePtr makeTuple(vector<STypePtr> types) {
        auto t = make_shared<SType>(BaseType::ST_TUPLE);
        t->tuple_types = move(types);
        return t;
    }

    static STypePtr makeFunction(vector<STypePtr> params, STypePtr ret) {
        auto t = make_shared<SType>(BaseType::ST_FUNCTION);
        t->param_types = move(params);
        t->return_type = ret;
        return t;
    }

    bool isI32() const { return base == BaseType::ST_I32; }
    bool isBool() const { return base == BaseType::ST_BOOL; }
    bool isVoid() const { return base == BaseType::ST_VOID; }
    bool isUnknown() const { return base == BaseType::ST_UNKNOWN; }
    bool isUnit() const { return base == BaseType::ST_UNIT; }
    bool isRef() const { return base == BaseType::ST_REF; }
    bool isMutRef() const { return base == BaseType::ST_MUT_REF; }
    bool isArray() const { return base == BaseType::ST_ARRAY; }
    bool isTuple() const { return base == BaseType::ST_TUPLE; }
    bool isFunction() const { return base == BaseType::ST_FUNCTION; }
    bool isIntegerLike() const { return isI32() || isBool(); }

    string toString() const;
    bool equals(const STypePtr& other) const;
};

bool operator==(const SType& a, const SType& b);
bool operator!=(const SType& a, const SType& b);

#endif
