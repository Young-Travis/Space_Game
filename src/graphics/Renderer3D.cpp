#include "Graphics/Renderer3D.hpp"

#include <SFML/Graphics/Vertex.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {

constexpr float CLIP_EPSILON = 0.0001f;
constexpr float RASTER_EPSILON = 0.001f;

/*
VIEW-SPACE DEPTH CALCULATION

Algorithm: Camera-space / view-space depth calculation.

The camera is assumed to look along the positive Z direction
according to this renderer's coordinate convention.

cameraDistance represents the camera's position along Z.
Subtracting the vertex's Z coordinate gives its distance
from the camera.

Smaller depth = closer to the camera.
Larger depth  = farther from the camera.

This is not a full view matrix transformation. It is the
depth calculation used by this renderer's camera convention.
*/

float getDepth(const Vector3& point, float cameraDistance) {
    return cameraDistance - point.z;
}

/*
NEAR-PLANE POLYGON CLIPPING

Algorithm: Sutherland-Hodgman polygon clipping.

The near plane is an imaginary plane in front of the camera.
Geometry closer than this plane cannot be projected safely.

Perspective projection divides by depth:

    projectedX = x * focalLength / depth

If depth approaches zero, the projected coordinates approach
infinity. Near-plane clipping prevents this.

Sutherland-Hodgman works by examining each polygon edge:

1. Both vertices inside: keep the current vertex.
2. Both vertices outside: discard the edge.
3. One inside, one outside: calculate their intersection.

The intersection is calculated using Linear Interpolation
(LERP).

LERP formula:

    P(t) = A + (B - A) * t

Where:
    A = starting point
    B = ending point
    t = interpolation amount between 0 and 1

This works for both 3D positions and the depth calculation.
*/
std::vector<Vector3> clipPolygon(
    const std::vector<Vector3>& polygon,
    float cameraDistance,
    float nearPlane
) {
    std::vector<Vector3> clipped;

    if (polygon.empty()) {
        return clipped;
    }

    const float clippingDepth = nearPlane + CLIP_EPSILON;

    // Start with the polygon's final vertex.
    //
    // Polygon edges wrap around, so the final vertex is connected to the first vertex.
    Vector3 previous = polygon.back();

    float previousDepth = getDepth(previous, cameraDistance);

    bool previousInside = previousDepth >= clippingDepth;

    for (const Vector3& current : polygon) {
        const float currentDepth = getDepth(current, cameraDistance);

        const bool currentInside = currentDepth >= clippingDepth;

        // Detect whether the edge crosses the clipping plane.
        //
        // An edge crosses the plane when one endpoint is inside
        // and the other endpoint is outside.

        if (previousInside != currentInside) {

            // Calculate the interpolation parameter t.
            //
            // This determines where along the edge the near plane
            // intersects it.
            //
            // t = (targetDepth - startingDepth) / (endingDepth - startingDepth)
            
            const float t =
                (clippingDepth - previousDepth) /
                (currentDepth - previousDepth);

            // Linear interpolation between the two 3D vertices.
            //
            // Vector3 operator+ and operator* perform the
            // component-wise vector operations.

            const Vector3 intersection =
                previous + (current - previous) * t;

            clipped.push_back(intersection);
        }

        // Keep vertices that are on the valid side of the plane.
        if (currentInside) {
            clipped.push_back(current);
        }

        previous = current;
        previousDepth = currentDepth;
        previousInside = currentInside;
    }

    return clipped;
}


/*
TRIANGLE RASTERIZATION AND BARYCENTRIC COORDINATES

Bounding-box triangle rasterization.

Barycentric coordinate interpolation.

Rasterization is the process of determining which screen
pixels are covered by a geometric primitive.

This function takes a projected triangle and determines
which pixels belong to it.

Instead of checking every pixel on the screen, we calculate
the triangle's axis-aligned bounding box and only examine
pixels inside that rectangle.

Barycentric coordinates describe a position relative to the
three vertices of a triangle.

Any point P inside a triangle can be represented as:

    P = weightA * A +
        weightB * B +
        weightC * C

For points inside the triangle:

    weightA + weightB + weightC = 1

Each weight represents the contribution of its corresponding
vertex.

A point is inside the triangle when all three weights are
non-negative, allowing a small floating-point tolerance.

This lets us interpolate other properties, such as depth.
*/

