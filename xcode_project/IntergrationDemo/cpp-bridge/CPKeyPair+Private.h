// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from ec.djinni

#include "KeyPair.hpp"
#include <memory>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

@class CPKeyPair;

namespace djinni_generated {

class KeyPair
{
public:
    using CppType = std::shared_ptr<::cryptoplus::KeyPair>;
    using CppOptType = std::shared_ptr<::cryptoplus::KeyPair>;
    using ObjcType = CPKeyPair*;

    using Boxed = KeyPair;

    static CppType toCpp(ObjcType objc);
    static ObjcType fromCppOpt(const CppOptType& cpp);
    static ObjcType fromCpp(const CppType& cpp) { return fromCppOpt(cpp); }

private:
    class ObjcProxy;
};

}  // namespace djinni_generated
