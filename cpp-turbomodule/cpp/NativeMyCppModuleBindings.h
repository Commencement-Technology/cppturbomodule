#pragma once

#if __has_include(<React-Codegen/MyCppTurboPackageJSI.h>)
#include <React-Codegen/MyCppTurboPackageJSI.h>
#elif __has_include("MyCppTurboPackageJSI.h")
#include "MyCppTurboPackageJSI.h"
#endif

#include <optional>
#include <string>

#if __has_include(<CommonCppPackage/Module.h>)
#include <CommonCppPackage/Module.h>
#elif __has_include("Module.h")
#include "Module.h"
#endif

namespace facebook::react {

#pragma mark Custom C++ structs made with struct generators

using Address = MyCppModuleBaseAddress<std::string, std::string, std::string>;

template<>
struct Bridging<Address>
    : MyCppModuleBaseAddressBridging<std::string, std::string, std::string> {};

using User = MyCppModuleBaseUser<int, std::string, std::optional<bool>, Address>;

template<>
struct Bridging<User>
    : MyCppModuleBaseUserBridging<int, std::string, std::optional < bool>, Address> {};

#pragma mark Manually typed custom C++ structs

template<>
struct Bridging<sharedlogic::Address> {
    static sharedlogic::Address fromJs(
            jsi::Runtime &rt,
            const jsi::Object &value,
            const std::shared_ptr <CallInvoker> &jsInvoker) {
        return sharedlogic::Address{
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "street"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "city"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "zipcode"), jsInvoker),
        };
    }

    static jsi::Object toJs(jsi::Runtime &rt, const sharedlogic::Address &value) {
        auto result = facebook::jsi::Object(rt);
        result.setProperty(rt, "street", bridging::toJs(rt, value.street));
        result.setProperty(rt, "city", bridging::toJs(rt, value.city));
        result.setProperty(rt, "zipcode", bridging::toJs(rt, value.zipcode));
        return result;
    }
};

template<>
struct Bridging<sharedlogic::User> {
    static sharedlogic::User fromJs(
            jsi::Runtime &rt,
            const jsi::Object &value,
            const std::shared_ptr <CallInvoker> &jsInvoker) {
        return sharedlogic::User{
                bridging::fromJs<int>(rt, value.getProperty(rt, "id"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "name"), jsInvoker),
                bridging::fromJs<bool>(
                    rt,
                    value.hasProperty(rt, "hasChildren") ? value.getProperty(rt, "hasChildren") : false,
                    jsInvoker
                ),
                bridging::fromJs<sharedlogic::Address>(
                    rt,
                    value.getProperty(rt, "address"),
                    jsInvoker
                ),
        };
    }

    static jsi::Object toJs(jsi::Runtime &rt, const sharedlogic::User &value) {
        auto result = facebook::jsi::Object(rt);
        result.setProperty(rt, "id", bridging::toJs(rt, value.id));
        result.setProperty(rt, "name", bridging::toJs(rt, value.name));
        result.setProperty(rt, "hasChildren", bridging::toJs(rt, value.hasChildren));
        result.setProperty(rt, "address", bridging::toJs(rt, value.address));
        return result;
    }
};

}
