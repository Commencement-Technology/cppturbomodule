#pragma once

#if __has_include(<React-Codegen/MyCppTurboPackageJSI.h>)
#include <React-Codegen/MyCppTurboPackageJSI.h>
#elif __has_include("MyCppTurboPackageJSI.h")
#include "MyCppTurboPackageJSI.h"
#endif

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#if __has_include(<CommonCppPackage/Module.h>)
#include <CommonCppPackage/Module.h>
#elif __has_include("Module.h")
#include "Module.h"
#endif

#include "NativeMyCppModuleBindings.h"

namespace facebook::react {

class NativeMyCppModuleAdapter : public NativeMyCppModuleCxxSpec<NativeMyCppModuleAdapter> {
public:
    NativeMyCppModuleAdapter(std::shared_ptr<CallInvoker> jsInvoker);

    std::vector<User> getUsers(jsi::Runtime& rt, User user);
    AsyncPromise<std::vector<sharedlogic::User>> getUsersAsync(jsi::Runtime& rt, const sharedlogic::User& user);
private:
    common::Module instance;
};

}
