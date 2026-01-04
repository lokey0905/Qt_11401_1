# Desktop Widget Suite (桌面實用工具組)

![Project Status](https://img.shields.io/badge/status-active-brightgreen) ![Qt Version](https://img.shields.io/badge/Qt-5.15%2B-blue) ![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

[🇺🇸 English](README_EN.md) | [🇹🇼 繁體中文](README.md)

## 📖 專案簡介 (Overview)

**Desktop Widget Suite** 是一個基於 Qt/C++ 開發的高效能桌面資訊監控與實用工具集合。本專案旨在提供類似 Rainmeter 的輕量級、模組化且高度可客製化的桌面體驗。

透過直覺的 **控制面板 (Control Panel)**，使用者可以自由啟用、排列並設定各種桌面小工具，從系統效能監控到生產力工具一應俱全。所有小工具皆支援無邊框設計、透明背景以及豐富的互動行為。

## ✨ 核心功能 (Key Features)

### 🖥️ 系統監控 (System Monitoring)
*   **CPU & RAM 監控**：即時顯示處理器負載與記憶體使用量，支援多核心頻率顯示演算法切換。
*   **硬碟資訊 (Disk Info)**：監控各個磁碟分區的剩餘空間、讀寫速度 (R/W Speed) 與活動時間 (Active Time)。
*   **網路流量 (Network Monitor)**：
    *   即時上傳/下載速度顯示 (支援 Bits/Bytes 單位切換)。
    *   **多網卡支援**：自動偵測系統所有網路介面，可選擇特定介面或加總顯示 (Total)。
    *   **[NEW] 網路延遲檢測 (Ping)**：內建背景 Ping 檢測，即時監控連線品質 (綠/黃/紅 狀態指示)，可自訂目標 IP (如 8.8.8.8 或遊戲伺服器)。

### ⏱️ 時間與生產力 (Time & Productivity)
*   **數位時鐘 (Digital Clock)**：簡約設計的日期與時間顯示。
*   **待辦清單 (To-Do List)**：輕量級待辦事項管理，支援完成狀態切換與資料保存。
*   **[NEW] 番茄鐘 (Pomodoro Timer)**：
    *   內建標準番茄工作法流程 (工作 Work / 短休息 Short Break / 長休息 Long Break)。
    *   可自訂各階段時間長度 (分鐘)。
    *   視覺化倒數計時與狀態顏色提示 (紅/綠/藍)。

### 🛠️ 實用工具 (Utilities)
*   **[NEW] 剪貼簿歷史 (Clipboard History)**：
    *   自動記錄最近複製的文字與圖片。
    *   **智慧去重**：圖片採用 MD5 雜湊比對，避免重複圖片佔用紀錄空間。
    *   **一鍵還原**：點擊歷史清單項目即可重新複製到系統剪貼簿。
    *   **數量限制**：可自訂歷史紀錄保存數量 (History Limit)，預設保存近 30 筆。
*   **多媒體顯示 (Media Viewer)**：支援圖片與 GIF 動圖播放，可作為桌面裝飾或動態桌布元件。
*   **音樂播放器 (Music Player)**：支援 MP3/WAV 播放，提供播放/暫停/音量控制與進度條，可指定音樂資料夾自動掃描播放。

### 🎨 高度客製化 (Customization)
*   **全域控制面板**：統一管理所有小工具的開關、位置與細部設定。
*   **視窗行為設定**：
    *   **拖曳鎖定 (Lock Drag)**：防止誤觸移動小工具。
    *   **滑鼠穿透 (Click-through)**：讓小工具成為背景，滑鼠點擊可直接穿透至後方視窗。
    *   **懸停互動 (Hover Effects)**：支援滑鼠懸停時自動隱藏或改變透明度。
    *   **層級控制 (Layering)**：可設定置頂 (Always on Top) 或貼底 (On Desktop)。
*   **主題管理 (Theme Manager)**：支援儲存與載入不同的桌面佈局設定 (Layouts)，方便在不同情境下切換。

## 🚀 安裝與編譯 (Installation & Build)

### 系統需求 (Prerequisites)
*   **OS**: Windows 10 / 11
*   **Framework**: Qt 5.15+ 或 Qt 6.x
*   **Compiler**: MinGW 8.1+ 或 MSVC 2019+ (C++17 Support)

### 編譯步驟 (Build Steps)
1.  複製專案庫：
    ```bash
    git clone https://github.com/hi-ma-san/Qt_11401_1
    ```
2.  使用 **Qt Creator** 開啟 `Qt_11401_1.pro` 專案檔。
3.  設定建置套件 (Kit) 並執行 **qmake**。
4.  點擊 **Build** (錘子圖示) 或 **Run** (綠色播放鍵) 即可執行。

## ⚙️ 使用說明 (Usage)

1.  **啟動程式**：程式啟動後，**控制面板 (Control Panel)** 會自動開啟。
2.  **啟用工具**：在左側清單選擇欲設定的小工具 (如 Network, Pomodoro)，勾選 **「啟用此工具」** 即可在桌面顯示。
3.  **調整設定**：
    *   **通用設定**：調整透明度、座標、縮放比例等。
    *   **進階設定**：右側面板下方提供該工具的專屬設定 (如：Ping 目標 IP、番茄鐘工作時間、剪貼簿數量限制等)。
4.  **全域控制**：透過控制面板下方的 **「全域設定」** 可一次調整所有工具的行為 (如鎖定拖曳、開機自動啟動)。
5.  **背景執行**：關閉控制面板視窗時，程式會自動縮小至 **系統托盤 (System Tray)** 常駐執行。點擊托盤圖示可再次開啟控制面板。

## 📂 專案結構 (Project Structure)

```
Qt_11401_1/
├── Core/               # 核心架構
│   ├── BaseComponent   # 所有小工具的基礎類別 (提供拖曳、右鍵選單、設定介面接口)
│   └── SettingsManager # 全域設定管理 (Singleton)
├── Widgets/            # 各式小工具實作
│   ├── CpuWidget       # CPU/RAM 監控
│   ├── DiskWidget      # 硬碟監控
│   ├── NetworkWidget   # 網路流量與 Ping 監控
│   ├── PomodoroWidget  # 番茄鐘
│   ├── ClipboardWidget # 剪貼簿歷史
│   └── ...
├── ControlPanel        # 主控台介面與邏輯
├── ToolSettingsForm    # 通用設定表單介面
└── config/             # 設定檔與資源
```

## 📝 License
This project is developed for the "Windows Programming" course final project.