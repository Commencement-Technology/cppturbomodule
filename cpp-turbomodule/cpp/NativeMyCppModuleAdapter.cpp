#include "NativeMyCppModuleAdapter.h"

namespace facebook::react {

#pragma mark Class implementation

NativeMyCppModuleAdapter::NativeMyCppModuleAdapter(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeMyCppModuleCxxSpec(std::move(jsInvoker)), instance() {}

std::vector<User> NativeMyCppModuleAdapter::getUsers(jsi::Runtime& rt, User user) {
    auto sharedUserArg = sharedlogic::User{
        user.id,
        user.name,
        user.hasChildren.has_value() && user.hasChildren.value(),
        sharedlogic::Address{
            user.address.street,
            user.address.city,
            user.address.zipcode
        }
    };
    auto sharedUsers = instance.getUsers(sharedUserArg);
    std::vector<User> array = {};
    for (auto sharedUser : sharedUsers) {
        array.push_back(User{sharedUser.id, sharedUser.name, sharedUser.hasChildren, Address{sharedUser.address.street, sharedUser.address.city, sharedUser.address.zipcode}});
    }
    return array;
}

AsyncPromise<std::vector<sharedlogic::User>> NativeMyCppModuleAdapter::getUsersAsync(jsi::Runtime& rt, const sharedlogic::User& user) {
    auto promise = AsyncPromise<std::vector<sharedlogic::User>>(rt, jsInvoker_);
    auto sharedUsers = instance.getUsers(user);
    promise.resolve(sharedUsers);
    return promise;
}

}
