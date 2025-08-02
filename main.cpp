#include "json.hpp"
#include <cpr/cpr.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
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

  void renderMatToAscii(cv::Mat &img, const RenderOptions &options) {
    // renderIma
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

  void renderImage(cv::Mat &img, const RenderOptions &options) {
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

  void renderGrayScaleAscii(const cv::Mat &img, const std::string &charSet) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    for (int i = 0; i < gray.rows; i++) {
      for (int j = 0; j < gray.cols; j++) {
        int pixel = gray.at<uchar>(i, j);
        int idx = pixel * (charSet.length() - 1) / 255;
        std::cout << charSet[idx];
      }
      std::cout << '\n';
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

        // \033[38;2;R;G;Bm : set foreground color
        // \033[0m          : reset attributes
        printf("\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, charSet[idx]);
      }
      std::cout << '\n';
    }
  }
};

const std::string ImageRenderer::ASCII_CHARS_SIMPLE = " .:-=+*#%@";
const std::string ImageRenderer::ASCII_CHARS_DETAILED =
    " .'`^\",:;Il!i><~+_-?][}{1)(|\\/"
    "tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const std::string ImageRenderer::ASCII_CHARS_BLOCKS = " ░▒▓█";

bool downloadImg(const std::string &imgUrl, const std::string &fileName);
void displayImg(const std::string &fileName);
void urlToAscii(const std::string &imgUrl);

int main(int argc, char **argv) {
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
    opts.width = 130;
    opts.height = 70;
    opts.style = ImageRenderer::SIMPLE;
    opts.aspectRatio = true;
    opts.brightness = 10.0;
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
