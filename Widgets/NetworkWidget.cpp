#include "NetworkWidget.h"
#include <QDateTime>
#include <QDebug>

NetworkWidget::NetworkWidget(QWidget *parent) : BaseComponent(parent) {
    // 設定預設大小與標題
    // this->resize(180, 90); // Remove fixed size, let it adjust
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(5);

    m_titleLabel = new QLabel("NETWORK", this);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    // Container for dynamic rows
    m_container = new QWidget(this);
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(0, 5, 0, 0);
    m_containerLayout->setSpacing(8);
    mainLayout->addWidget(m_container);

    // 初始化定時器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &NetworkWidget::updateData);
    m_updateTimer->start(1000); // 預設每秒更新一次

    initStyle();

#ifdef Q_OS_WIN
    initPdh();
#endif
}

NetworkWidget::~NetworkWidget() {
#ifdef Q_OS_WIN
    if (m_pdhQuery) {
        PdhCloseQuery(m_pdhQuery);
    }
#endif
}

void NetworkWidget::initStyle() {
    BaseComponent::initStyle();

    this->setStyleSheet(this->styleSheet() +
                        "NetworkWidget {"
                        "  background-color: rgba(0, 0, 0, 150);"
                        "  border: 1px solid rgba(255, 255, 255, 30);"
                        "  border-radius: 8px;"
                        "}"
                        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
                        "#titleLabel { font-size: 14px; font-weight: bold; color: rgba(255, 255, 255, 220); margin-bottom: 2px; }"
                        "#nameLabel { font-size: 12px; font-weight: bold; color: rgba(255, 255, 255, 200); }"
                        "#upLabel { font-size: 11px; color: #4CAF50; margin-left: 5px; }"
                        "#downLabel { font-size: 11px; color: #2196F3; margin-left: 5px; }"
                        );
    
    m_titleLabel->setObjectName("titleLabel");
}

void NetworkWidget::createInterfaceRow(const QString &name) {
    if (m_uiRows.contains(name)) return;

    QWidget *rowWidget = new QWidget(m_container);
    QVBoxLayout *rowLayout = new QVBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(2);

    QLabel *nameLabel = new QLabel(name, rowWidget);
    nameLabel->setObjectName("nameLabel");
    
    QLabel *upLabel = new QLabel("↑ 0 B/s", rowWidget);
    upLabel->setObjectName("upLabel");

    QLabel *downLabel = new QLabel("↓ 0 B/s", rowWidget);
    downLabel->setObjectName("downLabel");

    rowLayout->addWidget(nameLabel);
    rowLayout->addWidget(upLabel);
    rowLayout->addWidget(downLabel);

    m_containerLayout->addWidget(rowWidget);

    NetworkInterfaceUI ui;
    ui.rowWidget = rowWidget;
    ui.nameLabel = nameLabel;
    ui.uploadLabel = upLabel;
    ui.downloadLabel = downLabel;
    
    m_uiRows.insert(name, ui);
    this->adjustSize();
}

void NetworkWidget::removeInterfaceRow(const QString &name) {
    if (!m_uiRows.contains(name)) return;

    NetworkInterfaceUI ui = m_uiRows.take(name);
    m_containerLayout->removeWidget(ui.rowWidget);
    delete ui.rowWidget;
    this->adjustSize();
}

void NetworkWidget::setUpdateInterval(int ms) {
    BaseComponent::setUpdateInterval(ms);
    if (m_updateTimer) {
        m_updateTimer->setInterval(ms);
    }
}

