#include "Sdk.h"

namespace sharedlogic {

std::vector<User> getRelativesForProvidedUser(const User& user) {
    std::vector<User> relatives = {};
    relatives.emplace_back(user.id + 1, "Judy Doe", user.hasChildren, user.address);
    if (user.hasChildren) {
        relatives.emplace_back(user.id + 2, "Frank Doe", false, user.address);
        relatives.emplace_back(user.id + 3, "Marta Doe", false, user.address);
    }
    return relatives;
}

}
