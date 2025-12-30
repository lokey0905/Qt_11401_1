#include "DiskWidget.h"
#include <QDebug>
#include <QFileInfo>

DiskWidget::DiskWidget(QWidget *parent) : BaseComponent(parent) {
    m_titleLabel = new QLabel("DISK INFO", this);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(5);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    m_diskContainer = new QWidget(this);
    m_diskLayout = new QVBoxLayout(m_diskContainer);
    m_diskLayout->setContentsMargins(0, 5, 0, 0);
    m_diskLayout->setSpacing(8);
    mainLayout->addWidget(m_diskContainer);

    // 定時器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &DiskWidget::updateData);
    m_updateTimer->start(2000); // 硬碟資訊不用更新太快，預設 2秒

#ifdef Q_OS_WIN
    initPdh();
#endif

    initStyle();
    updateData();
}

DiskWidget::~DiskWidget() {
#ifdef Q_OS_WIN
    if (m_pdhDiskQuery) {
        PdhCloseQuery(m_pdhDiskQuery);
    }
#endif
}

void DiskWidget::setCustomSetting(const QString &key, const QVariant &value) {
    if (key == "showUsagePercent") {
        m_showUsagePercent = value.toBool();
        // 立即更新 UI 顯示狀態
        for (auto &ui : m_diskUIs) {
            ui.usageBar->setTextVisible(m_showUsagePercent);
            // 調整高度以容納文字
            ui.usageBar->setFixedHeight(m_showUsagePercent ? 14 : 4);
            ui.usageBar->setStyleSheet(ui.usageBar->styleSheet().replace(
                m_showUsagePercent ? "color: transparent;" : "color: white;", 
                m_showUsagePercent ? "color: white;" : "color: transparent;"
            ));
        }
        this->adjustSize();
    } else if (key == "showTransferSpeed") {
        m_showTransferSpeed = value.toBool();
        for (auto &ui : m_diskUIs) {
            if (ui.speedLabel) ui.speedLabel->setVisible(m_showTransferSpeed || m_showActiveTime);
        }
        this->adjustSize();
    } else if (key == "showActiveTime") {
        m_showActiveTime = value.toBool();
        for (auto &ui : m_diskUIs) {
            // 只要其中一個開啟，就顯示 Label
            if (ui.speedLabel) ui.speedLabel->setVisible(m_showTransferSpeed || m_showActiveTime);
        }
        this->adjustSize();
    }
}

void DiskWidget::initStyle() {
    BaseComponent::initStyle();

    this->setStyleSheet(this->styleSheet() +
                        "DiskWidget {"
                        "  background-color: rgba(0, 0, 0, 150);"
                        "  border: 1px solid rgba(255, 255, 255, 30);"
                        "  border-radius: 8px;"
                        "}"
                        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
                        "#titleLabel { font-size: 14px; font-weight: bold; color: rgba(255, 255, 255, 220); margin-bottom: 2px; }"
                        "QProgressBar {"
                        "  border: 1px solid rgba(255, 255, 255, 50);"
                        "  border-radius: 2px;"
                        "  background-color: rgba(0, 0, 0, 100);"
                        "  text-align: center;"
                        "  color: transparent;" /* 隱藏進度條文字 */
                        "  height: 4px;"
                        "  font-size: 10px;"
                        "}"
                        "QProgressBar::chunk {"
                        "  background-color: rgba(0, 120, 215, 200);"
                        "  border-radius: 2px;"
                        "}"
                        );

    m_titleLabel->setObjectName("titleLabel");
}

void DiskWidget::setUpdateInterval(int ms) {
    BaseComponent::setUpdateInterval(ms);
    if (m_updateTimer) {
        // 硬碟資訊更新頻率可以比系統監控慢一點，這裡做個簡單的調整
        // 如果使用者設定極速(100ms)，硬碟可能不需要那麼快，最低限制在 500ms 避免 I/O 頻繁
        m_updateTimer->setInterval(qMax(500, ms));
    }
}

