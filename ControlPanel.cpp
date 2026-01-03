#include "ControlPanel.h"
#include "ui_ControlPanel.h"
#include "SettingsManager.h"

/* ----- 引入具體的小工具類別 ----- */
#include "Widgets/TimeWidget.h"
#include "Widgets/CpuWidget.h"
#include "Widgets/DiskWidget.h"
#include "Widgets/NetworkWidget.h"
#include "Widgets/ToDoWidget.h"
#include "Widgets/ImageWidget.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QTimer>

#include <QCloseEvent>

/**
 * @brief 建構子：統籌初始化流程
 */
ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel),
    m_settingsForm(nullptr)
{
    ui->setupUi(this);

    /** 1. 載入工具配置與托盤初始化 **/
    if (!loadToolsConfiguration()) {
        qCritical() << "Failed to load tool configurations.";
    }
    initTrayIcon();

    /** 2. 初始化全域設定 UI 狀態 **/
    ui->globalLockDrag_checkBox->setChecked(SettingsManager::instance()->isGlobalDragLocked());
    ui->trayIcon_checkBox->setChecked(SettingsManager::instance()->showTrayIcon());
    ui->autoStart_checkBox->setChecked(SettingsManager::instance()->isAutoStart());

    /** 3. 實例化所有工具 Widget **/
    initWidgets();

    /** 4. 嵌入右側設定表單與訊號連線 **/
    m_settingsForm = new ToolSettingsForm(ui->settingsContainer_widget);
    QVBoxLayout *layout = new QVBoxLayout(ui->settingsContainer_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_settingsForm);

    // 連接萬用設定訊號
    connect(m_settingsForm, &ToolSettingsForm::settingChanged, this, [this](QString key, QVariant value){
        int currentRow = ui->toolList_widget->currentRow();
        if (currentRow < 0) return;

        QString id = m_toolsData.at(currentRow)["id"].toString();
        if (!m_widgetInstances.contains(id)) return;

        BaseComponent* w = m_widgetInstances[id];

        if (key == "visible") w->setVisible(value.toBool());
        else if (key == "pos") w->move(value.toPoint());
        else if (key == "hover") {
            QVariantMap data = value.toMap();
            w->setHoverHide(data["enabled"].toBool());
            w->setHoverOpacity(data["opacity"].toDouble());
        }
        else if (key == "layer") w->setWindowLayer(value.toInt());
        else if (key == "draggable") w->setDraggable(value.toBool());
        else if (key == "snap") w->setSnapToEdge(value.toBool());
        else if (key == "clickThrough") {
            Qt::WindowFlags flags = w->windowFlags();
            if (value.toBool()) flags |= Qt::WindowTransparentForInput;
            else flags &= ~Qt::WindowTransparentForInput;
            w->setWindowFlags(flags);
            w->show();
        }
        else if (key == "interval") w->setUpdateInterval(value.toInt());
        else if (key == "path") w->setCustomSetting("path", value);
        else if (key == "clear_media") w->setCustomSetting("clear_media", value);
        else if (key == "save_request") this->on_saveTheme_clicked();
        else w->setCustomSetting(key, value);
    });

    /** 5. 設定清單連動與主題清單初始化 **/
    if (!m_toolsData.isEmpty()) {
        connect(ui->toolList_widget, &QListWidget::currentRowChanged, this, &ControlPanel::on_toolList_currentRowChanged);
        ui->toolList_widget->setCurrentRow(0);
        on_toolList_currentRowChanged(0);
    }

    /** 6. 全域更新頻率邏輯實作 **/
    const QList<int> intervalPresets = {2000, 1000, 500, 250, 100, -1};
    ui->globalInterval_comboBox->setCurrentIndex(1);

    auto updateIntervalFunc = [this, intervalPresets](int index) {
        int ms = (index >= 0 && index < intervalPresets.size()) ? intervalPresets[index] : 1000;
        if (ms == -1) {
            ui->globalInterval_spinBox->setEnabled(true);
            ms = ui->globalInterval_spinBox->value();
        } else {
            ui->globalInterval_spinBox->setEnabled(false);
            ui->globalInterval_spinBox->setValue(ms);
        }
        for(auto w : m_widgetInstances) if(w) w->setUpdateInterval(ms);
    };

    connect(ui->globalInterval_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, updateIntervalFunc);
    connect(ui->globalInterval_spinBox, &QSpinBox::valueChanged, this, [this, updateIntervalFunc](int){
        if (ui->globalInterval_comboBox->currentIndex() == 5) updateIntervalFunc(5);
    });

    /** 7. 設定管理員訊號連動 (托盤即時同步) **/
    connect(SettingsManager::instance(), &SettingsManager::trayIconSettingChanged, this, [this](bool show){
        if (show) m_trayIcon->show();
        else m_trayIcon->hide();
    });

    /** 8. 載入上次套用主題 **/
    QTimer::singleShot(500, this, [this]() {
        QString lastTheme = SettingsManager::instance()->lastTheme();
        if (!lastTheme.isEmpty()) {
            // 尋找清單中是否有對應的主題項目
            QList<QListWidgetItem*> items = ui->themeList_widget->findItems(lastTheme, Qt::MatchExactly);
            if (!items.isEmpty()) {
                ui->themeList_widget->setCurrentItem(items.first());
                this->on_LoadTheme_clicked();
            }
        }
    });

    connect(ui->saveTheme_button, &QPushButton::clicked, this, &ControlPanel::on_saveTheme_clicked);
    connect(ui->applySetting_button, &QPushButton::clicked, this, &ControlPanel::on_applySetting_clicked);
    updateThemeList();
    connect(ui->loadTheme_button, &QPushButton::clicked, this, &ControlPanel::on_LoadTheme_clicked);
    connect(ui->deleteTheme_button, &QPushButton::clicked, this, &ControlPanel::on_DeleteTheme_clicked);
}