void rasterizeTriangle(
    const Renderer3D::ProjectedPoint& a,
    const Renderer3D::ProjectedPoint& b,
    const Renderer3D::ProjectedPoint& c,
    unsigned int width,
    unsigned int height,
    std::vector<float>& depthBuffer
) {
    // Extract the projected 2D coordinates of the triangle.
    // The original geometry exists in 3D, but rasterization
    // operates on the 2D screen after projection.
    const float ax = a.position.x;
    const float ay = a.position.y;

    const float bx = b.position.x;
    const float by = b.position.y;

    const float cx = c.position.x;
    const float cy = c.position.y;

    // Calculate the denominator used by the barycentric coordinate equations
    // This is related to the signed area of the triangle.
    // If the denominator is zero, the vertices are collinear and the triangle has no area
    const float denominator =
        (by - cy) * (ax - cx) +
        (cx - bx) * (ay - cy);

    if (std::abs(denominator) < 0.000001f) {
        return;
    }

    // TRIANGLE BOUNDING BOX
    //
    // Find the minimum and maximum screen coordinates.
    //
    // This creates an axis-aligned rectangle around the
    // triangle.
    //
    // Only pixels inside this rectangle need to be tested.
    //
    const float minX = std::min({ax, bx, cx});
    const float maxX = std::max({ax, bx, cx});

    const float minY = std::min({ay, by, cy});
    const float maxY = std::max({ay, by, cy});

    // Clamp the bounding box to the screen dimensions.
    //
    // This prevents accessing pixels outside the depth buffer.
    const int startX = std::max(
        0,
        static_cast<int>(std::floor(minX))
    );

    const int endX = std::min(
        static_cast<int>(width) - 1,
        static_cast<int>(std::ceil(maxX))
    );

    const int startY = std::max(
        0,
        static_cast<int>(std::floor(minY))
    );

    const int endY = std::min(
        static_cast<int>(height) - 1,
        static_cast<int>(std::ceil(maxY))
    );

/*
    RECIPROCAL DEPTH
    
    Algorithm: Reciprocal-depth representation.
    
    Instead of storing depth directly, we store its reciprocal:
    
        q = 1 / depth
    
    Why?
    
    Under perspective projection, reciprocal depth can be
    interpolated linearly across the screen.
    
    Larger reciprocal depth = closer to the camera.
    
    This makes depth comparisons convenient:
    
        q1 > q2
    
    means q1 represents a point closer to the camera.
*/

    const float inverseDepthA = 1.0f / a.depth;
    const float inverseDepthB = 1.0f / b.depth;
    const float inverseDepthC = 1.0f / c.depth;


    // Examine every pixel in the triangle's bounding rectangle
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {

            // Sample at the center of the pixel
            const float px = static_cast<float>(x) + 0.5f;
            const float py = static_cast<float>(y) + 0.5f;


/*
            BARYCENTRIC COORDINATE CALCULATION
            
            Calculate how much each triangle vertex
            contributes to the current pixel position.
            
            These are the barycentric weights:
            
                weightA = contribution of vertex A
                weightB = contribution of vertex B
                weightC = contribution of vertex C
            
            The equations are derived from the signed-area
            formulation of barycentric coordinates.
*/

            const float weightA =
                ((by - cy) * (px - cx) +
                 (cx - bx) * (py - cy)) / denominator;

            const float weightB = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denominator;

            // weights must add up to 1 calculate the third weight from the first two.
            const float weightC = 1.0f - weightA - weightB;

/*
            POINT-IN-TRIANGLE TEST
            
            A point is inside the triangle when all three
            barycentric weights are non-negative.
            
            A small negative tolerance is allowed to account
            for floating-point rounding errors.
*/

            if (weightA < -0.001f ||
                weightB < -0.001f ||
                weightC < -0.001f) {
                continue;
            }

/*
            PERSPECTIVE-CORRECT DEPTH INTERPOLATION
            
            Algorithm: Perspective-correct interpolation.
            
            Interpolate reciprocal depth using the barycentric
            weights rather than interpolating ordinary depth.
            
                q = weightA * qA +
                    weightB * qB +
                    weightC * qC
            
            Where q is reciprocal depth.
            
            This gives the correct depth relationship across
            a projected triangle under perspective projection.
*/

            const float inverseDepth =
                weightA * inverseDepthA +
                weightB * inverseDepthB +
                weightC * inverseDepthC;

/*
            DEPTH BUFFER ADDRESS CALCULATION
            
            The depth buffer is a one-dimensional array
            representing a two-dimensional screen.
            
            Convert (x, y) into a one-dimensional index:
            
                index = y * screenWidth + x
*/

            const std::size_t index =
                static_cast<std::size_t>(y) * width +
                static_cast<std::size_t>(x);

/*
Z-BUFFER DEPTH TEST
            Algorithm: Z-buffering / depth buffering.
            
            Compare the current triangle's depth with the
            closest surface already recorded at this pixel.
            
            Larger reciprocal depth means closer to camera.
            
            If the new surface is closer, replace the old value.
*/
            
            if (inverseDepth > depthBuffer[index]) {
                depthBuffer[index] = inverseDepth;
            }
        }
    }
}

