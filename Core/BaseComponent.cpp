#include "BaseComponent.h"
#include "SettingsManager.h"
#include <QStyleOption>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QDebug>

BaseComponent::BaseComponent(QWidget *parent) : QWidget(parent) {
    // 基礎視窗設定：無邊框、背景透明、工具視窗(不顯示在工作列)
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // 初始化懸停監控定時器
    m_hoverCheckTimer = new QTimer(this);
    m_hoverCheckTimer->setInterval(100); // 100ms 輪詢一次
    connect(m_hoverCheckTimer, &QTimer::timeout, this, &BaseComponent::handleHoverCheck);
}

void BaseComponent::initStyle() {
    // 統一圓角與半透明背景樣式
    this->setStyleSheet(
        "BaseComponent {"
        "  background-color: rgba(30, 30, 30, 180);"
        "  border-radius: 20px;"
        "  border: 1px solid rgba(255, 255, 255, 40);"
        "}"
        );
}

/** --- 視窗狀態查詢 --- **/

int BaseComponent::windowLayer() const {
    if (this->windowFlags() & Qt::WindowStaysOnTopHint) return 1; // 置頂
    if (this->windowFlags() & Qt::WindowStaysOnBottomHint) return 2; // 置底
    return 0; // 普通
}

bool BaseComponent::isClickThrough() const {
    // 檢查目前的視窗旗標中是否含有「點擊穿透」標籤
    return (this->windowFlags() & Qt::WindowTransparentForInput);
}

/** --- 行為控制實作 --- **/

void BaseComponent::setWindowLayer(int layer) {
    Qt::WindowFlags flags = this->windowFlags();
    if (layer == 1) { // 永遠置頂
        flags |= Qt::WindowStaysOnTopHint;
        flags &= ~Qt::WindowStaysOnBottomHint;
    } else if (layer == 2) { // 置於底部
        flags |= Qt::WindowStaysOnBottomHint;
        flags &= ~Qt::WindowStaysOnTopHint;
    } else { // 普通
        flags &= ~Qt::WindowStaysOnTopHint;
        flags &= ~Qt::WindowStaysOnBottomHint;
    }
    this->setWindowFlags(flags);
    this->show(); // 必須重新顯示以應用 Flag
}

// --- 懸停透明度邏輯 ---
void BaseComponent::setHoverHide(bool enable) {
    m_hoverHide = enable;
    if (m_hoverHide) {
        m_hoverCheckTimer->start(); // 開啟功能即啟動主動輪詢
    } else {
        m_hoverCheckTimer->stop();
        this->setWindowOpacity(1.0); // 確保功能關閉時恢復顯示
    }
}

/** --- 懸停偵測核心邏輯 (主動輪詢) --- **/

void BaseComponent::handleHoverCheck() {
    if (!m_hoverHide) return;

    // 主動偵測滑鼠是否在元件範圍內 (解決點擊穿透導致 enterEvent 不觸發的問題)
    bool isHovering = this->geometry().contains(QCursor::pos());

    if (isHovering) {
        double targetOpacity = qMax(0.01, 1.0 - m_hoverOpacity);
        if (this->windowOpacity() != targetOpacity) {
            this->setWindowOpacity(targetOpacity);
        }
    } else {
        if (this->windowOpacity() != 1.0) {
            this->setWindowOpacity(1.0);
        }
    }
}

// 舊邏輯
// 會與點擊穿透衝突 暫時保留
void BaseComponent::enterEvent(QEnterEvent *event) { QWidget::enterEvent(event); }
void BaseComponent::leaveEvent(QEvent *event) { QWidget::leaveEvent(event); }

/** --- 拖曳與吸附處理 --- **/
void BaseComponent::mousePressEvent(QMouseEvent *event) {
    bool isGlobalLocked = SettingsManager::instance()->isGlobalDragLocked();
    if (event->button() == Qt::LeftButton && !isGlobalLocked && m_isDraggable) {
        m_isMoving = true;
        m_dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        event->accept();
    }
}

void BaseComponent::mouseMoveEvent(QMouseEvent *event) {
    if (m_isMoving && (event->buttons() & Qt::LeftButton)) {
        QPoint newPos = event->globalPos() - m_dragPosition;
        if (m_isSnapEnabled) {
            performSnap(newPos);
        }
        this->move(newPos);
        event->accept();
    }
}

void BaseComponent::mouseReleaseEvent(QMouseEvent *event) {
    m_isMoving = false;
}

void BaseComponent::performSnap(QPoint &newPos) {
    int snapMargin = 20;
    QScreen *currentScreen = this->screen();
    if (!currentScreen) currentScreen = QApplication::primaryScreen();
    QRect screenRect = currentScreen->availableGeometry();

    // 螢幕邊緣吸附
    if (qAbs(newPos.x() - screenRect.left()) < snapMargin)
        newPos.setX(screenRect.left());
    if (qAbs(newPos.x() + this->width() - screenRect.right()) < snapMargin)
        newPos.setX(screenRect.right() - this->width());
    if (qAbs(newPos.y() - screenRect.top()) < snapMargin)
        newPos.setY(screenRect.top());
    if (qAbs(newPos.y() + this->height() - screenRect.bottom()) < snapMargin)
        newPos.setY(screenRect.bottom() - this->height());

    // 元件間吸附
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *w : widgets) {
        // 排除自己、隱藏的視窗、以及非 BaseComponent 的視窗
        if (w == this || !w->isVisible() || !w->inherits("BaseComponent")) continue;

        QRect otherRect = w->frameGeometry();
        
        // X 軸吸附
        // 左對左 (對齊)
        if (qAbs(newPos.x() - otherRect.left()) < snapMargin) newPos.setX(otherRect.left());
        // 右對右 (對齊)
        if (qAbs(newPos.x() + this->width() - otherRect.right()) < snapMargin) newPos.setX(otherRect.right() - this->width());
        // 左對右 (緊貼)
        if (qAbs(newPos.x() - otherRect.right()) < snapMargin) newPos.setX(otherRect.right());
        // 右對左 (緊貼)
        if (qAbs(newPos.x() + this->width() - otherRect.left()) < snapMargin) newPos.setX(otherRect.left() - this->width());

        // Y 軸吸附
        // 上對上 (對齊)
        if (qAbs(newPos.y() - otherRect.top()) < snapMargin) newPos.setY(otherRect.top());
        // 下對下 (對齊)
        if (qAbs(newPos.y() + this->height() - otherRect.bottom()) < snapMargin) newPos.setY(otherRect.bottom() - this->height());
        // 上對下 (緊貼)
        if (qAbs(newPos.y() - otherRect.bottom()) < snapMargin) newPos.setY(otherRect.bottom());
        // 下對上 (緊貼)
        if (qAbs(newPos.y() + this->height() - otherRect.top()) < snapMargin) newPos.setY(otherRect.top() - this->height());
    }
}

void BaseComponent::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
