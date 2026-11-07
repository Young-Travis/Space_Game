#ifndef SHIP_HPP
#define SHIP_HPP

#include "Graphics/Mesh.hpp"

class Ship {
public:
    Ship();

    const Mesh& getMesh() const;

private:
    Mesh mesh;

    void createGeometry();
};

#endif