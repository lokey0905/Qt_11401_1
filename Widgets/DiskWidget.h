#ifndef DISKWIDGET_H
#define DISKWIDGET_H

#include "Core/BaseComponent.h"
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QStorageInfo>
#include <QMap>
#include <QProgressBar>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#endif

class DiskWidget : public BaseComponent {
    Q_OBJECT
public:
    explicit DiskWidget(QWidget *parent = nullptr);
    ~DiskWidget();

    void initStyle() override;
    void updateData() override;
    void setUpdateInterval(int ms) override;
    void setCustomSetting(const QString &key, const QVariant &value) override;

    bool isShowUsagePercent() const { return m_showUsagePercent; }
    bool isShowTransferSpeed() const { return m_showTransferSpeed; }
    bool isShowActiveTime() const { return m_showActiveTime; }

private:
    QLabel *m_titleLabel;
    QWidget *m_diskContainer;
    QVBoxLayout *m_diskLayout;
    QTimer *m_updateTimer;

    bool m_showUsagePercent = false; // 空間使用率文字
    bool m_showTransferSpeed = false;
    bool m_showActiveTime = false;   // 新增：硬碟活動時間 (Active Time)

    // 用於快取每個硬碟的 UI 元件，避免每次重建
    struct DiskUI {
        QLabel *nameLabel;
        QProgressBar *usageBar;
        QLabel *detailLabel;
        QLabel *speedLabel; // 讀寫速度 + 活動時間
        QWidget *container;
    };
    
    // Key: Root Path (e.g., "C:/")
    QMap<QString, DiskUI> m_diskUIs;

    void refreshDiskList();

#ifdef Q_OS_WIN
    PDH_HQUERY m_pdhDiskQuery = NULL;
    // Key: Drive Letter (e.g., "C:") -> Counter
    QMap<QString, PDH_HCOUNTER> m_readCounters;
    QMap<QString, PDH_HCOUNTER> m_writeCounters;
    QMap<QString, PDH_HCOUNTER> m_idleCounters; // 新增：用於計算 Active Time
    
    void initPdh();
    void addDiskCounter(const QString &driveLetter);
    void removeDiskCounter(const QString &driveLetter);
#endif
};

#endif // DISKWIDGET_H
