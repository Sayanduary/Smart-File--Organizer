#include <iostream>
#include <filesystem>
#include <map>
#include <fstream>
#include <chrono>
#include <vector>

namespace fs = std::filesystem;

// Function to log file moves for undo
void logMove(const fs::path &oldPath, const fs::path &newPath)
{
  std::ofstream logFile("undo.log", std::ios::app);
  if (logFile)
  {
    logFile << oldPath.string() << "\t" << newPath.string() << std::endl; // Using tab as delimiter
  }
}

// Function to organize files by type
void organizeFiles(const std::string &directory)
{
  std::map<std::string, std::string> fileCategories = {
      {".avif", "Images"}, {".jpg", "Images"}, {".png", "Images"}, {".mp4", "Videos"}, {".pdf", "Documents"}, {".docx", "Documents"}, {".xlsx", "Documents"}, {".cpp", "Code"}, {".mp3", "Music"}, {".txt", "Text Files"}};

  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (fs::is_regular_file(entry))
    {
      std::string ext = entry.path().extension().string();
      if (fileCategories.find(ext) != fileCategories.end())
      {
        fs::path newFolder = fs::path(directory) / fileCategories[ext];
        fs::create_directories(newFolder);

        fs::path newPath = newFolder / entry.path().filename();
        logMove(entry.path(), newPath);

        fs::rename(entry.path(), newPath);
      }
    }
  }
}

// Function to get file modification category
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

// Function to sort files by modification date
void sortByDate(const std::string &directory)
{
  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (fs::is_regular_file(entry))
    {
      auto modTime = fs::last_write_time(entry);
      std::string category = getTimeCategory(modTime);

      fs::path newFolder = fs::path(directory) / category;
      fs::create_directories(newFolder);

      fs::path newPath = newFolder / entry.path().filename();
      logMove(entry.path(), newPath);

      fs::rename(entry.path(), newPath);
    }
  }
}

// Function to undo the last operation
void undoLastOperation()
{
  std::ifstream logFile("undo.log");
  if (!logFile)
  {
    std::cout << "No undo history found!" << std::endl;
    return;
  }

  std::vector<std::pair<fs::path, fs::path>> moves;
  std::string oldPathStr, newPathStr;

  while (std::getline(logFile, oldPathStr, '\t') && std::getline(logFile, newPathStr))
  {
    moves.push_back({oldPathStr, newPathStr});
  }
  logFile.close();

  if (moves.empty())
  {
    std::cout << "Nothing to undo!" << std::endl;
    return;
  }

  bool success = true;

  // Move files back to original locations
  for (auto it = moves.rbegin(); it != moves.rend(); ++it)
  {
    const fs::path &oldPath = it->first;
    const fs::path &newPath = it->second;

    if (fs::exists(newPath))
    {
      try
      {
        fs::rename(newPath, oldPath);
      }
      catch (const fs::filesystem_error &e)
      {
        std::cerr << "Failed to restore: " << newPath << " -> " << oldPath
                  << "\nError: " << e.what() << std::endl;
        success = false;
      }
    }
    else
    {
      std::cerr << "File missing: " << newPath << " (Undo failed for this file)" << std::endl;
      success = false;
    }
  }

  // Clear the log file only if all undo operations were successful
  if (success)
  {
    std::ofstream clearLog("undo.log", std::ios::trunc);
    std::cout << "Undo completed successfully. Files restored." << std::endl;
  }
  else
  {
    std::cerr << "Undo completed with errors. Log file not cleared." << std::endl;
  }
}

int main()
{
  std::string path;
  int choice;

  std::cout << "Enter the folder path to organize: ";
  std::cin >> path;

  std::cout << "Choose an option:\n";
  std::cout << "1. Organize by File Type\n";
  std::cout << "2. Organize by Date\n";
  std::cout << "3. Undo Last Operation\n";
  std::cin >> choice;

  if (choice == 1)
    organizeFiles(path);
  else if (choice == 2)
    sortByDate(path);
  else if (choice == 3)
    undoLastOperation();
  else
    std::cout << "Invalid choice!" << std::endl;

  std::cout << "Operation completed!" << std::endl;
  return 0;
}
