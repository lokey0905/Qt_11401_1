#ifndef BASECOMPONENT_H
#define BASECOMPONENT_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QTimer>
#include <QCursor>

/**
 * @brief 所有桌面小工具的基底類別
 * 繼承自 QWidget，提供通用的拖曳、吸附、透明度與視窗屬性控制
 */
class BaseComponent : public QWidget
{
    Q_OBJECT
public:
    explicit BaseComponent(QWidget *parent = nullptr);
    virtual ~BaseComponent() = default;

    // --- 虛擬函式：由子類別實作具體內容 ---
    virtual void initStyle();
    virtual void updateData() = 0; // 用於每秒更新 (如時間、系統資訊)
    virtual void setUpdateInterval(int ms) { m_updateInterval = ms; } // 新增：設定更新頻率
    virtual int updateInterval() const { return m_updateInterval; }   // 新增：取得更新頻率

    // --- 行為設定 (Setters) ---
    void setDraggable(bool enable) { m_isDraggable = enable; }
    void setSnapToEdge(bool enable) { m_isSnapEnabled = enable; }
    void setWindowLayer(int layer); // 0:普通, 1:置頂, 2:置底

    // 透明度與懸停效果
    void setHoverHide(bool enable);
    void setHoverOpacity(double opacity) { m_hoverOpacity = opacity; } // 0.0 ~ 1.0

    // --- 狀態讀取 (Getters) ---
    // 這些函式是讓 ControlPanel 同步介面數值的關鍵
    bool isDraggable() const { return m_isDraggable; }
    bool isSnapEnabled() const { return m_isSnapEnabled; }
    bool isHoverHideEnabled() const { return m_hoverHide; }
    double hoverOpacity() const { return m_hoverOpacity; }

    // --- 擴充設定介面 ---
    virtual void setCustomSetting(const QString &key, const QVariant &value) {
        Q_UNUSED(key);
        Q_UNUSED(value);
    }

    /**
     * @brief 取得目前的視窗層級標籤
     * @return 0:普通, 1:置頂, 2:置底
     */
    int windowLayer() const;

    /**
     * @brief 檢查目前是否啟用了點擊穿透屬性
     */
    bool isClickThrough() const;

    /**
     * @brief 設定是否開啟滑鼠點擊穿透喵
     * @param enabled 為 true 時視窗將不再接收滑鼠事件
     */
    void setClickThrough(bool enabled) {
        Qt::WindowFlags flags = windowFlags();
        if (enabled) {
            // 加上穿透旗標喵
            flags |= Qt::WindowTransparentForInput;
        } else {
            // 移除穿透旗標喵
            flags &= ~Qt::WindowTransparentForInput;
        }
        setWindowFlags(flags);

        // 必須重新 show 才能確保 Windows 系統正確套用旗標變動喵
        if (isVisible()) {
            show();
        }
    }

signals:
    // 當視窗座標改變時發出，讓控制面板的輸入框能跟著動
    void positionChanged(int x, int y);

protected:
    // 繪圖與事件處理
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 視窗移動事件：當視窗被移動時發送 positionChanged 訊號
    void moveEvent(QMoveEvent *event) override {
        QWidget::moveEvent(event);
        emit positionChanged(this->x(), this->y());
    }

    // 懸停隱藏邏輯屬性
    bool m_hoverHide = false;
    double m_hoverOpacity = 1.0;
    int m_updateInterval = 1000; // 預設 1000ms

    // 滑鼠進出事件：處理懸停透明度變化
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    /** @brief 定時檢查滑鼠位置，處理穿透模式下的懸停邏輯 */
    void handleHoverCheck();

private:
    // 拖曳相關內部狀態
    bool m_isDraggable = true;
    bool m_isSnapEnabled = true;
    bool m_isMoving = false;
    QPoint m_dragPosition; // 紀錄滑鼠按下時的相對位置

    // 監控計時器
    QTimer *m_hoverCheckTimer;

    // 邊緣吸附運算
    void performSnap(QPoint &newPos);
};

#endif // BASECOMPONENT_H
