#ifndef IMAGE_RENDERER_HPP
#define IMAGE_RENDERER_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @class ImageRenderer
 * @brief Renders images as ASCII art in the terminal.
 */
class ImageRenderer {
public:
  enum CharStyle { SIMPLE, DETAILED, BLOCKS };

  struct RenderOptions {
    int width = 120;
    int height = 40;
    CharStyle style = SIMPLE;
    bool colorSupport = false;
    bool aspectRatio = true;
    double contrast = 1.0;
    double brightness = 0.0;
  };

  bool urlToAscii(const std::string &imgUrl);
  bool urlToAscii(const std::string &imgUrl, const RenderOptions &options);

private:
  static const std::string ASCII_CHARS_SIMPLE;
  static const std::string ASCII_CHARS_DETAILED;
  static const std::string ASCII_CHARS_BLOCKS;

  const std::string &getCharSet(CharStyle style) const;
  std::vector<std::string> splitCharSet(const std::string &charSet);
  void renderImage(cv::Mat &img, const RenderOptions &options);
  void renderGrayScaleAscii(const cv::Mat &img, const std::string &charSet);
  void renderColorAscii(const cv::Mat &img, const std::string &charSet);
  void betterRenderColorAscii(const cv::Mat &img, const std::string &charSet);
};

#endif // IMAGE_RENDERER_HPP
