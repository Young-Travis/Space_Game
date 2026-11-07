
import bpy
import os
import re


def cpp_identifier(name):
    """Convert a Blender object name into a valid C++ identifier."""
    name = re.sub(r"[^a-zA-Z0-9_]", "_", name)

    if not name:
        name = "GeneratedMesh"

    if name[0].isdigit():
        name = "_" + name

    return name


def cpp_float(value):
    """Format a number as a valid C++ float literal."""
    value = float(value)

    if abs(value) < 0.000000001:
        value = 0.0

    result = format(value, ".9g")

    if "." not in result and "e" not in result.lower():
        result += ".0"

    return result + "f"


def export_mesh():
    # Get the currently selected object.
    obj = bpy.context.active_object

    if obj is None or obj.type != "MESH":
        raise RuntimeError("Please select a mesh object first.")

    if not bpy.data.filepath:
        raise RuntimeError("Please save your Blender project first.")

    # Get the mesh data.
    mesh_data = obj.data

    # Include object rotation and scale, but not world position.
    transform = obj.matrix_basis.to_3x3()

    # Generate a valid C++ identifier.
    name = cpp_identifier(obj.name)

    # Create the output directory next to the Blender project.
    output_dir = os.path.join(
        bpy.path.abspath("//"),
        "generated_geometry"
    )

    os.makedirs(output_dir, exist_ok=True)

    header_path = os.path.join(
        output_dir,
        f"{name}Geometry.hpp"
    )

    source_path = os.path.join(
        output_dir,
        f"{name}Geometry.cpp"
    )

    function_name = f"build{name}Geometry"

    # --------------------------------------------------
    # Generate the header.
    # --------------------------------------------------

    header = f"""#pragma once

#include "Graphics/Mesh.hpp"

void {function_name}(Mesh& mesh);
"""

    # --------------------------------------------------
    # Generate the source.
    # --------------------------------------------------

    source = f"""#include "{name}Geometry.hpp"

void {function_name}(Mesh& mesh) {{
"""

    # --------------------------------------------------
    # Export vertices.
    # --------------------------------------------------

    source += "    // Vertices\n\n"

    for vertex in mesh_data.vertices:
        blender_position = transform @ vertex.co

        # Convert Blender coordinates to game coordinates.
        # Blender: X = horizontal, Y = depth, Z = vertical.
        # Game:    X = horizontal, Y = vertical, Z = depth.

        x = blender_position.x
        y = blender_position.z
        z = -blender_position.y

        source += (
            f"    mesh.addVertex({{{cpp_float(x)}, "
            f"{cpp_float(y)}, {cpp_float(z)}}});\n"
        )

    # --------------------------------------------------
    # Export edges.
    # --------------------------------------------------

    source += "\n    // Edges\n\n"

    for edge in mesh_data.edges:
        start = edge.vertices[0]
        end = edge.vertices[1]

        source += (
            f"    mesh.addEdge({start}, {end});\n"
        )

    # --------------------------------------------------
    # Export faces.
    # --------------------------------------------------

    source += "\n    // Faces\n\n"

    for face in mesh_data.polygons:
        indices = ", ".join(
            str(index) for index in face.vertices
        )

        source += (
            f"    mesh.addFace({{{indices}}});\n"
        )

    source += "}\n"

    # --------------------------------------------------
    # Write the generated files.
    # --------------------------------------------------

    with open(header_path, "w", encoding="utf-8") as file:
        file.write(header)

    with open(source_path, "w", encoding="utf-8") as file:
        file.write(source)

    # --------------------------------------------------
    # Display export information.
    # --------------------------------------------------

    print("\n================================")
    print("       C++ MESH EXPORT")
    print("================================")

    print(f"Object:   {obj.name}")
    print(f"Vertices: {len(mesh_data.vertices)}")
    print(f"Edges:    {len(mesh_data.edges)}")
    print(f"Faces:    {len(mesh_data.polygons)}")

    print("--------------------------------")

    print(f"Header: {header_path}")
    print(f"Source: {source_path}")

    print("================================\n")


export_mesh()
