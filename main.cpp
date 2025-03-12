#include <iostream>
#include <filesystem>
#include <map>
#include <chrono>

namespace fs = std::filesystem;

void organizeFiles(const std::string &directory)
{
  std::map<std::string, std::string> fileCategories = {
      {".jpg", "Images"}, {".png", "Images"}, {".mp4", "Videos"}, {".pdf", "Documents"}, {".docx", "Documents"}, {".cpp", "Code"}, {".mp3", "Music"}, {".txt", "Text Files"}};

  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (fs::is_regular_file(entry))
    {
      std::string ext = entry.path().extension().string();
      if (fileCategories.find(ext) != fileCategories.end())
      {
        std::string newFolder = directory + "/" + fileCategories[ext];
        fs::create_directories(newFolder);
        fs::rename(entry.path(), newFolder + "/" + entry.path().filename().string());
      }
    }
  }
}
#include <chrono>

std::string getTimeCategory(fs::file_time_type ftime)
{
  auto now = std::chrono::system_clock::now();
  auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

  auto file_time = std::chrono::duration_cast<std::chrono::hours>(sctp.time_since_epoch()).count() / 24;
  auto today = std::chrono::duration_cast<std::chrono::hours>(now.time_since_epoch()).count() / 24;

  if (file_time == today)
    return "Today";
  else if (file_time >= today - 7)
    return "Last Week";
  else
    return "Older";
}

void sortByDate(const std::string &directory)
{
  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (fs::is_regular_file(entry))
    {
      auto modTime = fs::last_write_time(entry);
      std::string category = getTimeCategory(modTime);

      std::string newFolder = directory + "/" + category;
      fs::create_directories(newFolder);
      fs::rename(entry.path(), newFolder + "/" + entry.path().filename().string());
    }
  }
}

int main()
{
  std::string path;
  int choice;
  std::cout << "Enter the folder path to organize:  ";
  std::cin >> path;
  organizeFiles(path);
  std::cout << "Choose an option:\n1. Organize by File Type\n2. Organize by Date\n";
  std::cin >> choice;

  if (choice == 1)
    organizeFiles(path);
  else if (choice == 2)
    sortByDate(path);
  else
    std::cout << "Invalid choice!";

  std::cout << "File organization completed!" << std::endl;
  return 0;
}