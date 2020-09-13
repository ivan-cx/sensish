#include "open_file.h"

#include <algorithm> // for std::sort

#include "imgui.h" // for ImGui::*

std::filesystem::path openFile(std::filesystem::path &currentBrowsePath) {
  // ------------------------------------------
  // update :
  std::vector<std::filesystem::path> files{};
  std::vector<std::filesystem::path> directories{};
  for (const auto &entry :
       std::filesystem::directory_iterator(currentBrowsePath)) {
    if (entry.is_directory()) {
      directories.emplace_back(entry.path());
    } else {
      auto extension = entry.path().extension();
      if (!strcmp(extension.c_str(), ".tiff") ||
          !strcmp(extension.c_str(), ".tif") ||
          !strcmp(extension.c_str(), ".png") ||
          !strcmp(extension.c_str(), ".jpg") ||
          !strcmp(extension.c_str(), ".jpeg") ||
          !strcmp(extension.c_str(), ".tga") ||
          !strcmp(extension.c_str(), ".gif") ||
          !strcmp(extension.c_str(), ".pic") ||
          !strcmp(extension.c_str(), ".pgm") ||
          !strcmp(extension.c_str(), ".ppm") ||
          !strcmp(extension.c_str(), ".psd") ||
          !strcmp(extension.c_str(), ".bmp")) {
        files.emplace_back(entry.path());
      }
    }
  }
  std::sort(directories.begin(), directories.end());
  std::sort(files.begin(), files.end());

  std::filesystem::path result{};
  // ------------------------------------------
  // draw:
  {
    ImVec2 winSize = ImGui::GetIO().DisplaySize;
    winSize.x *= 0.9;
    winSize.y *= 0.9;
    ImGui::SetNextWindowSize(std::move(winSize), ImGuiCond_Once);
    winSize = ImGui::GetIO().DisplaySize;
    winSize.x /= 2;
    winSize.y /= 2;
    ImGui::SetNextWindowPos(std::move(winSize), ImGuiCond_Once, {0.5, 0.5});
  }
  if (ImGui::Begin("Select image")) {

    ImGui::TextUnformatted(currentBrowsePath.c_str());

    if (ImGui::Button(".. (parent directory)")) {
      currentBrowsePath = currentBrowsePath.parent_path();
    }
    for (const std::filesystem::path &dir : directories) {
      if (ImGui::Button(dir.filename().c_str())) {
        currentBrowsePath = dir;
      }
    }
    if (files.size() > 0) {
      ImGui::Text("\tImages in current directory:");
      for (const std::filesystem::path &file : files) {
        if (ImGui::Button(file.filename().c_str())) {
          result = file;
        }
      }
    } else {
      ImGui::Text("\n\tThere is no images in current directory.\n"
                  "Please, select directory that contains some images");
    }
  }
  ImGui::End();
  return result;
}

std::filesystem::path getDefaultWorkingDirectory() {
  std::filesystem::path result{std::filesystem::current_path()};
#ifdef _WIN32
  const char *homeDrive = getenv("HOMEDRIVE");
  const char *homePath = getenv("HOMEPATH");
  if (homeDrive != nullptr && homePath != nullptr) {
    const int MAX_PATH = 8192;
    char homedir[MAX_PATH];
    int printResult = snprintf(homedir, MAX_PATH, "%s%s", , getenv("HOMEPATH"));
    if (printResult > 0 && printResult < MAX_PATH)
      result = printResult;
  }
#else
  const char *homePath = getenv("HOME");
  if (homePath != nullptr)
    result = homePath;
#endif
  return result;
}

