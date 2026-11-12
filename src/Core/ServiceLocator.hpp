#ifndef CORE_SERVICELOCATOR_HPP
#define CORE_SERVICELOCATOR_HPP

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>

namespace Core {

class IService {
public:
    virtual ~IService() = default;
};

class ServiceLocator {
private:
    std::unordered_map<std::type_index, std::shared_ptr<IService>> services;

    ServiceLocator() = default;

public:
    static ServiceLocator& instance() {
        static ServiceLocator locator;
        return locator;
    }

    template <typename ServiceType>
    void provide(std::shared_ptr<ServiceType> service) {
        services[std::type_index(typeid(ServiceType))] = service;
    }

    template <typename ServiceType>
    std::shared_ptr<ServiceType> get() {
        auto it = services.find(std::type_index(typeid(ServiceType)));
        if (it != services.end()) {
            return std::static_pointer_cast<ServiceType>(it->second);
        }
        return nullptr;
    }

    void clear() {
        services.clear();
    }
};

} // namespace Core

#endif // CORE_SERVICELOCATOR_HPP
