#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QList>
#include <QVariantMap>
#include <QMap>
#include <QString>

// 引入自定義組件
#include "ToolSettingsForm.h"
#include "BaseComponent.h"
#include "TimeWidget.h"

namespace Ui { class ControlPanel; }

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

private slots:
    /**
     * @brief 當左側小工具列表切換選中項目時觸發
     * @param currentRow 目前選中的行號
     */
    void on_toolList_currentRowChanged(int currentRow);

    /**
     * @brief 處理主題分頁的儲存按鈕點擊
     */
    void on_saveTheme_clicked();

    /**
     * @brief 處理全域設定分頁的套用按鈕點擊
     */
    void on_applySetting_clicked();

    /**
     * @brief 處理來自 ToolSettingsForm 的工具顯示/隱藏開關
     * @param visible 是否顯示
     */
    //void on_widgetVisibilityChanged(bool visible);

private:
    Ui::ControlPanel *ui;

    /**
     * @brief 小工具實例管理器
     * Key: 工具的唯一的 ID (來自 JSON)
     * Value: 繼承自 BaseComponent 的實體指標
     */
    QMap<QString, BaseComponent*> m_widgetInstances;

    /**
     * @brief 儲存從 JSON 讀取的原始配置數據列表
     */
    QList<QVariantMap> m_toolsData;

    /**
     * @brief 右側嵌入式的通用設定表單實例
     */
    ToolSettingsForm *m_settingsForm;

    /**
     * @brief 從資源檔讀取 tools_config.json 並初始化 m_toolsData
     * @return 讀取成功返回 true
     */
    bool loadToolsConfiguration();

    /**
     * @brief 根據 m_toolsData 內的配置，預先實例化所有小工具
     */
    void initWidgets();

    /**
     * @brief 輔助函式：根據目前列表選中的項目，獲取對應的 Widget 實例
     */
    BaseComponent* getCurrentSelectedWidget();

    // 追蹤當前已連接訊號的 Widget，用於切換時斷開連線
    BaseComponent* m_currentConnectedWidget = nullptr;
};

#endif // CONTROLPANEL_H
