#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>  // 核心：負責讀寫 .ini 設定檔
#include <QVariantMap>

/**
 * @brief 設定管理中心 (單例模式)
 * 負責管理全域設定以及各個小工具的持久化儲存
 */
class SettingsManager : public QObject
{
    Q_OBJECT
public:
    // 獲取唯一的管理實例
    static SettingsManager* instance();

    // --- 1. 全域設定 (Global Settings) ---
    bool isGlobalDragLocked() const { return m_globalDragLocked; }
    void setGlobalDragLocked(bool locked);

    bool showTrayIcon() const { return m_showTrayIcon; }
    void setShowTrayIcon(bool show);

    // --- 2. 小工具個別設定 (Widget-Specific Settings) ---
    /**
     * @brief 儲存特定工具的所有設定 (座標、層級、透明度等)
     * @param id 工具的唯一識別碼 (如 "time_info")
     * @param config 包含所有設定項的 Map
     */
    void saveWidgetConfig(const QString &id, const QVariantMap &config);

    /**
     * @brief 讀取特定工具的設定
     * @param id 工具的唯一識別碼
     * @return 儲存的設定項 Map，若無資料則回傳空 Map
     */
    QVariantMap loadWidgetConfig(const QString &id);

signals:
    // 當全域設定改變時發出，通知所有元件即時反應
    void globalDragLockedChanged(bool locked);
    void trayIconSettingChanged(bool show);

private:
    // 私有建構子，確保單例模式
    explicit SettingsManager(QObject *parent = nullptr);
    static SettingsManager* m_instance;
    static QMutex m_mutex;

    // 記憶體中的快取狀態
    bool m_globalDragLocked = false;
    bool m_showTrayIcon = true;

    // 負責檔案操作的實例
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
