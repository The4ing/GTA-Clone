#include "VehiclePool.h"

VehiclePool::VehiclePool(size_t initialSize) : maxSize(initialSize) {
    pool.reserve(initialSize);
    for (size_t i = 0; i < initialSize; ++i) {
        auto vehicle = std::make_unique<Vehicle>();
        vehicle->setActive(false);
        pool.push_back(std::move(vehicle));
    }
}

Vehicle* VehiclePool::getVehicle() {
    for (auto& vptr : pool) {
        if (!vptr->isActive()) {
            return vptr.get();
        }
    }
    return nullptr;
}

void VehiclePool::returnVehicle(Vehicle* vehicle) {
    if (vehicle) {
        vehicle->setActive(false);
        vehicle->setDestroyed(false);
    }
}

std::vector<std::unique_ptr<Vehicle>>& VehiclePool::getAllVehicles() {
    return pool;
}