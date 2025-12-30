#include "ControlPanel.h"
#include "ui_ControlPanel.h"
#include "SettingsManager.h"
#include "Widgets/TimeWidget.h"
#include "Widgets/CpuWidget.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSpinBox>

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel),
    m_settingsForm(nullptr)
{
    ui->setupUi(this);

    // 1. 載入配置
    if (!loadToolsConfiguration()) {
        qCritical() << "Failed to load tool configurations.";
    }

    // 初始化全域設定 UI 狀態
    ui->globalLockDrag_checkBox->setChecked(SettingsManager::instance()->isGlobalDragLocked());
    ui->trayIcon_checkBox->setChecked(SettingsManager::instance()->showTrayIcon());

    // 2. 實例化工具
    initWidgets();

    // 3. 嵌入設定面板
    m_settingsForm = new ToolSettingsForm(ui->settingsContainer_widget);
    QVBoxLayout *layout = new QVBoxLayout(ui->settingsContainer_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_settingsForm);

    // 4. 【核心中樞】連接萬用設定訊號
    connect(m_settingsForm, &ToolSettingsForm::settingChanged, this, [this](QString key, QVariant value){
        int currentRow = ui->toolList_widget->currentRow();
        if (currentRow < 0) return;

        QString id = m_toolsData.at(currentRow)["id"].toString();
        if (!m_widgetInstances.contains(id)) return;

        BaseComponent* w = m_widgetInstances[id];

        // 分發指令邏輯
        if (key == "visible") {
            w->setVisible(value.toBool());
        }
        else if (key == "pos") {
            w->move(value.toPoint());
        }
        else if (key == "hover") {
            QVariantMap data = value.toMap();
            w->setHoverHide(data["enabled"].toBool());
            w->setHoverOpacity(data["opacity"].toDouble());
        }
        else if (key == "layer") {
            w->setWindowLayer(value.toInt());
        }
        else if (key == "draggable") {
            w->setDraggable(value.toBool());
        }
        else if (key == "snap") {
            w->setSnapToEdge(value.toBool());
        }
        else if (key == "clickThrough") {
            // --- 修正後的點擊穿透處理 ---
            Qt::WindowFlags flags = w->windowFlags();
            if (value.toBool()) {
                // 增加穿透標籤：這會讓視窗不再接收任何滑鼠點擊
                flags |= Qt::WindowTransparentForInput;
            } else {
                flags &= ~Qt::WindowTransparentForInput;

            }
            w->setWindowFlags(flags);
            w->show(); // 必須重新 show 才能生效
        }
        else if (key == "interval") { // 新增：處理更新頻率
            w->setUpdateInterval(value.toInt());
        }
        else if (key == "path") {
            qDebug() << "路徑變更通知：" << id << " -> " << value.toString();
        }
        else if (key == "save_request") {
            this->on_saveTheme_clicked();
        }
        else {
            // 轉發其他自訂設定給元件
            w->setCustomSetting(key, value);
        }
    });

    // 5. 清單切換連動
    if (!m_toolsData.isEmpty()) {
        connect(ui->toolList_widget, &QListWidget::currentRowChanged,
                this, &ControlPanel::on_toolList_currentRowChanged);
        
        // 手動觸發第一次選擇，確保介面初始化正確
        ui->toolList_widget->setCurrentRow(0);
        on_toolList_currentRowChanged(0);
    }

    // 6. 全域更新頻率連動
    // 定義預設選項對應的數值 (與 ComboBox 順序一致)
    // 慢速(2000), 普通(1000), 快速(500), 極速(250), 即時(100), 自訂(-1)
    const QList<int> intervalPresets = {2000, 1000, 500, 250, 100, -1};

    // 設定預設值 (普通 1000ms)
    ui->globalInterval_comboBox->setCurrentIndex(1); 

    auto updateIntervalFunc = [this, intervalPresets](int index) {
        int ms = 1000;
        if (index >= 0 && index < intervalPresets.size()) {
            ms = intervalPresets[index];
        }
        
        // 如果是自訂模式
        if (ms == -1) {
            ui->globalInterval_spinBox->setEnabled(true);
            ms = ui->globalInterval_spinBox->value();
        } else {
            ui->globalInterval_spinBox->setEnabled(false);
            ui->globalInterval_spinBox->setValue(ms); // 同步顯示數值
        }

        // 更新所有元件
        for(auto w : m_widgetInstances) {
            if(w) w->setUpdateInterval(ms);
        }
    };

    // 連接 ComboBox 變更
    connect(ui->globalInterval_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, updateIntervalFunc);

    // 連接 SpinBox 變更 (僅在自訂模式下有效)
    connect(ui->globalInterval_spinBox, &QSpinBox::valueChanged, this, [this, updateIntervalFunc](int){
        if (ui->globalInterval_comboBox->currentIndex() == 5) { // 5 is Custom
            updateIntervalFunc(5);
        }
    });

    connect(ui->saveTheme_button, &QPushButton::clicked, this, &ControlPanel::on_saveTheme_clicked);
    connect(ui->applySetting_button, &QPushButton::clicked, this, &ControlPanel::on_applySetting_clicked);
}

