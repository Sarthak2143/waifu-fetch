#include "json.hpp"
#include <cpr/cpr.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
using json = nlohmann::json;

class ImageRenderer {
private:
  const std::string ASCII_CHARS_SIMPLE = " .:-=+*#%@";
  const std::string ASCII_CHARS_DETAILED =
      " .'`^\",:;Il!i><~+_-?][}{1)(|\\/"
      "tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  const std::string ASCII_CHARS_BLOCKS = " ░▒▓█";

public:
  enum CharStyle {
    SIMPLE,
    DETAILED,
    BLOCKS,
  };

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
  std::string getCharSet(CharStyle style) {
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

  void renderImage(cv::Mat &img, const RenderOptions &options) {
    // adjusting color and brightness
    if (options.contrast != 1.0 || options.brightness != 0.0) {
      img.convertTo(img, -1, options.contrast, options.brightness);
    }

    int width = options.width;
    int height = options.height;

    if (options.aspectRatio) {
      double aspectRatio = (double)img.cols / img.rows;
      aspectRatio *= 0.5;

      if (aspectRatio > (double)width / height) {
        height = width / aspectRatio;
      } else {
        width = height * aspectRatio;
      }
    }
    // resizing img
    cv::resize(img, img, cv::Size(width, height));

    std::string charSet = getCharSet(options.style);
    if (options.colorSupport) {
      renderColorAscii(img, charSet);
    } else {
      renderGrayScaleAscii(img, charSet);
    }
  }

  void renderGrayScaleAscii(const cv::Mat &img, const std::string &charSet) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    for (int i = 0; i < gray.rows; i++) {
      for (int j = 0; j < gray.cols; j++) {
        int pixel = gray.at<uchar>(i, j);
        int idx = pixel * (charSet.length() - 1) / 255;
        std::cout << charSet[idx];
      }
      std::cout << std::endl;
    }
  }

  void renderColorAscii(const cv::Mat &img, const std::string &charSet) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    for (int i = 0; i < gray.rows; i++) {
      for (int j = 0; j < gray.cols; j++) {
        cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
        int b = pixel[0], g = pixel[1], r = pixel[2];

        int brightness = gray.at<uchar>(i, j);
        int idx = brightness * (charSet.length() - 1) / 255;

        printf("\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, charSet[idx]);
      }
      std::cout << std::endl;
    }
  }
};

std::string userAgent =
    "Mozilla/5.0 (X11; Linux x86_64; rv:141.0) Gecko/20100101 Firefox/141.0";
std::string acceptHeader =
    "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";

bool downloadImg(const std::string &imgUrl, const std::string &fileName);
void displayImg(const std::string &fileName);
void urlToAscii(const std::string &imgUrl);

int main(int argc, char **argv) {
  std::cout << "OpenCV version: " << CV_VERSION << std::endl;
  std::string tags[] = {"maid",          "waifu",         "marin-kitagawa",
                        "mori-calliope", "raiden-shogun", "oppai",
                        "selfies",       "uniform",       "kamisato-ayaka"};
  if (argc != 2) {
    return 1;
  }
  std::string tag = std::string(argv[1]);
  bool found = false;
  for (std::string t : tags) {
    if (t == tag) {
      found = true;
      break;
    }
  }
  if (!found) {
    std::cout << "Not a valid tag" << std::endl;
    return 1;
  }

  std::string url = "https://api.waifu.im/search";
  std::string tempFile = "temp.jpg";
  auto response = cpr::Get(
      cpr::Url{url}, cpr::Parameters{{"included_tags", tag}},
      cpr::Header{{"User-Agent", userAgent}, {"accept", acceptHeader}});

  if (response.status_code == 200) {
    json data = json::parse(response.text);
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
    opts.width = 100;
    opts.height = 30;
    opts.style = ImageRenderer::DETAILED;
    opts.colorSupport = true;

    renderer.urlToAscii(imgUrl, opts);
  }
  return 0;
}

bool downloadImg(const std::string &imgUrl, const std::string &fileName) {
  std::ofstream of(fileName, std::ios::binary);
  auto response = cpr::Download(
      of, cpr::Url{imgUrl},
      cpr::Header{{"User-Agent", userAgent}, {"accept", acceptHeader}});
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
