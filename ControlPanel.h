#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QList>
#include <QVariantMap>
#include <QMap>
#include <QString>
#include <QJsonObject>

// 引入自定義組件
#include "ToolSettingsForm.h"
#include "BaseComponent.h"
#include "ThemeManager.h"

#include <QSystemTrayIcon>
#include <QMenu>


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
     * @brief 處理主題分頁的載入按鈕點擊
     */
    void on_LoadTheme_clicked();

    /**
     * @brief 處理主題分頁的刪除按鈕點擊
     */
    void on_DeleteTheme_clicked();

    /**
     * @brief 處理全域設定分頁的套用按鈕點擊
     */
    void on_applySetting_clicked();

    /**
     * @brief 切換主視窗的顯示或隱藏狀態 (用於托盤連動) 喵
     */
    void toggleVisibility();

signals:
    void requestLoadTheme(const QJsonObject &themeData);

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
     * @brief 初始化系統托盤功能與選單喵
     */
    void initTrayIcon();

    /**
     * @brief 根據 m_toolsData 內的配置，預先實例化所有小工具
     */
    void initWidgets();

    /**
     * @brief 輔助函式：根據目前列表選中的項目，獲取對應的 Widget 實例
     */
    BaseComponent* getCurrentSelectedWidget();

    /**
     * @brief 系統托盤圖示實例
     */
    QSystemTrayIcon *m_trayIcon = nullptr;
    bool m_forceClose = false; // 標記是否強制關閉程式

    ThemeManager themeMgr; // 主題管理員

    // 核心功能函數
    void updateThemeList(); // 更新 UI 上的主題清單
    QJsonObject getGlobalLayoutData(); // 抓取目前所有 Widget 的狀態

    // 追蹤當前已連接訊號的 Widget，用於切換時斷開連線
    BaseComponent* m_currentConnectedWidget = nullptr;

protected:
    /**
     * @brief 處理視窗關閉事件
     * 若托盤圖示正在顯示，則縮排至托盤；否則直接結束程式喵
     * @param event 關閉事件指標
     */
    void closeEvent(QCloseEvent *event) override;
};

#endif // CONTROLPANEL_H
