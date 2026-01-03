#include "ToolSettingsForm.h"
#include "ui_ToolSettingsForm.h"
#include "Widgets/NetworkWidget.h"
#include "ImageWidget.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QSpinBox>
#include <QListWidget>


ToolSettingsForm::ToolSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolSettingsForm)
{
    ui->setupUi(this);
    ui->data_media_group->hide(); // 預設隱藏特定欄位

    // 初始化內部 UI 元件與萬用訊號的連接
    setupInternalConnections();
}

ToolSettingsForm::~ToolSettingsForm() {
    delete ui;
}

void ToolSettingsForm::setupInternalConnections() {
    // 當這些開關或輸入框變動時，統統交給 emitSetting 打包發送
    connect(ui->enableTool_checkBox, &QCheckBox::clicked, this, &ToolSettingsForm::emitSetting);
    connect(ui->coordX_lineEdit, &QLineEdit::editingFinished, this, &ToolSettingsForm::onCoordinateEdited);
    connect(ui->coordY_lineEdit, &QLineEdit::editingFinished, this, &ToolSettingsForm::onCoordinateEdited);
    connect(ui->hoverHide_checkBox, &QCheckBox::clicked, this, &ToolSettingsForm::emitSetting);

    // Slider 使用 sliderReleased 是為了避免拉動過程中發送太多次訊號
    connect(ui->transparency_slider, &QSlider::sliderReleased, this, &ToolSettingsForm::emitSetting);

    connect(ui->position_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolSettingsForm::emitSetting);
    connect(ui->draggable_checkBox, &QCheckBox::clicked, this, &ToolSettingsForm::emitSetting);
    connect(ui->snapToEdge_checkBox, &QCheckBox::clicked, this, &ToolSettingsForm::emitSetting);
    connect(ui->clickThrough_checkBox, &QCheckBox::clicked, this, &ToolSettingsForm::emitSetting);
}

void ToolSettingsForm::loadToolData(const QVariantMap &toolData) {
    m_currentToolData = toolData;
    QString toolId = m_currentToolData["id"].toString();

    // 介面初始化 (不發射訊號)
    ui->enableTool_checkBox->blockSignals(true);
    ui->enableTool_checkBox->setChecked(m_currentToolData["default_enabled"].toBool());
    ui->enableTool_checkBox->blockSignals(false);

    // 根據 ID 決定要不要顯示路徑設定群組
    updateSpecificFields(toolId);

    // 建立進階設定
    createAdvancedSettings(toolId);
}

