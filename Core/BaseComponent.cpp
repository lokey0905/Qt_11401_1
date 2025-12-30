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

// --- 視窗屬性讀取 (解決遇到的 undefined reference 錯誤) ---

int BaseComponent::windowLayer() const {
    if (this->windowFlags() & Qt::WindowStaysOnTopHint) return 1; // 置頂
    if (this->windowFlags() & Qt::WindowStaysOnBottomHint) return 2; // 置底
    return 0; // 普通
}

bool BaseComponent::isClickThrough() const {
    // 檢查目前的視窗旗標中是否含有「點擊穿透」標籤
    return (this->windowFlags() & Qt::WindowTransparentForInput);
}
// --- 行為設定 ---

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

void BaseComponent::enterEvent(QEnterEvent *event) {
    if (m_hoverHide) {
        // 設定透明度，保留 0.01 確保滑鼠離開時仍能觸發事件
        this->setWindowOpacity(qMax(0.01, 1.0 - m_hoverOpacity));
    }
    QWidget::enterEvent(event);
}

void BaseComponent::leaveEvent(QEvent *event) {
    if (m_hoverHide) {
        this->setWindowOpacity(1.0); // 恢復完全不透明
    }
    QWidget::leaveEvent(event);
}

// --- 拖曳與吸附核心邏輯 ---

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

    // 1. 螢幕邊緣吸附
    if (qAbs(newPos.x() - screenRect.left()) < snapMargin)
        newPos.setX(screenRect.left());
    if (qAbs(newPos.x() + this->width() - screenRect.right()) < snapMargin)
        newPos.setX(screenRect.right() - this->width());
    if (qAbs(newPos.y() - screenRect.top()) < snapMargin)
        newPos.setY(screenRect.top());
    if (qAbs(newPos.y() + this->height() - screenRect.bottom()) < snapMargin)
        newPos.setY(screenRect.bottom() - this->height());

    // 2. 元件間吸附
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
