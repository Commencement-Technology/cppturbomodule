#pragma once

#include <vector>

#include "User.h"

namespace sharedlogic {

std::vector<User> getRelativesForProvidedUser(const User& user);

}