void ControlPanel::initWidgets()
{
    for (const QVariantMap &data : m_toolsData) {
        QString id = data["id"].toString();
        QString widgetClass = data["widget_class"].toString();
        bool defaultEnabled = data["default_enabled"].toBool();

        BaseComponent* instance = nullptr;
        if (widgetClass == "TimeWidget") {
            instance = new TimeWidget();
        } else if (widgetClass == "CpuWidget") {
            instance = new CpuWidget();
        }

        if (instance) {
            m_widgetInstances.insert(id, instance);
            if (defaultEnabled) {
                // 延遲顯示，防止初始化時視窗定位異常
                QTimer::singleShot(150, instance, &BaseComponent::show);
            } else {
                instance->hide();
            }
        }
    }
}

void ControlPanel::on_toolList_currentRowChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < m_toolsData.size()) {
        const QVariantMap &toolData = m_toolsData.at(currentRow);
        m_settingsForm->loadToolData(toolData);

        QString id = toolData["id"].toString();
        if (m_widgetInstances.contains(id)) {
            BaseComponent* widget = m_widgetInstances[id];

            // 同步 UI 狀態
            m_settingsForm->updateAllUI(widget);

            // 座標即時連動：斷開舊連線並建立新連線
            if (m_currentConnectedWidget) {
                disconnect(m_currentConnectedWidget, &BaseComponent::positionChanged, m_settingsForm, &ToolSettingsForm::setCoordinateDisplay);
            }
            connect(widget, &BaseComponent::positionChanged, m_settingsForm, &ToolSettingsForm::setCoordinateDisplay);
            m_currentConnectedWidget = widget;
        }
    }
}

// ... 其餘 loadToolsConfiguration, on_saveTheme_clicked, on_applySetting_clicked 保持不變...

bool ControlPanel::loadToolsConfiguration()
{
    QFile loadFile(":/config/tools_config.json");
    if (!loadFile.open(QIODevice::ReadOnly)) return false;
    QByteArray data = loadFile.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    if (!jsonDoc.isArray()) return false;
    QJsonArray toolsArray = jsonDoc.array();
    for (const QJsonValue &value : toolsArray) {
        QJsonObject toolObject = value.toObject();
        m_toolsData.append(toolObject.toVariantMap());
        ui->toolList_widget->addItem(toolObject["name"].toString());
    }
    return true;
}

void ControlPanel::on_saveTheme_clicked() {
    qDebug() << "設定已儲存";
}

void ControlPanel::on_applySetting_clicked() {
    SettingsManager::instance()->setGlobalDragLocked(ui->globalLockDrag_checkBox->isChecked());
    SettingsManager::instance()->setShowTrayIcon(ui->trayIcon_checkBox->isChecked());
}

ControlPanel::~ControlPanel() {
    qDeleteAll(m_widgetInstances); // 清理所有小工具實例
    delete ui;
}
