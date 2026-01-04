#include "SquareButton.h"
#include <QResizeEvent>
#include <algorithm>

SquareButton::SquareButton(QWidget *parent)
    : QPushButton(parent)
{
}

void SquareButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    int side = std::min(width(), height());
    setFixedSize(side, side);
}
