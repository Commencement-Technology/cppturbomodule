#pragma once

#include <vector>

#if __has_include("sharedlogic/Sdk.h")
#include "sharedlogic/Sdk.h"
#elif __has_include("Sdk.h")
#include "Sdk.h"
#endif
#if __has_include("sharedlogic/User.h")
#include "sharedlogic/User.h"
#elif __has_include("User.h")
#include "User.h"
#endif

namespace common {

class Module {
public:
    Module();
    std::vector<sharedlogic::User> getUsers(const sharedlogic::User& user);
};

}
