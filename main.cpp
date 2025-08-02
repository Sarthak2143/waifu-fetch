#include "json.hpp"
#include <clocale>
#include <cpr/cpr.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

class ImageRenderer {
private:
  static const std::string ASCII_CHARS_SIMPLE;
  static const std::string ASCII_CHARS_DETAILED;
  static const std::string ASCII_CHARS_BLOCKS;

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

  bool urlToAscii(const std::string &imgUrl) {
    return urlToAscii(imgUrl, RenderOptions{});
  }

  bool urlToAscii(const std::string &imgUrl, const RenderOptions &options) {
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

private:
  const std::string &getCharSet(CharStyle style) const {
    switch (style) {
    case SIMPLE:
      return ASCII_CHARS_SIMPLE;
    case DETAILED:
      return ASCII_CHARS_DETAILED;
    case BLOCKS:
      return ASCII_CHARS_BLOCKS;
    default:
      return ASCII_CHARS_SIMPLE;
    }
  }

  std::vector<std::string> splitCharSet(const std::string &charSet) {
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

  void renderImage(cv::Mat &img, const RenderOptions &options) {
    // adjusting color and brightness
    if (options.contrast != 1.0 || options.brightness != 0.0) {
      img.convertTo(img, -1, options.contrast, options.brightness);
    }

    int target_width = options.width;
    int target_height = options.height;

    std::cout << "Original image size: " << img.cols << "x" << img.rows
              << std::endl;
    std::cout << "Target size: " << target_width << "x" << target_height
              << std::endl;

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

    std::cout << "Adjusted size: " << target_width << "x" << target_height
              << std::endl;
    // resizing img
    cv::resize(img, img, cv::Size(target_width, target_height));
    std::cout << "Final image size: " << img.cols << "x" << img.rows
              << std::endl;

    const std::string &charSet = getCharSet(options.style);
    if (options.colorSupport) {
      renderColorAscii(img, charSet);
    } else {
      renderGrayScaleAscii(img, charSet);
    }
  }

  void renderGrayScaleAscii(const cv::Mat &img, const std::string &charSet) {
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

  // void renderColorAscii(const cv::Mat &img, const std::string &charSet) {
  //   for (int i = 0; i < img.rows; i++) {
  //     std::string line = "";
  //     for (int j = 0; j < img.cols; j++) {
  //       cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
  //       int b = pixel[0], g = pixel[1], r = pixel[2];
  //
  //       // Use background color with space (more stable)
  //       std::ostringstream colorStr;
  //       colorStr << "\033[48;2;" << r << ";" << g << ";" << b << "m \033[0m";
  //       line += colorStr.str();
  //     }
  //     std::cout << line << std::endl;
  //   }
  // }

  void renderColorAscii(const cv::Mat &img, const std::string &charSet) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    auto chars = splitCharSet(charSet);

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
};

const std::string ImageRenderer::ASCII_CHARS_SIMPLE = " .:-=+*#%@";
const std::string ImageRenderer::ASCII_CHARS_DETAILED =
    " .'`^\",:;Il!i><~+_-?][}{1)(|\\/"
    "tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const std::string ImageRenderer::ASCII_CHARS_BLOCKS =
    " \u2591\u2592\u2593\u2588";

bool downloadImg(const std::string &imgUrl, const std::string &fileName);
void displayImg(const std::string &fileName);
void urlToAscii(const std::string &imgUrl);

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  std::locale::global(std::locale("en_US.UTF-8"));

  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);

  std::cout << "OpenCV version: " << CV_VERSION << '\n';
  std::string tags[] = {"maid",          "waifu",         "marin-kitagawa",
                        "mori-calliope", "raiden-shogun", "oppai",
                        "selfies",       "uniform",       "kamisato-ayaka"};
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <tag>\n";
    return 1;
  }
  std::string tag = std::string(argv[1]);
  bool found = false;
  for (const std::string &t : tags) {
    if (t == tag) {
      found = true;
      break;
    }
  }
  if (!found) {
    std::cerr << "Error: Not a valid tag. Valid tags are:\n";
    for (const std::string &t : tags) {
      std::cerr << "- " << t << '\n';
    }
    return 1;
  }

  std::string url = "https://api.waifu.im/search";
  std::string tempFile = "temp.jpg";
  auto response =
      cpr::Get(cpr::Url{url}, cpr::Parameters{{"included_tags", tag}});

  if (response.status_code == 200) {
    json data = json::parse(response.text);
    if (!data.contains("images") || !data["images"].is_array() ||
        data["images"].empty()) {
      std::cerr << "Error: API response doesn't contain valid image data.\n";
      return 1;
    }
    std::string imgUrl = data["images"][0]["url"];

    if (downloadImg(imgUrl, "temp.jpg")) {
      displayImg("temp.jpg");
    } else {
      std::cout << "Failed to download image." << std::endl;
    }
    remove(tempFile.c_str());
    // urlToAscii(imgUrl);

    ImageRenderer renderer;
    ImageRenderer::RenderOptions opts;
    opts.width = 80;
    opts.height = 55;
    opts.style = ImageRenderer::BLOCKS;
    opts.aspectRatio = false;
    // opts.brightness = 10.0;
    opts.colorSupport = true;

    renderer.urlToAscii(imgUrl, opts);
  }
  return 0;
}

bool downloadImg(const std::string &imgUrl, const std::string &fileName) {
  std::ofstream of(fileName, std::ios::binary);
  auto response = cpr::Download(of, cpr::Url{imgUrl});
  if (response.status_code == 200) {
    return true;
  }
  return false;
}

void displayImg(const std::string &fileName) {
  system(("chafa " + fileName).c_str());
  system(("ascii-image-converter " + fileName).c_str());
}

void urlToAscii(const std::string &imgUrl) {
  auto response = cpr::Get(cpr::Url{imgUrl});
  if (response.status_code == 200) {
    // write to temp file
    std::vector<uchar> imageData(response.text.begin(), response.text.end());
    cv::Mat img = cv::imdecode(imageData, cv::IMREAD_GRAYSCALE);

    // conv to ascii :0
    cv::resize(img, img, cv::Size(80, 40));
    std::string chars = " .:-=*#%@";
    for (size_t i = 0; i < img.rows; i++) {
      for (size_t j = 0; j < img.cols; j++) {
        int pixel = img.at<uchar>(i, j);
        int index = pixel * (chars.length() - 1) / 255;
        std::cout << chars[index];
      }
      std::cout << std::endl;
    }
  }
}
