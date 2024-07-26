# Windows Drive Space Scanner

## Overview

The Windows Drive Space Scanner is a C++ program that scans your Windows system drives and displays the storage space usage in a readable format. It can scan the main drive, attached drives, or all drives and provide detailed information about the used and free space, as well as the sizes of directories on each drive.

## Features

- Scan the main drive (C:\)
- Scan all attached drives (D:\ to Z:\)
- Scan all drives (A:\ to Z:\)
- Show all current drives on the system
- Display detailed storage space usage for directories
- User-friendly menu for selecting options

## Usage

1. Compile the program using a C++ compiler that supports C++17.
2. Run the compiled executable.
3. Select an option from the menu to scan the main drive, attached drives, all drives, or show all current drives.
4. The program will display the storage space usage and directory sizes for the selected drives.

## Example Output

Select an option:

Scan main drive (C:)
Scan attached drives
Scan all drives
Show all drives
Exit
---
Enter your choice: 1
Checking drive: C:
Drive: C:
Total Space: 930.45 GB
Free Space: 338.02 GB
Used Space: 592.43 GB
---
C:\Windows: 20.03 GB
C:\Program Files: 15.47 GB
C:\Users: 25.67 GB


## Limitations

- The program requires administrative privileges to access certain directories and drives.
- The program may encounter permission issues and fail to access certain directories, displaying an error message.
- The program uses the `std::filesystem` library which requires C++17 or later.
- The program does not support scanning network drives or remote servers.
- The program assumes drives beyond C:\ are attached drives and scans from D:\ to Z:\.
- The program may take a long time to scan drives with a large number of files and directories.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Author

Paul Begg

## Acknowledgements

- The program uses the `std::filesystem` library for directory and file handling.
- The program uses the Windows API for retrieving drive information.
