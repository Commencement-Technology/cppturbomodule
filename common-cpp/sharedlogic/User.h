#pragma once

#include <string>

#include "Address.h"

namespace sharedlogic {

class User {
public:
    User(int id, std::string name, bool hasChildren, Address address);
    int id;
    std::string name;
    bool hasChildren;
    Address address;
};

}