void ToolSettingsForm::createAdvancedSettings(const QString &toolId) {
    // 清除舊的動態元件
    // 這裡簡單實作：如果已經有 advanced_groupBox 就先移除
    QGroupBox *oldGroup = findChild<QGroupBox*>("advanced_groupBox");
    if (oldGroup) {
        delete oldGroup;
    }

    if (toolId == "system_monitor") {
        QGroupBox *advGroup = new QGroupBox("進階顯示設定", this);
        advGroup->setObjectName("advanced_groupBox");
        QVBoxLayout *layout = new QVBoxLayout(advGroup);

        QCheckBox *chkCores = new QCheckBox("顯示 CPU 核心詳細資訊", advGroup);
        QCheckBox *chkCoreFreq = new QCheckBox("顯示 CPU 頻率", advGroup); // 改名
        QCheckBox *chkRam = new QCheckBox("顯示記憶體詳細 (GB)", advGroup);
        
        // 新增：頻率演算法選擇
        QLabel *lblFreq = new QLabel("頻率顯示演算法:", advGroup);
        QComboBox *comboFreq = new QComboBox(advGroup);
        comboFreq->addItem("最大頻率 (Max)", 0);
        comboFreq->addItem("平均頻率 (Average)", 1);
        comboFreq->setObjectName("freqAlgo_comboBox");

        layout->addWidget(chkCores);
        layout->addWidget(chkCoreFreq); // 新增
        layout->addWidget(chkRam);
        layout->addWidget(lblFreq);
        layout->addWidget(comboFreq);

        // 連接訊號
        connect(chkCores, &QCheckBox::clicked, this, [this, chkCores](){
            emit settingChanged("showCores", chkCores->isChecked());
        });
        connect(chkCoreFreq, &QCheckBox::clicked, this, [this, chkCoreFreq](){ // 新增
            emit settingChanged("showCoreFreq", chkCoreFreq->isChecked());
        });
        connect(chkRam, &QCheckBox::clicked, this, [this, chkRam](){
            emit settingChanged("showRamDetail", chkRam->isChecked());
        });
        connect(comboFreq, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboFreq](int index){
            emit settingChanged("freqAlgo", comboFreq->currentData());
        });

        // 將新群組加入主佈局 (插入在最後面)
        ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1, advGroup);
    }
    else if (toolId == "disks_info") {
        QGroupBox *advGroup = new QGroupBox("進階顯示設定", this);
        advGroup->setObjectName("advanced_groupBox");
        QVBoxLayout *layout = new QVBoxLayout(advGroup);

        QCheckBox *chkUsage = new QCheckBox("顯示使用率百分比", advGroup);
        chkUsage->setObjectName("chkUsage");
        QCheckBox *chkSpeed = new QCheckBox("顯示讀寫速度 (R/W)", advGroup);
        chkSpeed->setObjectName("chkSpeed");
        QCheckBox *chkActive = new QCheckBox("顯示硬碟活動時間 (Active Time)", advGroup);
        chkActive->setObjectName("chkActive");

        layout->addWidget(chkUsage);
        layout->addWidget(chkSpeed);
        layout->addWidget(chkActive);

        connect(chkUsage, &QCheckBox::clicked, this, [this, chkUsage](){
            emit settingChanged("showUsagePercent", chkUsage->isChecked());
        });
        connect(chkSpeed, &QCheckBox::clicked, this, [this, chkSpeed](){
            emit settingChanged("showTransferSpeed", chkSpeed->isChecked());
        });
        connect(chkActive, &QCheckBox::clicked, this, [this, chkActive](){
            emit settingChanged("showActiveTime", chkActive->isChecked());
        });

        ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1, advGroup);
    }
    else if (toolId == "network") {
        QGroupBox *advGroup = new QGroupBox("進階顯示設定", this);
        advGroup->setObjectName("advanced_groupBox");
        QVBoxLayout *layout = new QVBoxLayout(advGroup);

        QCheckBox *chkBits = new QCheckBox("顯示為 Bits (bps)", advGroup);
        chkBits->setObjectName("network_bits_checkBox");
        
        QLabel *lblInterface = new QLabel("選擇網路介面 (可多選):", advGroup);
        QListWidget *listInterfaces = new QListWidget(advGroup);
        listInterfaces->setObjectName("network_interface_list");
        listInterfaces->setSelectionMode(QAbstractItemView::NoSelection); // Handle selection via checkboxes
        listInterfaces->setFixedHeight(150);

        layout->addWidget(chkBits);
        layout->addWidget(lblInterface);
        layout->addWidget(listInterfaces);

        connect(chkBits, &QCheckBox::clicked, this, [this, chkBits](){
            emit settingChanged("showInBits", chkBits->isChecked());
        });
        
        // Handle item changes in the list widget
        connect(listInterfaces, &QListWidget::itemChanged, this, [this, listInterfaces](QListWidgetItem *item){
            QStringList selected;
            for(int i=0; i<listInterfaces->count(); ++i) {
                QListWidgetItem* it = listInterfaces->item(i);
                if (it->checkState() == Qt::Checked) {
                    selected.append(it->text());
                }
            }
            emit settingChanged("selectedInterfaces", selected);
        });

        ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1, advGroup);
    }
    else if (toolId == "image_viewer") {
        QGroupBox *advGroup = new QGroupBox("圖片縮放比例", this);
        advGroup->setObjectName("advanced_groupBox");
        QVBoxLayout *layout = new QVBoxLayout(advGroup);

        // 使用一個25% ~ 100%的拉bar
        QLabel *lblScale = new QLabel("縮放百分比 (25% - 100%):", advGroup);
        QSlider *scaleSlider = new QSlider(Qt::Horizontal, advGroup);
        scaleSlider->setRange(25, 100);
        scaleSlider->setValue(100); // 預設 100%
        scaleSlider->setObjectName("image_scale_slider");

        layout->addWidget(lblScale);
        layout->addWidget(scaleSlider);

        // 滑動拉桿時
        connect(scaleSlider, &QSlider::valueChanged, this, [this, scaleSlider](int value){
            emit settingChanged("scale", value);
        });

        ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1, advGroup);
    }
}

