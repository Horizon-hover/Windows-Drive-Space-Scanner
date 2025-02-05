/*
MIT License

Copyright (c) 2024 Paul Begg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <format> // C++20
#include <fstream>
#include <limits>

namespace fs = std::filesystem;

// Function to convert bytes to a more readable format
std::string formatSize(ULONGLONG size) {
    const char* suffix[] = { "B", "KB", "MB", "GB", "TB" };
    int i = 0;
    double dblByte = static_cast<double>(size);

    while (size >= 1024 && i < 4) {
        dblByte = size / 1024.0;
        size /= 1024;
        i++;
    }

    return std::format("{:.2f} {}", dblByte, suffix[i]);
}

// Function to calculate the size of a directory
ULONGLONG calculateDirectorySize(const fs::path& path) {
    ULONGLONG totalSize = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing " << path << ": " << e.what() << std::endl;
    }
    return totalSize;
}

// Function to display directory sizes with progress indicator
void displayDirectorySizes(const fs::path& path) {
    try {
        std::vector<std::pair<std::string, ULONGLONG>> directorySizes;
        int totalDirs = 0, processedDirs = 0;

        // Count total directories
        for (const auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            if (fs::is_directory(entry.status())) {
                totalDirs++;
            }
        }

        // Process directories
        for (const auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            if (fs::is_directory(entry.status())) {
                ULONGLONG dirSize = calculateDirectorySize(entry.path());
                directorySizes.push_back({ entry.path().string(), dirSize });
                processedDirs++;
                std::cout << "\rProcessing: " << processedDirs << "/" << totalDirs << " directories...";
            }
        }

        std::sort(directorySizes.begin(), directorySizes.end(), [](const auto& a, const auto& b) {
            return b.second < a.second; // Sort in descending order
        });

        std::cout << "\n";
        for (const auto& dir : directorySizes) {
            std::cout << dir.first << ": " << formatSize(dir.second) << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing " << path << ": " << e.what() << std::endl;
    }
}

// Function to scan a specific drive
void scanDrive(const std::string& driveName) {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    std::cout << "Checking drive: " << driveName << std::endl;

    if (GetDiskFreeSpaceEx(driveName.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        std::cout << "\nDrive: " << driveName << std::endl;
        std::cout << "Total Space: " << formatSize(totalNumberOfBytes.QuadPart) << std::endl;
        std::cout << "Free Space: " << formatSize(totalNumberOfFreeBytes.QuadPart) << std::endl;
        std::cout << "Used Space: " << formatSize(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) << std::endl;
        std::cout << std::endl;

        ULONGLONG threshold = 50ULL * 1024 * 1024 * 1024; // 50GB
        if (totalNumberOfBytes.QuadPart > threshold) {
            std::cout << "\nPlease wait while the program scans your directories. This may take a while...\n" << std::endl;
        }

        // Display directory sizes for the drive
        displayDirectorySizes(driveName);
    } else {
        DWORD error = GetLastError();
        std::cerr << "Error getting disk free space for drive " << driveName << ": " << error << std::endl;
        switch (error) {
            case ERROR_ACCESS_DENIED:
                std::cerr << "Access denied. Try running the program as an administrator." << std::endl;
                break;
            case ERROR_NOT_READY:
                std::cerr << "The drive is not ready." << std::endl;
                break;
            default:
                std::cerr << "Unknown error." << std::endl;
                break;
        }
    }
}

// Function to scan the main drive
void scanMainDrive() {
    scanDrive("C:\\");
}

// Function to scan attached drives
void scanAttachedDrives() {
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) {
        std::cerr << "Error getting logical drives: " << GetLastError() << std::endl;
        return;
    }

    bool foundDrive = false;
    for (char drive = 'D'; drive <= 'Z'; drive++) {
        if (driveMask & (1 << (drive - 'A'))) {
            foundDrive = true;
            std::string driveName = std::string(1, drive) + ":\\";
            scanDrive(driveName);
        }
    }

    if (!foundDrive) {
        std::cout << "No attached drives found." << std::endl;
    }
}

// Function to scan all drives and display a summary
void scanAllDrives() {
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) {
        std::cerr << "Error getting logical drives: " << GetLastError() << std::endl;
        return;
    }

    ULONGLONG totalUsedSpace = 0;
    for (char drive = 'A'; drive <= 'Z'; drive++) {
        if (driveMask & (1 << (drive - 'A'))) {
            std::string driveName = std::string(1, drive) + ":\\";
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            if (GetDiskFreeSpaceEx(driveName.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                totalUsedSpace += (totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart);
                scanDrive(driveName);
            }
        }
    }

    std::cout << "\nTotal used space across all drives: " << formatSize(totalUsedSpace) << std::endl;
}

// Function to display all drives
void showAllDrives() {
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) {
        std::cerr << "Error getting logical drives: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "\nCurrent drives on the system:\n";
    for (char drive = 'A'; drive <= 'Z'; drive++) {
        if (driveMask & (1 << (drive - 'A'))) {
            std::cout << drive << ":\\\n";
        }
    }
    std::cout << std::endl;
}

// Function to validate user input
int getValidatedInput() {
    int choice;
    while (true) {
        std::cin >> choice;
        if (std::cin.fail() || choice < 1 || choice > 5) {
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "Invalid input. Please enter a number between 1 and 5: ";
        } else {
            break;
        }
    }
    return choice;
}

// Main function
int main() {
    while (true) {
        std::cout << "\nSelect an option:\n";
        std::cout << "1. Scan main drive (C:\\)\n";
        std::cout << "2. Scan attached drives\n";
        std::cout << "3. Scan all drives\n";
        std::cout << "4. Show all drives\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        int choice = getValidatedInput();

        switch (choice) {
            case 1:
                scanMainDrive();
                break;
            case 2:
                std::cout << "Please wait while the program scans attached drives." << std::endl;
                scanAttachedDrives();
                break;
            case 3:
                std::cout << "Please wait while the program scans all drives." << std::endl;
                scanAllDrives();
                break;
            case 4:
                showAllDrives();
                break;
            case 5:
                std::cout << "Exiting program." << std::endl;
                return 0;
        }
    }
}
















