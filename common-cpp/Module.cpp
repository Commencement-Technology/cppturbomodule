#include "Module.h"

namespace common {

Module::Module() {}

std::vector<sharedlogic::User> Module::getUsers(const sharedlogic::User& user) {
    return sharedlogic::getRelativesForProvidedUser(user);
}

}
