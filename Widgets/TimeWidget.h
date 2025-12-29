#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include "Core/BaseComponent.h"
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TimeWidget : public BaseComponent {
    Q_OBJECT
public:
    explicit TimeWidget(QWidget *parent = nullptr);

    void initStyle() override;
    void updateData() override;

private:
    QLabel *m_timeLabel;
    QLabel *m_dayLabel;   // 顯示星期 (Thursday)
    QLabel *m_dateLabel;  // 顯示日期 (11.09.2025)
    QTimer *m_updateTimer;
};

#endif // TIMEWIDGET_H
