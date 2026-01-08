# Running Tracker

A modern C++ application for tracking daily running kilometers with a Qt-based user interface.

## Features

- ✅ Add daily running distances in kilometers
- ✅ Automatic date tracking
- ✅ View running history in a clean list format
- ✅ Real-time statistics (total distance, average, entry count)
- ✅ Clear all entries with confirmation
- ✅ Modern C++17 implementation with Qt5

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake qtbase5-dev
```

### Fedora
```bash
sudo dnf install gcc-c++ cmake qt5-qtbase-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake qt5-base
```

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make

# Run
./running_tracker
```

## Usage

1. Enter the distance you ran in kilometers (e.g., 5.5)
2. Click "Add Entry" or press Enter
3. View your running history and statistics
4. Use "Clear All" to reset your data (with confirmation)

## Project Structure

```
running_ui/
├── CMakeLists.txt
├── README.md
├── include/
│   └── MainWindow.h
└── src/
    ├── main.cpp
    └── MainWindow.cpp
```

## License

MIT License
