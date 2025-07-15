#pragma once

#include "Vehicle.h"
#include <vector>
#include <memory>

class VehiclePool {
public:
    explicit VehiclePool(size_t initialSize = 20);

    Vehicle* getVehicle();
    void returnVehicle(Vehicle* vehicle);

    std::vector<std::unique_ptr<Vehicle>>& getAllVehicles();

private:
    std::vector<std::unique_ptr<Vehicle>> pool;
    size_t maxSize;
};