/** --- 工具實例化與配置載入 --- **/

void ControlPanel::initWidgets() {
    for (const QVariantMap &data : m_toolsData) {
        QString id = data["id"].toString();
        QString widgetClass = data["widget_class"].toString();
        BaseComponent* instance = nullptr;

        if (widgetClass == "TimeWidget") instance = new TimeWidget();
        else if (widgetClass == "CpuWidget") instance = new CpuWidget();
        else if (widgetClass == "DiskWidget") instance = new DiskWidget();
        else if (widgetClass == "NetworkWidget") instance = new NetworkWidget();
        else if (widgetClass == "ToDoWidget") instance = new ToDoWidget();
        else if (widgetClass == "ImageWidget") instance = new ImageWidget();

        if (instance) {
            m_widgetInstances.insert(id, instance);
            if (data["default_enabled"].toBool()) QTimer::singleShot(150, instance, &BaseComponent::show);
            else instance->hide();
        }
    }
}

bool ControlPanel::loadToolsConfiguration() {
    QFile loadFile(":/config/tools_config.json");
    if (!loadFile.open(QIODevice::ReadOnly)) return false;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(loadFile.readAll()));
    if (!jsonDoc.isArray()) return false;
    QJsonArray toolsArray = jsonDoc.array();
    for (const QJsonValue &value : toolsArray) {
        QJsonObject toolObject = value.toObject();
        m_toolsData.append(toolObject.toVariantMap());
        ui->toolList_widget->addItem(toolObject["name"].toString());
    }
    return true;
}

/** --- 系統托盤實作 --- **/

void ControlPanel::initTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/config/system_call.png"));

    QMenu *trayMenu = new QMenu(this);
    QAction *showAction = new QAction("顯示/隱藏控制面板", this);
    QAction *quitAction = new QAction("退出程式", this);

    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    m_trayIcon->setContextMenu(trayMenu);

    connect(showAction, &QAction::triggered, this, &ControlPanel::toggleVisibility);
    connect(quitAction, &QAction::triggered, this, [this](){
        m_forceClose = true; // 設定強制關閉
        qApp->quit();
    });

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r){
        if (r == QSystemTrayIcon::Trigger) toggleVisibility();
    });

    if (SettingsManager::instance()->showTrayIcon()) m_trayIcon->show();
}

