#include "ImageRenderer.hpp"
#include <cmath>
#include <cpr/cpr.h>
#include <cstdio>
#include <iostream>
#include <limits>
#include <sys/types.h>
#include <vector>

const std::string ImageRenderer::ASCII_CHARS_SIMPLE = " .:-=+*#%@";
const std::string ImageRenderer::ASCII_CHARS_DETAILED =
    " .'`^\",:;Il!i><~+_-?][}{1)(|\\/"
    "tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const std::string ImageRenderer::ASCII_CHARS_BLOCKS =
    " \u2591\u2592\u2593\u2588";

bool ImageRenderer::urlToAscii(const std::string &imgUrl) {
  return urlToAscii(imgUrl, ImageRenderer::RenderOptions{});
}

bool ImageRenderer::urlToAscii(const std::string &imgUrl,
                               const ImageRenderer::RenderOptions &options) {
  auto response = cpr::Get(cpr::Url{imgUrl});
  if (response.status_code != 200) {
    std::cerr << "Failed to download image. Status: " << response.status_code
              << std::endl;
    return false;
  }
  // decoding image
  std::vector<uchar> imgData(response.text.begin(), response.text.end());
  cv::Mat img = cv::imdecode(imgData, cv::IMREAD_COLOR);

  if (img.empty()) {
    std::cerr << "Failed to decode image" << std::endl;
    return false;
  }

  renderImage(img, options);
  return true;
}

const std::string &
ImageRenderer::getCharSet(ImageRenderer::CharStyle style) const {
  switch (style) {
  case ImageRenderer::SIMPLE:
    return ASCII_CHARS_SIMPLE;
  case DETAILED:
    return ASCII_CHARS_DETAILED;
  case BLOCKS:
    return ASCII_CHARS_BLOCKS;
  default:
    return ASCII_CHARS_SIMPLE;
  }
}

std::vector<std::string>
ImageRenderer::splitCharSet(const std::string &charSet) {
  std::vector<std::string> chars;
  if (charSet == getCharSet(BLOCKS)) {
    chars = {" ", "░", "▒", "▓", "█"};
  } else if (charSet == getCharSet(SIMPLE)) {
    chars = {" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"};
  } else {
    for (char c : charSet) {
      chars.push_back(std::string(1, c));
    }
  }
  return chars;
}

void ImageRenderer::renderImage(cv::Mat &img,
                                const ImageRenderer::RenderOptions &options) {
  // adjusting color and brightness
  if (options.contrast != 1.0 || options.brightness != 0.0) {
    img.convertTo(img, -1, options.contrast, options.brightness);
  }

  int target_width = options.width;
  int target_height = options.height;

  if (options.aspectRatio) {
    double aspectRatio = (double)img.cols / img.rows;
    aspectRatio *= 2.0;

    if (aspectRatio > (double)target_width / target_height) {
      target_height = static_cast<int>(target_width / aspectRatio);
    } else {
      target_width = static_cast<int>(target_height * aspectRatio);
    }

    target_width = std::max(target_width, 20);
    target_height = std::max(target_height, 20);
  }

  // resizing img
  cv::resize(img, img, cv::Size(target_width, target_height));

  const std::string &charSet = getCharSet(options.style);
  if (options.colorSupport) {
    renderColorAscii(img, charSet);
  } else {
    renderGrayScaleAscii(img, charSet);
  }
}

void ImageRenderer::renderGrayScaleAscii(const cv::Mat &img,
                                         const std::string &charSet,
                                         const bool &usePalette) {
  cv::Mat gray;
  cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

  auto chars = splitCharSet(charSet);

  std::string line;
  for (int i = 0; i < gray.rows; i++) {
    line = "";
    for (int j = 0; j < gray.cols; j++) {
      int pixel = gray.at<uchar>(i, j);
      int idx = pixel * (chars.size() - 1) / 255;
      line += chars[idx];
    }
    std::cout << line << std::endl;
  }
}

void ImageRenderer::renderColorAscii(const cv::Mat &img,
                                     const std::string &charSet,
                                     const bool &usePalette) {
  cv::Mat gray;
  cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
  auto chars = splitCharSet(charSet);

  struct Color {
    int r, g, b;
  };
  std::vector<Color> palette = {
      {255, 218, 185}, // Peach (skin tone)
      {255, 192, 203}, // Pink (hair)
      {0, 128, 255},   // Sky Blue (eyes)
      {0, 0, 0},       // Black (outlines)
      {255, 215, 0},   // Gold (accents)
      {255, 99, 71}    // Tomato (bold color)
  };

  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols; j++) {
      cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
      int b = pixel[0], g = pixel[1], r = pixel[2];

      r = std::max(0, std::min(255, r));
      g = std::max(0, std::min(255, g));
      b = std::max(0, std::min(255, b));

      int brightness = gray.at<uchar>(i, j);
      int maxIdx = (int)chars.size() - 1;
      int calcIdx = brightness * maxIdx / 255;
      int idx = std::max(0, std::min(maxIdx, calcIdx));

      if (usePalette) {
        float minDist = std::numeric_limits<float>::max();
        Color closestColor = palette[0];
        for (const auto &color : palette) {
          float dist =
              std::sqrt(std::pow(r - color.r, 2) + std::pow(g - color.g, 2) +
                        std::pow(b - color.b, 2));
          if (dist < minDist) {
            minDist = dist;
            closestColor = color;
          }
        }
        r = closestColor.r;
        g = closestColor.g;
        b = closestColor.b;
      }

      if (idx < chars.size() && !chars[idx].empty()) {
        printf("\x1b[48;2;%d;%d;%dm%s\x1b[0m", r, g, b, chars[idx].c_str());
      } else {
        printf("\x1b[48;2;%d;%d;%dm \x1b[0m", r, g, b);
      }
    }
    printf("\n");

    // flushing periodically to prevent buffering issues
    if (i % 5 == 0) {
      fflush(stdout);
    }
  }
  fflush(stdout);
}