#include "Widgets/CpuWidget.h" // 需要引入以使用 dynamic_cast
#include "Widgets/DiskWidget.h"

void ToolSettingsForm::updateAllUI(BaseComponent* w) {
    if (!w) return;

    // 暫時鎖定訊號，避免同步 UI 時觸發 emitSetting 導致無限迴圈
    this->blockSignals(true);

    ui->coordX_lineEdit->setText(QString::number(w->x()));
    ui->coordY_lineEdit->setText(QString::number(w->y()));
    ui->enableTool_checkBox->setChecked(w->isVisible());

    // 這裡需要確保 BaseComponent 有這些 getter 函式
    ui->draggable_checkBox->setChecked(w->isDraggable());
    ui->snapToEdge_checkBox->setChecked(w->isSnapEnabled());
    ui->hoverHide_checkBox->setChecked(w->isHoverHideEnabled());
    ui->transparency_slider->setValue(static_cast<int>(w->hoverOpacity() * 100));

    ui->position_comboBox->setCurrentIndex(w->windowLayer());
    ui->clickThrough_checkBox->setChecked(w->isClickThrough());

    // 更新進階設定 (如果是 CpuWidget)
    CpuWidget* cpuWidget = dynamic_cast<CpuWidget*>(w);
    if (cpuWidget) {
        QComboBox* comboFreq = findChild<QComboBox*>("freqAlgo_comboBox");
        if (comboFreq) {
            comboFreq->setCurrentIndex(static_cast<int>(cpuWidget->frequencyMode()));
        }
    }
    
    // 更新進階設定 (如果是 DiskWidget)
    DiskWidget* diskWidget = dynamic_cast<DiskWidget*>(w);
    if (diskWidget) {
        QCheckBox* chkUsage = findChild<QCheckBox*>("chkUsage");
        QCheckBox* chkSpeed = findChild<QCheckBox*>("chkSpeed");
        QCheckBox* chkActive = findChild<QCheckBox*>("chkActive");
        
        if (chkUsage) chkUsage->setChecked(diskWidget->isShowUsagePercent());
        if (chkSpeed) chkSpeed->setChecked(diskWidget->isShowTransferSpeed());
        if (chkActive) chkActive->setChecked(diskWidget->isShowActiveTime());
    }
    NetworkWidget* netWidget = dynamic_cast<NetworkWidget*>(w);
    if (netWidget) {
        QCheckBox* chkBits = findChild<QCheckBox*>("network_bits_checkBox");
        if (chkBits) {
            chkBits->blockSignals(true);
            chkBits->setChecked(netWidget->isShowInBits());
            chkBits->blockSignals(false);
        }

        QListWidget* listInterfaces = findChild<QListWidget*>("network_interface_list");
        if (listInterfaces) {
            listInterfaces->blockSignals(true);
            listInterfaces->clear();
            
            QStringList available = netWidget->getAvailableInterfaces();
            QStringList selected = netWidget->getSelectedInterfaces();
            
            // Always add "Total" option
            QListWidgetItem* totalItem = new QListWidgetItem("Total", listInterfaces);
            totalItem->setFlags(totalItem->flags() | Qt::ItemIsUserCheckable);
            totalItem->setCheckState(selected.contains("Total") || selected.isEmpty() ? Qt::Checked : Qt::Unchecked);

            for (const QString &iface : available) {
                QListWidgetItem* item = new QListWidgetItem(iface, listInterfaces);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(selected.contains(iface) ? Qt::Checked : Qt::Unchecked);
            }
            
            listInterfaces->blockSignals(false);
        }
    }
    ImageWidget* imgWidget = dynamic_cast<ImageWidget*>(w);
    if (imgWidget) {
        QSlider* scaleSlider = findChild<QSlider*>("image_scale_slider");
        if (scaleSlider) {
            scaleSlider->blockSignals(true); // 避免觸發訊號導致重複更新
            scaleSlider->setValue(imgWidget->currentScale());
            scaleSlider->blockSignals(false);
        }

        // 同步路徑
        // ui->musicPath_lineEdit->setText(imgWidget->currentPath());
    }

    this->blockSignals(false);
}

