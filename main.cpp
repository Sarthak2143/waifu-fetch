#include "ImageRenderer.hpp"
#include "json.hpp"
#include <cpr/cpr.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

bool downloadImg(const std::string &imgUrl, const std::string &fileName);
void displayImg(const std::string &fileName);

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  std::locale::global(std::locale("en_US.UTF-8"));

  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);

  std::cout << "OpenCV version: " << CV_VERSION << '\n';
  std::string tags[] = {"maid",          "waifu",         "marin-kitagawa",
                        "mori-calliope", "raiden-shogun", "oppai",
                        "selfies",       "uniform",       "kamisato-ayaka"};
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " <tag> [--ascii]" << std::endl;
    std::cerr << "Example: " << argv[0] << " waifu --ascii" << std::endl;
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

  bool asciiMode = false;
  // check for the --ascii flag
  for (int i = 2; i < argc; ++i) {
    if (std::string(argv[i]) == "--ascii") {
      asciiMode = true;
      break;
    }
  }

  std::string url = "https://api.waifu.im/search";
  auto response =
      cpr::Get(cpr::Url{url}, cpr::Parameters{{"included_tags", tag}});

  if (response.status_code != 200) {
    std::cerr << "Error: API request failed. Status: " << response.status_code
              << std::endl;
    return 1;
  }

  try {
    json data = json::parse(response.text);
    if (!data.contains("images") || !data["images"].is_array() ||
        data["images"].empty()) {
      std::cerr << "Error: API response doesn't contain valid image data.\n";
      return 1;
    }
    std::string imgUrl = data["images"][0]["url"];

    if (asciiMode) {
      ImageRenderer renderer;
      ImageRenderer::RenderOptions opts;
      opts.style = ImageRenderer::DETAILED;
      opts.colorSupport = true;

      renderer.urlToAscii(imgUrl, opts);
    } else {
      std::string tempFile = "temp_img.jpg";

      if (downloadImg(imgUrl, tempFile)) {
        displayImg(tempFile);
        remove(tempFile.c_str());
      } else {
        std::cout << "Failed to download image." << std::endl;
      }
    }
  } catch (const json::parse_error &e) {
    std::cerr << "Error: Failed to parse API response. " << e.what()
              << std::endl;
    return 1;
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
}