/*
DEPTH-TESTED EDGE DRAWING

Algorithm: Per-pixel hidden-line removal.

The depth buffer now contains the nearest surface at every
covered screen pixel.

This function samples a projected edge and compares its depth
against the depth buffer.

If a face is closer than the edge, that portion of the edge
is hidden and must not be drawn.

If the edge is closer, it is visible.

Visible consecutive samples are grouped into line segments.
*/

void drawDepthTestedEdge(
    sf::RenderTarget& target,
    const Renderer3D::ProjectedPoint& a,
    const Renderer3D::ProjectedPoint& b,
    unsigned int width,
    unsigned int height,
    const std::vector<float>& depthBuffer,
    const sf::Color& color
) {
    // Calculate the 2D direction vector from endpoint A to B.
    //
    // In vector notation:
    //
    //     D = B - A
    //
    // dx and dy are the components of this direction vector.
    const float dx = b.position.x - a.position.x;
    const float dy = b.position.y - a.position.y;


    // Determine the number of samples along the line.
    //
    // We use the larger screen-space dimension so that the
    // sampling density is sufficient to cover the line.
    const int steps = static_cast<int>(
        std::ceil(std::max(std::abs(dx), std::abs(dy)))
    );


    // Calculate reciprocal depth at both endpoints.
    //
    // Reciprocal depth can be interpolated linearly along
    // a projected line under perspective projection.
    const float inverseDepthA = 1.0f / a.depth;
    const float inverseDepthB = 1.0f / b.depth;


    // Track whether we are currently collecting a visible
    // continuous portion of the line.
    bool drawingVisibleSegment = false;

    sf::Vector2f segmentStart{};
    sf::Vector2f segmentEnd{};


    // Lambda function that draws the current visible segment.
    //
    // Grouping visible samples avoids drawing every individual
    // pixel as a separate SFML primitive.
    auto drawSegment = [&]() {
        if (!drawingVisibleSegment) {
            return;
        }

        sf::Vertex line[2];

        line[0].position = segmentStart;
        line[0].color = color;

        line[1].position = segmentEnd;
        line[1].color = color;

        target.draw(line, 2, sf::PrimitiveType::Lines);

        drawingVisibleSegment = false;
    };


    const int sampleCount = std::max(steps, 1);

    for (int i = 0; i <= sampleCount; ++i) {

/*
        LINEAR INTERPOLATION ALONG THE EDGE
        
        Algorithm: Linear interpolation (LERP).
        
        t progresses from 0 to 1:
        
            t = 0: endpoint A
            t = 1: endpoint B
        
        Position is calculated as:
        
            P(t) = A + (B - A) * t
        
        This produces evenly spaced samples along the
        projected 2D line.
*/

        const float t =
            static_cast<float>(i) / static_cast<float>(sampleCount);

        const sf::Vector2f position(
            a.position.x + dx * t,
            a.position.y + dy * t
        );


        // Interpolate reciprocal depth using the same t.
        //
        //     q(t) = qA * (1 - t) + qB * t
        //
        const float inverseDepth =
            inverseDepthA * (1.0f - t) +
            inverseDepthB * t;


        // Convert floating-point screen coordinates into
        // integer pixel coordinates.
        const int pixelX = static_cast<int>(
            std::floor(position.x)
        );

        const int pixelY = static_cast<int>(
            std::floor(position.y)
        );


        bool visible = false;

        /*
            SCREEN BOUNDARY TEST
            
            Before accessing the depth buffer, make sure the
            pixel coordinates are within the screen dimensions.
        
        */

        if (pixelX >= 0 &&
            pixelY >= 0 &&
            pixelX < static_cast<int>(width) &&
            pixelY < static_cast<int>(height)) {

            // Convert 2D coordinates into the depth buffer index.
            const std::size_t index =
                static_cast<std::size_t>(pixelY) * width +
                static_cast<std::size_t>(pixelX);

            const float surfaceDepth = depthBuffer[index];

/*
            DEPTH COMPARISON TOLERANCE
            
            Floating-point arithmetic is not perfectly exact.
            
            A small tolerance helps prevent edges that lie
            directly on their own faces from disappearing
            because of tiny numerical differences.
*/

            const float tolerance =
                std::max(
                    0.00001f,
                    surfaceDepth * RASTER_EPSILON
                );

/*
            EDGE VISIBILITY TEST
            
            Compare the edge's reciprocal depth against the
            nearest surface's reciprocal depth.
            
            If no face has written to this pixel, surfaceDepth
            is zero and the edge is visible.
            
            Otherwise, the edge is visible if it is at least
            as close as the recorded surface, within tolerance.
            
            Larger reciprocal depth = closer to camera.
*/

            visible =
                surfaceDepth == 0.0f ||
                inverseDepth + tolerance >= surfaceDepth;
        }

/*
        VISIBLE LINE SEGMENT COLLECTION
        
        Consecutive visible samples are grouped together.
        
        When an invisible sample is encountered, the current
        visible segment is finished and sent to SFML.

*/

        if (visible) {
            if (!drawingVisibleSegment) {
                segmentStart = position;
                drawingVisibleSegment = true;
            }

            segmentEnd = position;
        } else {
            drawSegment();
        }
    }

    // Draw any visible segment remaining at the end of the edge.
    drawSegment();
}

}

