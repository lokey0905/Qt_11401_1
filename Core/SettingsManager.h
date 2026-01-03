#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QVariantMap>

/**
 * @brief 設定管理中心 (單例模式)
 * 負責管理全域設定、開機啟動捷徑以及各小工具的持久化儲存
 */
class SettingsManager : public QObject
{
    Q_OBJECT
public:
    static SettingsManager* instance();

    /** --- 全域設定 (Global Settings) --- **/
    bool isGlobalDragLocked() const { return m_globalDragLocked; }
    void setGlobalDragLocked(bool locked);

    bool showTrayIcon() const { return m_showTrayIcon; }
    void setShowTrayIcon(bool show);

    // 開機啟動 (捷徑方案)
    bool isAutoStart() const;
    void setAutoStart(bool enable);

    /** --- 小工具個別設定 (Widget-Specific Settings) --- **/
    void saveWidgetConfig(const QString &id, const QVariantMap &config);
    QVariantMap loadWidgetConfig(const QString &id);

signals:
    void globalDragLockedChanged(bool locked);
    void trayIconSettingChanged(bool show);

private:
    explicit SettingsManager(QObject *parent = nullptr);
    static SettingsManager* m_instance;
    static QMutex m_mutex;

    bool m_globalDragLocked = false;
    bool m_showTrayIcon = true;
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
