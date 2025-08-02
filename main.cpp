#include "json.hpp"
#include <cpr/cpr.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
using json = nlohmann::json;

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
  // std::string url =
  //     "https://api.waifu.im/"
  //     "search?included_tags=raiden-shogun&included_tags=maid&height=>=2000'";
  // std::string url = "http://www.httpbin.org/headers";
  // std::string url = "https://api.waifu.im/tags";
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
    urlToAscii(imgUrl);
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