Renderer3D::Renderer3D(
    unsigned int,
    unsigned int height
)
    : focalLength(static_cast<float>(height) * 0.82f),
      nearPlane(0.12f) {
}

/*
3D-TO-2D PERSPECTIVE PROJECTION

Algorithm: Pinhole camera perspective projection.

Perspective projection converts a 3D point into a 2D screen
coordinate.

The mathematical model is:

    x_screen = centerX + (x * focalLength * scale) / depth

    y_screen = centerY - (y * focalLength * scale) / depth

Where:

    x, y        = 3D coordinates
    depth       = distance from camera
    focalLength = camera projection parameter
    scale       = additional model scaling
    centerX/Y   = center of the screen

The division by depth creates perspective:

Objects farther away appear smaller.
Objects closer to the camera appear larger.

The negative sign in the Y equation is necessary because
screen coordinates increase downward, while this renderer's
3D coordinate system treats positive Y as upward.
*/

Renderer3D::ProjectedPoint Renderer3D::project(const Vector3& point, const sf::Vector2f& screenCenter, float scale, float cameraDistance) const {
    const float depth = cameraDistance - point.z;

    // Reject points that are on or behind the near plane.
    if (depth <= nearPlane) {
        return {{}, depth, false};
    }

    // Calculate the perspective projection factor.
    // focalLength divided by depth.
    // Larger depth means a smaller perspective factor.
    const float perspective = focalLength / depth;

    return {{screenCenter.x + point.x * perspective * scale, screenCenter.y - point.y * perspective * scale}, depth, true
    };
}


/*

MAIN MESH RENDERING PIPELINE


Rendering pipeline:

1. Model transformation: rotate local vertices.
2. Depth buffer initialization.
3. Face clipping and projection.
4. Triangle rasterization and depth-buffer construction.
5. Edge projection and depth testing.
6. Draw only visible wireframe segments.

HSR
*/

Renderer3D::DepthBuffer Renderer3D::buildDepthBuffer(
    const sf::RenderTarget& target,
    const Mesh& mesh,
    float yaw,
    float pitch,
    float roll,
    const sf::Vector2f& screenCenter,
    float scale,
    float cameraDistance,
    const Vector3& translation
) const {
    const sf::Vector2u targetSize = target.getSize();
    const unsigned int width = targetSize.x;
    const unsigned int height = targetSize.y;

    if (width == 0 || height == 0) {
        return {};
    }

    std::vector<Vector3> transformed;
    transformed.reserve(mesh.getVertices().size());

    for (const Vector3& vertex : mesh.getVertices()) {
        Vector3 point = rotateZ(vertex, roll);
        point = rotateX(point, pitch);
        point = rotateY(point, yaw);
        transformed.push_back(point + translation);
    }

    DepthBuffer depthBuffer;
    depthBuffer.width = width;
    depthBuffer.height = height;
    depthBuffer.reciprocalDepth.assign(
        static_cast<std::size_t>(width) * height,
        0.0f
    );

    for (const Face& face : mesh.getFaces()) {
        if (face.vertexIndices.size() < 3) {
            continue;
        }

        std::vector<Vector3> polygon;
        polygon.reserve(face.vertexIndices.size());
        bool validFace = true;

        for (unsigned int index : face.vertexIndices) {
            if (index >= transformed.size()) {
                validFace = false;
                break;
            }
            polygon.push_back(transformed[index]);
        }

        if (!validFace) {
            continue;
        }

        polygon = clipPolygon(polygon, cameraDistance, nearPlane);
        if (polygon.size() < 3) {
            continue;
        }

        std::vector<ProjectedPoint> projected;
        projected.reserve(polygon.size());

        for (const Vector3& point : polygon) {
            ProjectedPoint projectedPoint = project(
                point,
                screenCenter,
                scale,
                cameraDistance
            );
            if (!projectedPoint.visible) {
                projected.clear();
                break;
            }
            projected.push_back(projectedPoint);
        }

        for (std::size_t i = 1; i + 1 < projected.size(); ++i) {
            rasterizeTriangle(
                projected[0],
                projected[i],
                projected[i + 1],
                width,
                height,
                depthBuffer.reciprocalDepth
            );
        }
    }

    return depthBuffer;
}

