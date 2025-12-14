#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ControlPanel;
}
QT_END_NAMESPACE

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

private:
    Ui::ControlPanel *ui;
};
#endif // CONTROLPANEL_H
