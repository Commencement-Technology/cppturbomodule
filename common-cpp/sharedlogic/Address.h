#pragma once

#include <string>

namespace sharedlogic {

class Address {
public:
    Address(std::string street, std::string city, std::string zipcode);
    std::string street;
    std::string city;
    std::string zipcode;

    std::string to_string() const;
};

}