void ControlPanel::toggleVisibility() {
    if (this->isVisible()) this->hide();
    else { this->show(); this->activateWindow(); }
}

/** --- 處理視窗關閉事件的核心邏輯 --- **/
void ControlPanel::closeEvent(QCloseEvent *event) {
    // 檢查目前是否啟用了系統托盤顯示
    if (SettingsManager::instance()->showTrayIcon()) {
        /** 1. 如果有托盤 則隱藏視窗並忽略關閉事件 **/
        this->hide();
        event->ignore(); // 阻止程式真正結束

        qDebug() << "ControlPanel: 已縮小至系統托盤";
    } else {
        /** 2. 如果沒有托盤 則直接關閉**/
        qDebug() << "ControlPanel: 正在關閉程式..";
        event->accept(); // 允許程式關閉
    }
}

/** --- 槽函式實作 (分頁切換與按鈕處理) --- **/

void ControlPanel::on_toolList_currentRowChanged(int currentRow) {
    if (currentRow >= 0 && currentRow < m_toolsData.size()) {
        const QVariantMap &toolData = m_toolsData.at(currentRow);
        m_settingsForm->loadToolData(toolData);
        QString id = toolData["id"].toString();
        if (m_widgetInstances.contains(id)) {
            BaseComponent* widget = m_widgetInstances[id];
            m_settingsForm->updateAllUI(widget);
            if (m_currentConnectedWidget) disconnect(m_currentConnectedWidget, &BaseComponent::positionChanged, m_settingsForm, &ToolSettingsForm::setCoordinateDisplay);
            connect(widget, &BaseComponent::positionChanged, m_settingsForm, &ToolSettingsForm::setCoordinateDisplay);
            m_currentConnectedWidget = widget;
        }
    }
}

void ControlPanel::on_applySetting_clicked() {
    SettingsManager* sm = SettingsManager::instance();
    sm->setGlobalDragLocked(ui->globalLockDrag_checkBox->isChecked());
    sm->setShowTrayIcon(ui->trayIcon_checkBox->isChecked());
    sm->setAutoStart(ui->autoStart_checkBox->isChecked());
    //QMessageBox::information(this, "設定", "全域設定已成功套用");
}

void ControlPanel::on_saveTheme_clicked() {
    QString name = ui->themeName_lineEdit->text().trimmed();
    if (name.isEmpty()) { QMessageBox::warning(this, "提示", "請輸入主題名稱"); return; }
    themeMgr.saveTheme(name, getGlobalLayoutData());
    updateThemeList();
    ui->themeName_lineEdit->clear();
}

void ControlPanel::on_DeleteTheme_clicked() {
    QListWidgetItem *item = ui->themeList_widget->currentItem();
    if (item) { themeMgr.deleteTheme(item->text()); updateThemeList(); }
}

void ControlPanel::updateThemeList() {
    ui->themeList_widget->clear();
    ui->themeList_widget->addItems(themeMgr.getThemeList());
}

/**
 * @brief 抓取目前所有小工具的完整狀態（包含通用屬性與特定元件的私有設定）
 */
