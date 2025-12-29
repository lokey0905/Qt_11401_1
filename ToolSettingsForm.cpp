#include "ToolSettingsForm.h"
#include "ui_ToolSettingsForm.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

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
}

void ToolSettingsForm::updateAllUI(BaseComponent* w) {
    if (!w) return;

    // 暫時鎖定訊號，避免同步 UI 時觸發 emitSetting 導致無限迴圈
    this->blockSignals(true);

    ui->coordX_lineEdit->setText(QString::number(w->x()));
    ui->coordY_lineEdit->setText(QString::number(w->y()));
    ui->enableTool_checkBox->setChecked(w->isVisible());

    // 這裡需要確保 BaseComponent 有這些 getter 函式喔
    ui->draggable_checkBox->setChecked(w->isDraggable());
    ui->snapToEdge_checkBox->setChecked(w->isSnapEnabled());
    ui->hoverHide_checkBox->setChecked(w->isHoverHideEnabled());
    ui->transparency_slider->setValue(static_cast<int>(w->hoverOpacity() * 100));

    ui->position_comboBox->setCurrentIndex(w->windowLayer());
    ui->clickThrough_checkBox->setChecked(w->isClickThrough());

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
