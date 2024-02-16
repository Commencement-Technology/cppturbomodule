#include "Address.h"

#include <utility>

namespace sharedlogic {

Address::Address(std::string street, std::string city, std::string zipcode)
    : street(std::move(street)), city(std::move(city)), zipcode(std::move(zipcode)) {}

std::string Address::to_string() const {
    return this->street + " " + this->city + " " + this->zipcode;
}

}
