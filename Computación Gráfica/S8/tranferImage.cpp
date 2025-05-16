#include <cmath>
#include <openscv2/opencv.hpp>

void rectangleToCircle(const cv::Mat &input, cv::Mat &output, int radius) {
  // Crear imagen circular de salida (fondo transparente o negro)
  output = cv::Mat::zeros(radius * 2, radius * 2, input.type());

  int centerX = radius;
  int centerY = radius;

  for (int y = 0; y < output.rows; y++) {
    for (int x = 0; x < output.cols; x++) {
      // Calcular distancia al centro
      double dx = x - centerX;
      double dy = y - centerY;
      double distance = sqrt(dx * dx + dy * dy);

      // Si el punto está dentro del círculo
      if (distance <= radius) {
        // Convertir a coordenadas polares normalizadas
        double theta = atan2(dy, dx);
        double r = distance / radius;

        // Mapear a la imagen original
        int srcX = static_cast<int>((theta + M_PI) / (2 * M_PI) * input.cols);
        int srcY = static_cast<int>(r * input.rows);

        // Asegurar que las coordenadas están dentro de los límites
        srcX = std::min(std::max(srcX, 0), input.cols - 1);
        srcY = std::min(std::max(srcY, 0), input.rows - 1);

        // Copiar el color
        output.at<cv::Vec3b>(y, x) = input.at<cv::Vec3b>(srcY, srcX);
      }
    }
  }
}