#include "solution.h"

void frames_of_a_textured_sphere_rotating(
    std::string const &full_path_input_mesh,
    std::string const &full_path_input_texture, double rotation_line_point_x,
    double rotation_line_point_y, double rotation_line_point_z,
    double rotation_line_direction_x, double rotation_line_direction_y,
    double rotation_line_direction_z,
    std::string const &filename_without_suffix_output_frames) {

  RotatingRenderer renderer;

  if (!renderer.loadTexture(full_path_input_texture)) {
    cerr << "Failed to load texture from: " << full_path_input_texture << endl;
    return;
  }

  if (!renderer.loadPLYMesh(full_path_input_mesh)) {
    cerr << "Failed to load mesh from: " << full_path_input_mesh << endl;
    return;
  }

  double minX = -1.5, minY = -1.5, maxX = 1.5, maxY = 1.5;
  size_t width = 800, height = 800;

  for (int frame = 1; frame <= 360; frame++) {
    double angle = frame - 1;

    vector<Vertex> rotatedVertices = renderer.rotateVertices(
        angle, rotation_line_point_x, rotation_line_point_y,
        rotation_line_point_z, rotation_line_direction_x,
        rotation_line_direction_y, rotation_line_direction_z);

    Image image(width, height);
    renderer.render(image, rotatedVertices, minX, minY, maxX, maxY, width,
                    height);

    string filename =
        filename_without_suffix_output_frames + "-" + to_string(frame) + ".png";
    image.savePNG(filename);
  }
}

int main() {
  frames_of_a_textured_sphere_rotating("texture.ply", "t2.jpeg", 0, 0, 0, 0, 1,
                                       0, "frame");

  return 0;
}