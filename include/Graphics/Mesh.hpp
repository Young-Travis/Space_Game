#ifndef MESH_HPP
#define MESH_HPP

#include "Math/Vector3.hpp"

#include <vector>

struct Edge {
    unsigned int start{};
    unsigned int end{};
};

struct Face {
    std::vector<unsigned int> vertexIndices;
};

class Mesh {
public:
    void addVertex(const Vector3& vertex);
    void addEdge(unsigned int start, unsigned int end);
    void addFace(const std::vector<unsigned int>& vertexIndices);

    const std::vector<Vector3>& getVertices() const;
    const std::vector<Edge>& getEdges() const;
    const std::vector<Face>& getFaces() const;

private:
    std::vector<Vector3> vertices;
    std::vector<Edge> edges;
    std::vector<Face> faces;
};

#endif