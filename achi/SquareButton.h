#pragma once

#include <QPushButton>

class SquareButton : public QPushButton
{
public:
    explicit SquareButton(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
};