void ToolSettingsForm::emitSetting() {
    QObject* senderObj = sender();
    if (!senderObj) return;

    // --- 萬能打包邏輯 ---
    if (senderObj == ui->enableTool_checkBox) {
        emit settingChanged("visible", ui->enableTool_checkBox->isChecked());
    }
    else if (senderObj == ui->hoverHide_checkBox || senderObj == ui->transparency_slider) {
        QVariantMap hoverData;
        hoverData["enabled"] = ui->hoverHide_checkBox->isChecked();
        // 限制 99% 避免滑鼠無法感應
        hoverData["opacity"] = qMin(ui->transparency_slider->value(), 99) / 100.0;
        emit settingChanged("hover", hoverData);
    }
    else if (senderObj == ui->position_comboBox) {
        emit settingChanged("layer", ui->position_comboBox->currentIndex());
    }
    else if (senderObj == ui->draggable_checkBox) {
        emit settingChanged("draggable", ui->draggable_checkBox->isChecked());
    }
    else if (senderObj == ui->snapToEdge_checkBox) {
        emit settingChanged("snap", ui->snapToEdge_checkBox->isChecked());
    }
    else if (senderObj == ui->clickThrough_checkBox) {
        emit settingChanged("clickThrough", ui->clickThrough_checkBox->isChecked());
    }
}

void ToolSettingsForm::onCoordinateEdited() {
    int x = ui->coordX_lineEdit->text().toInt();
    int y = ui->coordY_lineEdit->text().toInt();
    emit settingChanged("pos", QPoint(x, y));
}

void ToolSettingsForm::updateSpecificFields(const QString &toolId) {
    ui->data_media_group->hide();

    if (toolId == "media_player") {
        ui->data_media_group->setTitle("音樂路徑設定");
        ui->path_label->setText("音樂資料夾:");
        ui->data_media_group->show();
    }
    else if (toolId == "image_viewer") {
        ui->data_media_group->setTitle("圖片顯示設定");
        ui->path_label->setText("當前圖片:");
        ui->data_media_group->show();
    }
}

void ToolSettingsForm::on_browsePath_button_clicked() {
    QString toolId = m_currentToolData["id"].toString();
    QString finalPath;

    if (toolId == "image_viewer") {
        QString filePath = QFileDialog::getOpenFileName(this, "選擇圖片/GIF", "", "Images (*.png *.jpg *.jpeg *.gif)");
        if (!filePath.isEmpty()) {
            // 釋放目前資源 否則gif會持續占用
            emit settingChanged("clear_media", true);
            if (QFile::exists(finalPath)) { QFile::remove(finalPath); }
            // 實作的備份邏輯

            QString saveDir = QCoreApplication::applicationDirPath() + "/user_assets/images/";
            QDir().mkpath(saveDir);
            QFileInfo info(filePath);
            finalPath = saveDir + "display_image." + info.suffix();

            if (QFile::exists(finalPath)) QFile::remove(finalPath);
            if (QFile::copy(filePath, finalPath)) {
                ui->musicPath_lineEdit->setText(finalPath);
            }
        }
    }
    else if (toolId == "media_player") {
        finalPath = QFileDialog::getExistingDirectory(this, "選擇音樂資料夾");
        if (!finalPath.isEmpty()) {
            ui->musicPath_lineEdit->setText(finalPath);
        }
    }

    // 當路徑改變時，也發送萬用訊號！標籤設為 "path" 
    if (!finalPath.isEmpty()) {
        emit settingChanged("path", finalPath);
    }
}

void ToolSettingsForm::on_saveSettings_button_clicked() {
    qDebug() << "正在儲存" << m_currentToolData["id"].toString() << "的本地設定";
    // 這裡可以透過發送一個 "save" 標籤來讓 ControlPanel 執行 SettingsManager 的儲存
    emit settingChanged("save_request", true);
}

void ToolSettingsForm::setCoordinateDisplay(int x, int y) {
    ui->coordX_lineEdit->blockSignals(true);
    ui->coordY_lineEdit->blockSignals(true);
    ui->coordX_lineEdit->setText(QString::number(x));
    ui->coordY_lineEdit->setText(QString::number(y));
    ui->coordX_lineEdit->blockSignals(false);
    ui->coordY_lineEdit->blockSignals(false);
}

void ToolSettingsForm::setSwitchState(bool checked) {
    ui->enableTool_checkBox->blockSignals(true);
    ui->enableTool_checkBox->setChecked(checked);
    ui->enableTool_checkBox->blockSignals(false);
}
