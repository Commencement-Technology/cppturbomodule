#include "User.h"

#include <utility>

namespace sharedlogic {

User::User(int id, std::string name, bool hasChildren, Address address)
    : id(id), name(std::move(name)), hasChildren(hasChildren), address(std::move(address)) {}

}