QString NetworkWidget::formatSpeed(double bytesPerSec) {
    double value = bytesPerSec;
    QString unit = "B/s";
    
    if (m_showInBits) {
        value *= 8;
        unit = "bps";
    }

    if (value < 1024) {
        return QString::number(value, 'f', 0) + " " + unit;
    } else if (value < 1024 * 1024) {
        return QString::number(value / 1024.0, 'f', 1) + " K" + unit;
    } else if (value < 1024 * 1024 * 1024) {
        return QString::number(value / (1024.0 * 1024.0), 'f', 1) + " M" + unit;
    } else {
        return QString::number(value / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " G" + unit;
    }
}

void NetworkWidget::setCustomSetting(const QString &key, const QVariant &value) {
    if (key == "showInBits") {
        m_showInBits = value.toBool();
        updateData();
    } else if (key == "selectedInterfaces") {
        m_selectedInterfaces = value.toStringList();
        // Clear existing rows to force rebuild on next update
        // Or we can handle it in updateData
        updateData();
    }
}

QStringList NetworkWidget::getAvailableInterfaces() const {
    return m_interfaceList;
}

#ifdef Q_OS_WIN
void NetworkWidget::initPdh() {
    if (PdhOpenQuery(NULL, 0, &m_pdhQuery) != ERROR_SUCCESS) {
        qWarning() << "Failed to open PDH query for NetworkWidget";
        m_pdhQuery = NULL;
        return;
    }

    // 嘗試加入英文計數器 (Vista+)
    // Network Interface(*)\Bytes Sent/sec
    PDH_STATUS status = PdhAddEnglishCounterW(m_pdhQuery, L"\\Network Interface(*)\\Bytes Sent/sec", 0, &m_pdhCounterSent);
    if (status != ERROR_SUCCESS) {
        qWarning() << "Failed to add English counter for Sent (Status:" << status << "). Trying index-based...";
        // Fallback: 使用索引路徑 \510(*)\506 (Network Interface / Bytes Sent/sec)
        if (PdhAddCounterW(m_pdhQuery, L"\\510(*)\\506", 0, &m_pdhCounterSent) != ERROR_SUCCESS) {
            qWarning() << "Failed to add index counter for Sent.";
        }
    }

    status = PdhAddEnglishCounterW(m_pdhQuery, L"\\Network Interface(*)\\Bytes Received/sec", 0, &m_pdhCounterReceived);
    if (status != ERROR_SUCCESS) {
        qWarning() << "Failed to add English counter for Received (Status:" << status << "). Trying index-based...";
        // Fallback: 使用索引路徑 \510(*)\264 (Network Interface / Bytes Received/sec)
        if (PdhAddCounterW(m_pdhQuery, L"\\510(*)\\264", 0, &m_pdhCounterReceived) != ERROR_SUCCESS) {
            qWarning() << "Failed to add index counter for Received.";
        }
    }

    // 第一次收集數據 (初始化)
    PdhCollectQueryData(m_pdhQuery);
    qDebug() << "NetworkWidget PDH initialized. Query:" << m_pdhQuery << "Sent:" << m_pdhCounterSent << "Recv:" << m_pdhCounterReceived;
}
#endif

void NetworkWidget::updateData() {
#ifdef Q_OS_WIN
    if (!m_pdhQuery) {
        initPdh();
        if (!m_pdhQuery) return;
    }

    // 收集數據
    PDH_STATUS collectStatus = PdhCollectQueryData(m_pdhQuery);
    if (collectStatus != ERROR_SUCCESS) {
        qWarning() << "PdhCollectQueryData failed:" << collectStatus;
        return;
    }

    // Temporary storage for per-interface data
    QMap<QString, double> sentMap;
    QMap<QString, double> recvMap;
    QStringList currentInterfaces;

    auto processCounter = [&](PDH_HCOUNTER counter, QMap<QString, double> &dataMap, bool isSent) {
        if (!counter) return;

        DWORD bufferSize = 0;
        DWORD itemCount = 0;
        
        PDH_STATUS status = PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, NULL);
        
        if (status == PDH_MORE_DATA || (status == ERROR_SUCCESS && bufferSize > 0)) {
            QVector<char> buffer(bufferSize);
            PPDH_FMT_COUNTERVALUE_ITEM_W items = (PPDH_FMT_COUNTERVALUE_ITEM_W)buffer.data();
            
            if (PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, items) == ERROR_SUCCESS) {
                for (DWORD i = 0; i < itemCount; i++) {
                    QString name = QString::fromWCharArray(items[i].szName);
                    double val = items[i].FmtValue.doubleValue;
                    
                    if (name.contains("Loopback", Qt::CaseInsensitive)) continue;

                    if (isSent) currentInterfaces.append(name);

                    if (items[i].FmtValue.CStatus == PDH_CSTATUS_VALID_DATA || 
                        items[i].FmtValue.CStatus == PDH_CSTATUS_NEW_DATA) {
                        dataMap[name] = val;
                    }
                }
            }
        }
    };

    processCounter(m_pdhCounterSent, sentMap, true);
    processCounter(m_pdhCounterReceived, recvMap, false);
    
    // Update member list
    if (!currentInterfaces.isEmpty()) {
        m_interfaceList = currentInterfaces;
        m_interfaceList.removeDuplicates();
    }

    // Determine what to show
    QStringList interfacesToShow = m_selectedInterfaces;
    if (interfacesToShow.isEmpty()) {
        interfacesToShow << "Total"; // Default to Total if nothing selected
    }

    // Remove rows that are no longer needed
    QList<QString> currentRows = m_uiRows.keys();
    for (const QString &rowName : currentRows) {
        if (!interfacesToShow.contains(rowName)) {
            removeInterfaceRow(rowName);
        }
    }

    // Update or create rows
    for (const QString &target : interfacesToShow) {
        if (!m_uiRows.contains(target)) {
            createInterfaceRow(target);
        }

        double sent = 0;
        double recv = 0;

        if (target == "Total") {
            // Sum all valid interfaces
            for (auto it = sentMap.begin(); it != sentMap.end(); ++it) sent += it.value();
            for (auto it = recvMap.begin(); it != recvMap.end(); ++it) recv += it.value();
        } else {
            // Specific interface
            sent = sentMap.value(target, 0);
            recv = recvMap.value(target, 0);
        }

        NetworkInterfaceUI &ui = m_uiRows[target];
        ui.uploadLabel->setText(QString("↑ %1").arg(formatSpeed(sent)));
        ui.downloadLabel->setText(QString("↓ %1").arg(formatSpeed(recv)));
    }
#endif
}
