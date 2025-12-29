#include "SettingsManager.h"
#include <QDebug>

SettingsManager* SettingsManager::m_instance = nullptr;
QMutex SettingsManager::m_mutex;

SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {
    // 初始化 QSettings，將設定存在程式執行目錄下的 config.ini 喵
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);

    // 啟動時自動載入之前存過的數值
    m_globalDragLocked = m_settings->value("Global/DragLocked", false).toBool();
    m_showTrayIcon = m_settings->value("Global/ShowTrayIcon", true).toBool();
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

void SettingsManager::setGlobalDragLocked(bool locked) {
    if (m_globalDragLocked != locked) {
        m_globalDragLocked = locked;
        // 同步寫入檔案喵！
        m_settings->setValue("Global/DragLocked", locked);
        emit globalDragLockedChanged(locked);
        qDebug() << "SettingsManager: 全域拖曳鎖定已儲存 ->" << locked;
    }
}

void SettingsManager::setShowTrayIcon(bool show) {
    if (m_showTrayIcon != show) {
        m_showTrayIcon = show;
        m_settings->setValue("Global/ShowTrayIcon", show);
        emit trayIconSettingChanged(show);
        qDebug() << "SettingsManager: 托盤顯示設定已儲存 ->" << show;
    }
}
