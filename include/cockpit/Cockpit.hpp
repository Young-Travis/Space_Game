#ifndef COCKPIT_HPP
#define COCKPIT_HPP

#include "Graphics/Mesh.hpp"

class Cockpit {
public:
    Cockpit();

    const Mesh& getMesh() const;

private:
    Mesh mesh;

    void createGeometry();
};

#endif