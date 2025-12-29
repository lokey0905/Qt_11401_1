#include "TimeWidget.h"
#include <QDateTime>
#include <QStyle>

TimeWidget::TimeWidget(QWidget *parent) : BaseComponent(parent) {
    m_timeLabel = new QLabel(this);
    m_dayLabel = new QLabel(this);
    m_dateLabel = new QLabel(this);

    // 垂直佈局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8); // 縮減內邊距
    mainLayout->setSpacing(2); // 讓文字靠緊縮

    // 時間
    mainLayout->addWidget(m_timeLabel, 0, Qt::AlignCenter);

    // 星期 日期
    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->addWidget(m_dayLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_dateLabel);

    mainLayout->addLayout(infoLayout);

    // 定時器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TimeWidget::updateData);

    initStyle();
    updateData();
}

void TimeWidget::initStyle() {
    BaseComponent::initStyle(); // 繼承半透明圓角背景

    // 調整字體大小與緊湊度
    this->setStyleSheet(this->styleSheet() +
                        "TimeWidget {"
                        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                        "    stop:0 rgba(0, 0, 0, 150), stop:0.55 rgba(0, 0, 0, 150), "
                        "    stop:0.56 rgba(30, 30, 30, 200), stop:1 rgba(30, 30, 30, 200)); "
                        "  border: 1px solid rgba(255, 255, 255, 30);"
                        "}"
                        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
                        "#timeLabel { font-size: 22px; font-weight: 600; letter-spacing: 1px; }"
                        "#dayLabel, #dateLabel { font-size: 11px; color: rgba(255, 255, 255, 180); font-weight: normal; }"
                        );

    m_timeLabel->setObjectName("timeLabel");
    m_dayLabel->setObjectName("dayLabel");
    m_dateLabel->setObjectName("dateLabel");

    //更新
    this->style()->unpolish(this);
    this->style()->polish(this);
    // 縮小視窗總尺寸
    this->setFixedSize(180, 70);
}

void TimeWidget::updateData() {
    QDateTime now = QDateTime::currentDateTime();
    m_timeLabel->setText(now.toString("HH:mm"));
    m_dayLabel->setText(now.toString("dddd")); // 使用縮寫 (Sun) 更緊湊
    m_dateLabel->setText(now.toString("dd.MM.yyyy"));

    // 計算到下一分鐘的秒數進行更新
    int msecToNextMinute = (60 - now.time().second()) * 1000;
    m_updateTimer->start(msecToNextMinute);
}