bool Renderer3D::DepthBuffer::isPointVisible(
    const sf::Vector2f& position,
    float depth
) const {
    if (depth <= 0.0f) {
        return false;
    }

    const int pixelX = static_cast<int>(std::floor(position.x));
    const int pixelY = static_cast<int>(std::floor(position.y));

    if (pixelX < 0 || pixelY < 0 ||
        pixelX >= static_cast<int>(width) ||
        pixelY >= static_cast<int>(height)) {
        return false;
    }

    const std::size_t index =
        static_cast<std::size_t>(pixelY) * width +
        static_cast<std::size_t>(pixelX);
    if (index >= reciprocalDepth.size()) {
        return false;
    }

    const float surfaceDepth = reciprocalDepth[index];
    if (surfaceDepth == 0.0f) {
        return true;
    }

    const float inversePointDepth = 1.0f / depth;
    const float tolerance = std::max(
        0.00001f,
        surfaceDepth * RASTER_EPSILON
    );
    return inversePointDepth + tolerance >= surfaceDepth;
}

void Renderer3D::drawMeshEdges(
    sf::RenderTarget& target,
    const Mesh& mesh,
    float yaw,
    float pitch,
    float roll,
    const sf::Vector2f& screenCenter,
    float scale,
    float cameraDistance,
    const DepthBuffer& depthBuffer,
    const Vector3& translation
) const {
    if (depthBuffer.width == 0 || depthBuffer.height == 0) {
        return;
    }

    std::vector<Vector3> transformed;
    transformed.reserve(mesh.getVertices().size());
    for (const Vector3& vertex : mesh.getVertices()) {
        Vector3 point = rotateZ(vertex, roll);
        point = rotateX(point, pitch);
        point = rotateY(point, yaw);
        transformed.push_back(point + translation);
    }

    const sf::Color lineColor(255, 255, 255);
    for (const Edge& edge : mesh.getEdges()) {
        if (edge.start >= transformed.size() ||
            edge.end >= transformed.size()) {
            continue;
        }

        Vector3 a = transformed[edge.start];
        Vector3 b = transformed[edge.end];
        float depthA = getDepth(a, cameraDistance);
        float depthB = getDepth(b, cameraDistance);

        if (depthA <= nearPlane && depthB <= nearPlane) {
            continue;
        }

        if (depthA <= nearPlane || depthB <= nearPlane) {
            const float clippingDepth = nearPlane + CLIP_EPSILON;
            const float t =
                (clippingDepth - depthA) /
                (depthB - depthA);
            const Vector3 clipped = a + (b - a) * t;

            if (depthA <= nearPlane) {
                a = clipped;
                depthA = clippingDepth;
            } else {
                b = clipped;
                depthB = clippingDepth;
            }
        }

        const ProjectedPoint projectedA = project(
            a, screenCenter, scale, cameraDistance
        );
        const ProjectedPoint projectedB = project(
            b, screenCenter, scale, cameraDistance
        );

        if (!projectedA.visible || !projectedB.visible) {
            continue;
        }

        drawDepthTestedEdge(
            target,
            projectedA,
            projectedB,
            depthBuffer.width,
            depthBuffer.height,
            depthBuffer.reciprocalDepth,
            lineColor
        );
    }
}

void Renderer3D::drawMesh(
    sf::RenderTarget& target,
    const Mesh& mesh,
    float yaw,
    float pitch,
    float roll,
    const sf::Vector2f& screenCenter,
    float scale,
    float cameraDistance,
    const Vector3& translation
) const {
    const DepthBuffer depthBuffer = buildDepthBuffer(
        target, mesh, yaw, pitch, roll, screenCenter, scale, cameraDistance,
        translation
    );
    drawMeshEdges(
        target, mesh, yaw, pitch, roll, screenCenter, scale, cameraDistance,
        depthBuffer, translation
    );
}

Renderer3D::ProjectedPoint Renderer3D::projectPoint(
    const Vector3& point,
    const sf::Vector2f& screenCenter,
    float scale,
    float cameraDistance
) const {
    return project(point, screenCenter, scale, cameraDistance);
}