QJsonObject ControlPanel::getGlobalLayoutData() {
    QJsonObject root;
    QJsonArray widgetsArray;

    for (auto it = m_widgetInstances.begin(); it != m_widgetInstances.end(); ++it) {
        QString id = it.key();
        BaseComponent* w = it.value();
        if (!w) continue;

        QJsonObject widgetInfo;
        // --- A. 通用基礎屬性 (BaseComponent) ---
        widgetInfo["id"] = id;
        widgetInfo["x"] = w->x();
        widgetInfo["y"] = w->y();
        widgetInfo["visible"] = w->isVisible();
        widgetInfo["draggable"] = w->isDraggable();
        widgetInfo["snap"] = w->isSnapEnabled();
        widgetInfo["hoverHide"] = w->isHoverHideEnabled();
        widgetInfo["hoverOpacity"] = w->hoverOpacity();
        widgetInfo["layer"] = w->windowLayer();
        widgetInfo["clickThrough"] = w->isClickThrough();

        qDebug() << w -> isVisible();

        // --- B. 針對特定 Widget 獲取私有設定 (動態轉型) ---

        // 1. CpuWidget
        if (auto* cpuW = dynamic_cast<CpuWidget*>(w)) {
            widgetInfo["frequencyMode"] = static_cast<int>(cpuW->frequencyMode());
        }
        // 2. DiskWidget
        else if (auto* diskW = dynamic_cast<DiskWidget*>(w)) {
            widgetInfo["showUsage"] = diskW->isShowUsagePercent();
            widgetInfo["showSpeed"] = diskW->isShowTransferSpeed();
            widgetInfo["showActive"] = diskW->isShowActiveTime();
        }
        // 3. NetworkWidget
        else if (auto* netW = dynamic_cast<NetworkWidget*>(w)) {
            widgetInfo["showInBits"] = netW->isShowInBits();
            widgetInfo["selectedInterfaces"] = QJsonArray::fromStringList(netW->getSelectedInterfaces());
        }
        // 4. ImageWidget
        else if (auto* imgW = dynamic_cast<ImageWidget*>(w)) {
            widgetInfo["scale"] = imgW->currentScale();
            widgetInfo["path"] = imgW->currentPath();
        }

        widgetsArray.append(widgetInfo);
    }
    root["widgets"] = widgetsArray;
    return root;
}

void ControlPanel::on_LoadTheme_clicked() {
    QListWidgetItem *item = ui->themeList_widget->currentItem();
    if (!item) return;
    QString themeName = item->text();
    QJsonObject data = themeMgr.loadTheme(themeName);
    QJsonArray widgets = data["widgets"].toArray();

    for (int i = 0; i < widgets.size(); ++i) {
        QJsonObject obj = widgets[i].toObject();
        QString id = obj["id"].toString();
        if (!m_widgetInstances.contains(id)) continue;

        BaseComponent* w = m_widgetInstances[id];

        // --- A. 還原通用外殼 (BaseComponent) ---
        w->move(obj["x"].toInt(), obj["y"].toInt());
        w->setVisible(obj["visible"].toBool());
        w->setDraggable(obj["draggable"].toBool());
        w->setSnapToEdge(obj["snap"].toBool());
        w->setHoverHide(obj["hoverHide"].toBool());
        w->setHoverOpacity(obj["hoverOpacity"].toDouble());
        w->setWindowLayer(obj["layer"].toInt());
        w->setClickThrough(obj["clickThrough"].toBool());

        // --- B. 還原特定私有設定 ---

        if (auto* cpuW = dynamic_cast<CpuWidget*>(w)) {
            cpuW->setCustomSetting("frequencyMode", obj["frequencyMode"].toVariant());
        }
        else if (auto* diskW = dynamic_cast<DiskWidget*>(w)) {
            diskW->setCustomSetting("showUsage", obj["showUsage"].toVariant());
            diskW->setCustomSetting("showSpeed", obj["showSpeed"].toVariant());
            diskW->setCustomSetting("showActive", obj["showActive"].toVariant());
        }
        else if (auto* netW = dynamic_cast<NetworkWidget*>(w)) {
            netW->setCustomSetting("showInBits", obj["showInBits"].toVariant());
            // 這裡假設 setCustomSetting 裡面有實作還原介面清單喵
            netW->setCustomSetting("selectedInterfaces", obj["selectedInterfaces"].toVariant());
        }
        else if (auto* imgW = dynamic_cast<ImageWidget*>(w)) {
            imgW->setCustomSetting("scale", obj["scale"].toVariant());
            imgW->setCustomSetting("path", obj["path"].toVariant());
        }

        if (obj["visible"].toBool())
        {
            qDebug() << id;
            w->show(); // 重新顯示以應用變動
        }
        else
        {
            w->hide();
        }
    }
    // 更新左側列表對應的 UI 面板喵
    on_toolList_currentRowChanged(ui->toolList_widget->currentRow());
    SettingsManager::instance()->setLastTheme(themeName);
}

ControlPanel::~ControlPanel() {
    qDeleteAll(m_widgetInstances);
    delete ui;
}
