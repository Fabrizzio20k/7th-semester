#include "solution.h"

void painter_algorithm_textures(std::string const &full_path_input_mesh,
                                std::string const &full_path_input_texture,
                                std::string const &full_path_output_image,
                                double min_x_coordinate_in_projection_plane,
                                double min_y_coordinate_in_projection_plane,
                                double max_x_coordinate_in_projection_plane,
                                double max_y_coordinate_in_projection_plane,
                                size_t width_in_pixels,
                                size_t height_in_pixels) {

  PainterAlgorithmRenderer renderer;

  if (!renderer.loadTexture(full_path_input_texture)) {
    cerr << "Failed to load texture from: " << full_path_input_texture << endl;
    return;
  }

  if (!renderer.loadPLYMesh(full_path_input_mesh)) {
    cerr << "Failed to load mesh from: " << full_path_input_mesh << endl;
    return;
  }

  Image image(width_in_pixels, height_in_pixels);

  renderer.render(image, min_x_coordinate_in_projection_plane,
                  min_y_coordinate_in_projection_plane,
                  max_x_coordinate_in_projection_plane,
                  max_y_coordinate_in_projection_plane, width_in_pixels,
                  height_in_pixels);

  string extension =
      full_path_output_image.substr(full_path_output_image.find_last_of("."));
  if (extension == ".png") {
    image.savePNG(full_path_output_image);
  } else if (extension == ".jpg" || extension == ".jpeg") {
    image.saveJPG(full_path_output_image);
  } else {
    image.savePNG(full_path_output_image);
  }
}

int main() {
  string input_mesh = "texture.ply";
  string input_texture = "t3.jpg";
  string output_image = "a.png";
  double min_x = -1.0;
  double min_y = -1.0;
  double max_x = 1.0;
  double max_y = 1.0;

  size_t width = 640;
  size_t height = 640;

  painter_algorithm_textures(input_mesh, input_texture, output_image, min_x,
                             min_y, max_x, max_y, width, height);
  return 0;
}