#ifndef TOOLSETTINGSFORM_H
#define TOOLSETTINGSFORM_H

#include <QWidget>
#include <QVariantMap>
#include <QVariant>
#include "BaseComponent.h" // 為了 updateAllUI 使用 BaseComponent 指針 [cite: 8, 9]

namespace Ui { class ToolSettingsForm; }

class ToolSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ToolSettingsForm(QWidget *parent = nullptr);
    ~ToolSettingsForm();

    // 從外部（ControlPanel）載入工具的 JSON 配置數據 [cite: 5, 8]
    void loadToolData(const QVariantMap &toolData);

    // 【核心連動】將 Widget 實例的當前狀態完全同步到 UI 介面上
    void updateAllUI(BaseComponent* w);

    // 【即時回傳】當小工具被滑鼠拖動時，由 ControlPanel 呼叫此函式更新座標數值
    void setCoordinateDisplay(int x, int y);

    // 更新「啟用工具」勾選框的狀態 [cite: 5, 8]
    void setSwitchState(bool checked);

signals:
    /**
     * @brief 萬能設定變更訊號
     * @param key 設定的標籤 (例如: "visible", "pos", "layer", "hover", "draggable" 等)
     * @param value 設定的新數值 (使用 QVariant 封裝不同的資料型態)
     */
    void settingChanged(QString key, QVariant value);

private slots:
    // 處理介面組件的 Slot
    void on_browsePath_button_clicked(); // 處理路徑瀏覽按鈕 [cite: 5]
    void on_saveSettings_button_clicked(); // 處理儲存按鈕 [cite: 5]

    // 內部處理：當座標 QLineEdit 編輯完成時觸發 [cite: 5, 8]
    void onCoordinateEdited();

    // 【中樞轉發】當任何 UI 組件變動時，統籌發射 settingChanged 訊號
    void emitSetting();

private:
    Ui::ToolSettingsForm *ui;

    // 儲存當前選中的工具數據 [cite: 5, 8]
    QVariantMap m_currentToolData;

    // 根據工具 ID 顯示或隱藏特定欄位（如音樂路徑） [cite: 5]
    void updateSpecificFields(const QString &toolId);

    // 初始化內部 UI 元件與 emitSetting 的連接
    void setupInternalConnections();
};

#endif // TOOLSETTINGSFORM_H
