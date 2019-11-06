// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from math.djinni

#include "Integer.hpp"
#include <memory>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@class CPInteger;

namespace djinni_generated {

class Integer
{
public:
    using CppType = std::shared_ptr<::cryptoplus::Integer>;
    using CppOptType = std::shared_ptr<::cryptoplus::Integer>;
    using ObjcType = CPInteger*;

    using Boxed = Integer;

    static CppType toCpp(ObjcType objc);
    static ObjcType fromCppOpt(const CppOptType& cpp);
    static ObjcType fromCpp(const CppType& cpp) { return fromCppOpt(cpp); }

private:
    class ObjcProxy;
};

}  // namespace djinni_generated
