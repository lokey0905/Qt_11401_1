#include "SettingsManager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

SettingsManager* SettingsManager::m_instance = nullptr;
QMutex SettingsManager::m_mutex;

SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);

    // 啟動時自動載入之前存過的數值
    m_globalDragLocked = m_settings->value("Global/DragLocked", false).toBool();
    m_showTrayIcon = m_settings->value("Global/ShowTrayIcon", true).toBool();
    m_lastTheme = m_settings->value("Global/LastTheme", "").toString();
}

SettingsManager* SettingsManager::instance() {
    if (!m_instance) {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new SettingsManager();
        }
    }
    return m_instance;
}

/** --- 開機啟動邏輯 捷徑--- **/

bool SettingsManager::isAutoStart() const {
    // 取得 Windows 使用者啟動資料夾下的 .lnk 路徑
    QString startupPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)
                          + "/Startup/Qt_11401_1.lnk";
    return QFile::exists(startupPath);
}

void SettingsManager::setAutoStart(bool enable) {
    QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup/";
    QString shortcutPath = startupDir + "Qt_11401_1.lnk";
    QString appPath = QCoreApplication::applicationFilePath();

    if (enable) {
        if (!QFile::exists(shortcutPath)) {
            // 建立指向 Qt_11401_1.exe 的捷徑
            QFile::link(appPath, shortcutPath);
            qDebug() << "SettingsManager: 已建立啟動捷徑" << shortcutPath;
        }
    } else {
        if (QFile::exists(shortcutPath)) {
            QFile::remove(shortcutPath);
            qDebug() << "SettingsManager: 已移除啟動捷徑";
        }
    }
}

/** --- 全域設定套用 --- **/

void SettingsManager::setGlobalDragLocked(bool locked) {
    if (m_globalDragLocked != locked) {
        m_globalDragLocked = locked;
        m_settings->setValue("Global/DragLocked", locked);
        emit globalDragLockedChanged(locked);
    }
}

void SettingsManager::setShowTrayIcon(bool show) {
    if (m_showTrayIcon != show) {
        m_showTrayIcon = show;
        m_settings->setValue("Global/ShowTrayIcon", show);

        // 發出訊號通知主視窗
        emit trayIconSettingChanged(show);

        qDebug() << "SettingsManager: 托盤圖示狀態更新為 ->" << show;
    }
}

void SettingsManager::setLastTheme(const QString &themeName) {
    if (m_lastTheme != themeName) {
        m_lastTheme = themeName;
        m_settings->setValue("Global/LastTheme", themeName);
        m_settings->sync(); // 確保立即寫入磁碟喵
    }
}
