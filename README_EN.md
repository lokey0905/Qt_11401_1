# Desktop Widget Suite

![Project Status](https://img.shields.io/badge/status-active-brightgreen) ![Qt Version](https://img.shields.io/badge/Qt-5.15%2B-blue) ![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

## 📖 Overview

**Desktop Widget Suite** is a high-performance desktop information monitoring and utility collection developed with Qt/C++. This project aims to provide a lightweight, modular, and highly customizable desktop experience similar to Rainmeter.

Through an intuitive **Control Panel**, users can freely enable, arrange, and configure various desktop widgets, ranging from system performance monitoring to productivity tools. All widgets support frameless design, transparent backgrounds, and rich interactive behaviors.

## ✨ Key Features

### 🖥️ System Monitoring
*   **CPU & RAM Monitor**: Real-time display of processor load and memory usage, supporting multi-core frequency display algorithm switching.
*   **Disk Info**: Monitors free space, read/write speeds (R/W Speed), and active time for each disk partition.
*   **Network Monitor**:
    *   Real-time upload/download speed display (supports Bits/Bytes unit switching).
    *   **Multi-Interface Support**: Automatically detects all system network interfaces, allowing selection of specific interfaces or a total summary.
    *   **[NEW] Network Latency (Ping)**: Built-in background Ping detection for real-time connection quality monitoring (Green/Yellow/Red status indicators), with customizable target IP (e.g., 8.8.8.8 or game servers).

### ⏱️ Time & Productivity
*   **Digital Clock**: Minimalist design for date and time display.
*   **To-Do List**: Lightweight task management supporting completion status toggling and data persistence.
*   **[NEW] Pomodoro Timer**:
    *   Built-in standard Pomodoro technique workflow (Work / Short Break / Long Break).
    *   Customizable duration for each stage (in minutes).
    *   Visual countdown and status color hints (Red/Green/Blue).

### 🛠️ Utilities
*   **[NEW] Clipboard History**:
    *   Automatically records recently copied text and images.
    *   **Smart Deduplication**: Uses MD5 hashing for images to avoid duplicate entries taking up space.
    *   **One-Click Restore**: Click on a history item to copy it back to the system clipboard.
    *   **History Limit**: Customizable history limit (default: last 30 entries).
*   **Media Viewer**: Supports image and GIF playback, serving as desktop decoration or dynamic wallpaper elements.
*   **Music Player**: Supports MP3/WAV playback, providing play/pause/volume controls and a progress bar, with automatic scanning of specified music folders.

### 🎨 High Customization
*   **Global Control Panel**: Unified management of all widget switches, positions, and detailed settings.
*   **Window Behavior Settings**:
    *   **Lock Drag**: Prevents accidental movement of widgets.
    *   **Click-through**: Makes widgets part of the background, allowing mouse clicks to pass through to windows behind.
    *   **Hover Effects**: Supports auto-hide or transparency changes on mouse hover.
    *   **Layering**: Configurable as Always on Top or On Desktop.
*   **Theme Manager**: Supports saving and loading different desktop layout settings (Layouts) for easy switching between scenarios.

## 🚀 Installation & Build

### Prerequisites
*   **OS**: Windows 10 / 11
*   **Framework**: Qt 5.15+ or Qt 6.x
*   **Compiler**: MinGW 8.1+ or MSVC 2019+ (C++17 Support)

### Build Steps
1.  Clone the repository:
    ```bash
    git clone https://github.com/hi-ma-san/Qt_11401_1
    ```
2.  Open `Qt_11401_1.pro` with **Qt Creator**.
3.  Configure the Build Kit and run **qmake**.
4.  Click **Build** (Hammer icon) or **Run** (Green Play button) to execute.

## ⚙️ Usage

1.  **Launch**: Upon startup, the **Control Panel** will open automatically.
2.  **Enable Widgets**: Select a widget from the list on the left (e.g., Network, Pomodoro) and check **"Enable Tool"** to display it on the desktop.
3.  **Adjust Settings**:
    *   **General Settings**: Adjust transparency, coordinates, scale, etc.
    *   **Advanced Settings**: The bottom right panel provides specific settings for the tool (e.g., Ping target IP, Pomodoro work duration, Clipboard history limit).
4.  **Global Control**: Use the **"Global Settings"** at the bottom of the Control Panel to adjust behavior for all tools at once (e.g., Lock Drag, Auto-start).
5.  **Background Execution**: When the Control Panel window is closed, the program minimizes to the **System Tray**. Click the tray icon to reopen the Control Panel.

## 📂 Project Structure

```
Qt_11401_1/
├── Core/               # Core Architecture
│   ├── BaseComponent   # Base class for all widgets (provides drag, context menu, settings interface)
│   └── SettingsManager # Global settings management (Singleton)
├── Widgets/            # Widget Implementations
│   ├── CpuWidget       # CPU/RAM Monitor
│   ├── DiskWidget      # Disk Monitor
│   ├── NetworkWidget   # Network Traffic & Ping Monitor
│   ├── PomodoroWidget  # Pomodoro Timer
│   ├── ClipboardWidget # Clipboard History
│   └── ...
├── ControlPanel        # Main Control Interface & Logic
├── ToolSettingsForm    # Generic Settings Form Interface
└── config/             # Configuration Files & Resources
```

## 📝 License
This project is developed for the "Windows Programming" course final project.
