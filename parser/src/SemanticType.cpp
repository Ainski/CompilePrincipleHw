#include "../include/SemanticType.h"

string SType::toString() const {
    switch (base) {
        case BaseType::ST_VOID: return "void";
        case BaseType::ST_I32: return "i32";
        case BaseType::ST_BOOL: return "bool";
        case BaseType::ST_UNKNOWN: return "unknown";
        case BaseType::ST_UNIT: return "()";
        case BaseType::ST_REF:
            return "&" + (inner ? inner->toString() : "?");
        case BaseType::ST_MUT_REF:
            return "&mut " + (inner ? inner->toString() : "?");
        case BaseType::ST_ARRAY:
            return "[" + (inner ? inner->toString() : "?") + "; " +
                   to_string(array_size) + "]";
        case BaseType::ST_TUPLE: {
            string s = "(";
            for (size_t i = 0; i < tuple_types.size(); i++) {
                if (i > 0) s += ", ";
                s += tuple_types[i] ? tuple_types[i]->toString() : "?";
            }
            return s + ")";
        }
        case BaseType::ST_FUNCTION: {
            string s = "fn(";
            for (size_t i = 0; i < param_types.size(); i++) {
                if (i > 0) s += ", ";
                s += param_types[i] ? param_types[i]->toString() : "?";
            }
            s += ")";
            if (return_type && !return_type->isVoid())
                s += " -> " + return_type->toString();
            return s;
        }
    }
    return "?";
}

bool SType::equals(const STypePtr& other) const {
    if (!other) return false;
    if (base != other->base) return false;
    switch (base) {
        case BaseType::ST_VOID: case BaseType::ST_I32: case BaseType::ST_BOOL:
        case BaseType::ST_UNKNOWN: case BaseType::ST_UNIT:
            return true;
        case BaseType::ST_REF: case BaseType::ST_MUT_REF:
            return (!inner && !other->inner) ||
                   (inner && other->inner && inner->equals(other->inner));
        case BaseType::ST_ARRAY:
            return array_size == other->array_size &&
                   ((!inner && !other->inner) ||
                    (inner && other->inner && inner->equals(other->inner)));
        case BaseType::ST_TUPLE:
            if (tuple_types.size() != other->tuple_types.size()) return false;
            for (size_t i = 0; i < tuple_types.size(); i++) {
                if (!tuple_types[i] && !other->tuple_types[i]) continue;
                if (!tuple_types[i] || !other->tuple_types[i]) return false;
                if (!tuple_types[i]->equals(other->tuple_types[i])) return false;
            }
            return true;
        case BaseType::ST_FUNCTION:
            if (param_types.size() != other->param_types.size()) return false;
            for (size_t i = 0; i < param_types.size(); i++) {
                if (!param_types[i]->equals(other->param_types[i])) return false;
            }
            return (!return_type && !other->return_type) ||
                   (return_type && other->return_type &&
                    return_type->equals(other->return_type));
    }
    return false;
}

bool operator==(const SType& a, const SType& b) { return a.equals(make_shared<SType>(b)); }
bool operator!=(const SType& a, const SType& b) { return !(a == b); }