void DiskWidget::updateData() {
#ifdef Q_OS_WIN
    if (m_pdhDiskQuery) {
        PdhCollectQueryData(m_pdhDiskQuery);
    }
#endif

    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    
    // 標記現有的硬碟，用於檢測移除
    QList<QString> currentPaths = m_diskUIs.keys();
    QList<QString> newPaths;

    for (const QStorageInfo &storage : volumes) {
        if (storage.isValid() && storage.isReady()) {
            // 排除唯讀或系統保留 (簡單過濾)
            // 通常我們只關心 C:, D: 等固定磁碟
            // 注意：QStorageInfo 在 Windows 上 rootPath 就是 "C:/"
            
            QString path = storage.rootPath();
            newPaths.append(path);

            // 先取得效能數據 (為了顯示在標題或速度欄)
            double readSpeed = 0.0;
            double writeSpeed = 0.0;
            double activeTime = 0.0;

#ifdef Q_OS_WIN
            if (m_showTransferSpeed || m_showActiveTime) {
                QString driveLetter = path.left(2);

                if (m_readCounters.contains(driveLetter)) {
                    PDH_FMT_COUNTERVALUE value;
                    if (PdhGetFormattedCounterValue(m_readCounters[driveLetter], PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                        readSpeed = value.doubleValue;
                    }
                }
                if (m_writeCounters.contains(driveLetter)) {
                    PDH_FMT_COUNTERVALUE value;
                    if (PdhGetFormattedCounterValue(m_writeCounters[driveLetter], PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                        writeSpeed = value.doubleValue;
                    }
                }
                if (m_idleCounters.contains(driveLetter)) {
                    PDH_FMT_COUNTERVALUE value;
                    if (PdhGetFormattedCounterValue(m_idleCounters[driveLetter], PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                        // Active Time = 100 - Idle Time
                        activeTime = 100.0 - value.doubleValue;
                        if (activeTime < 0) activeTime = 0;
                        if (activeTime > 100) activeTime = 100;
                    }
                }
            }
#endif

            double totalGB = storage.bytesTotal() / (1024.0 * 1024.0 * 1024.0);
            double freeGB = storage.bytesAvailable() / (1024.0 * 1024.0 * 1024.0);
            double usedGB = totalGB - freeGB;
            int usagePercent = (totalGB > 0) ? (int)((usedGB / totalGB) * 100) : 0;

            // 如果是新硬碟，建立 UI
            if (!m_diskUIs.contains(path)) {
                DiskUI ui;
                ui.container = new QWidget(m_diskContainer);
                QVBoxLayout *vLayout = new QVBoxLayout(ui.container);
                vLayout->setContentsMargins(0, 0, 0, 0);
                vLayout->setSpacing(2);

                // 第一行：名稱與路徑 (現在也包含 Active Time)
                ui.nameLabel = new QLabel(ui.container);
                ui.nameLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: rgba(255, 255, 255, 200);");
                
                // 第二行：進度條
                ui.usageBar = new QProgressBar(ui.container);
                ui.usageBar->setRange(0, 100);
                ui.usageBar->setTextVisible(m_showUsagePercent);
                ui.usageBar->setFixedHeight(m_showUsagePercent ? 14 : 4);
                if (m_showUsagePercent) {
                     ui.usageBar->setStyleSheet(ui.usageBar->styleSheet() + "color: white;");
                }

                // 第三行：詳細數值
                ui.detailLabel = new QLabel(ui.container);
                ui.detailLabel->setStyleSheet("font-size: 10px; color: rgba(255, 255, 255, 150);");

                // 第四行：讀寫速度 (預設隱藏)
                ui.speedLabel = new QLabel(ui.container);
                ui.speedLabel->setStyleSheet("font-size: 10px; color: rgba(100, 200, 255, 180);");
                ui.speedLabel->setVisible(m_showTransferSpeed);

                vLayout->addWidget(ui.nameLabel);
                vLayout->addWidget(ui.usageBar);
                vLayout->addWidget(ui.detailLabel);
                vLayout->addWidget(ui.speedLabel);

                m_diskLayout->addWidget(ui.container);
                m_diskUIs.insert(path, ui);

#ifdef Q_OS_WIN
                // 為新硬碟加入 Pdh Counter
                // path 格式為 "C:/"，我們需要 "C:"
                QString driveLetter = path.left(2);
                addDiskCounter(driveLetter);
#endif
            }

            // 更新 UI 內容
            DiskUI &ui = m_diskUIs[path];
            
            QString label = storage.displayName();
            if (label.isEmpty()) label = "Local Disk";
            
            // 更新標題：名稱 (路徑) [Active: XX%]
            QString nameText = QString("%1 (%2)").arg(label).arg(path);
            if (m_showActiveTime) {
                nameText += QString("   Active: %1%").arg(QString::number(activeTime, 'f', 0));
            }
            ui.nameLabel->setText(nameText);
            
            ui.usageBar->setValue(usagePercent);
            
            // 根據使用率改變顏色
            QString chunkColor = "rgba(0, 120, 215, 200)"; // 藍色
            if (usagePercent > 90) chunkColor = "rgba(220, 50, 50, 200)"; // 紅色
            else if (usagePercent > 75) chunkColor = "rgba(220, 180, 50, 200)"; // 黃色
            
            // 保持原本的樣式設定，只更新顏色
            QString baseStyle = "QProgressBar { border: 1px solid rgba(255, 255, 255, 50); border-radius: 2px; background-color: rgba(0, 0, 0, 100); font-size: 10px; ";
            baseStyle += (m_showUsagePercent ? "height: 14px; color: white; }" : "height: 4px; color: transparent; }");
            baseStyle += QString("QProgressBar::chunk { background-color: %1; border-radius: 2px; }").arg(chunkColor);
            ui.usageBar->setStyleSheet(baseStyle);

            ui.detailLabel->setText(QString("%1 GB free of %2 GB").arg(QString::number(freeGB, 'f', 1)).arg(QString::number(totalGB, 'f', 1)));

            // 更新讀寫速度 (只顯示速度)
            ui.speedLabel->setVisible(m_showTransferSpeed);
            if (m_showTransferSpeed) {
                // 格式化速度 (B/s -> KB/s -> MB/s)
                auto formatSpeed = [](double bytes) -> QString {
                    if (bytes < 1024) return QString::number(bytes, 'f', 0) + " B/s";
                    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB/s";
                    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB/s";
                };
                ui.speedLabel->setText(QString("R: %1  W: %2").arg(formatSpeed(readSpeed)).arg(formatSpeed(writeSpeed)));
            }
        }
    }

    // 移除已拔除的硬碟
    for (const QString &oldPath : currentPaths) {
        if (!newPaths.contains(oldPath)) {
#ifdef Q_OS_WIN
            QString driveLetter = oldPath.left(2);
            removeDiskCounter(driveLetter);
#endif
            DiskUI ui = m_diskUIs.take(oldPath);
            delete ui.container; // 這會連帶刪除子元件
        }
    }
    
    // 調整視窗大小
    this->adjustSize();
}

#ifdef Q_OS_WIN
void DiskWidget::initPdh() {
    if (PdhOpenQuery(NULL, 0, &m_pdhDiskQuery) != ERROR_SUCCESS) {
        qWarning() << "Failed to open PDH query for DiskWidget";
        m_pdhDiskQuery = NULL;
    }
}

void DiskWidget::addDiskCounter(const QString &driveLetter) {
    if (!m_pdhDiskQuery) return;
    if (m_readCounters.contains(driveLetter)) return; // 已存在

    // LogicalDisk(C:)\Disk Read Bytes/sec
    QString readPath = QString("\\LogicalDisk(%1)\\Disk Read Bytes/sec").arg(driveLetter);
    QString writePath = QString("\\LogicalDisk(%1)\\Disk Write Bytes/sec").arg(driveLetter);
    QString idlePath = QString("\\LogicalDisk(%1)\\% Idle Time").arg(driveLetter);

    PDH_HCOUNTER hRead, hWrite, hIdle;
    
    if (PdhAddCounter(m_pdhDiskQuery, readPath.toStdWString().c_str(), 0, &hRead) == ERROR_SUCCESS) {
        m_readCounters.insert(driveLetter, hRead);
    } else {
        qWarning() << "Failed to add read counter for" << driveLetter;
    }

    if (PdhAddCounter(m_pdhDiskQuery, writePath.toStdWString().c_str(), 0, &hWrite) == ERROR_SUCCESS) {
        m_writeCounters.insert(driveLetter, hWrite);
    } else {
        qWarning() << "Failed to add write counter for" << driveLetter;
    }

    if (PdhAddCounter(m_pdhDiskQuery, idlePath.toStdWString().c_str(), 0, &hIdle) == ERROR_SUCCESS) {
        m_idleCounters.insert(driveLetter, hIdle);
    } else {
        qWarning() << "Failed to add idle counter for" << driveLetter;
    }
}

void DiskWidget::removeDiskCounter(const QString &driveLetter) {
    if (!m_pdhDiskQuery) return;

    if (m_readCounters.contains(driveLetter)) {
        PdhRemoveCounter(m_readCounters.take(driveLetter));
    }
    if (m_writeCounters.contains(driveLetter)) {
        PdhRemoveCounter(m_writeCounters.take(driveLetter));
    }
    if (m_idleCounters.contains(driveLetter)) {
        PdhRemoveCounter(m_idleCounters.take(driveLetter));
    }
}
#endif
