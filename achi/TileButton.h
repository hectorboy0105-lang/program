#ifndef TILEBUTTON_H
#define TILEBUTTON_H

#include <QPushButton>
#include <QString>

class QMouseEvent;

class TileButton : public QPushButton
{
    Q_OBJECT

public:
    TileButton(int idx , const QString &name , int maxCount , QWidget *parent = nullptr);

    int index() const;
    int count() const;

    void setCount(int v);

signals:
    void countChanged(int index , int count);

protected:
    void mousePressEvent(QMouseEvent *e) override;

private:
    void refresh();

private:
    int m_index = 0;
    QString m_name;
    int m_max = 4;
    int m_count = 0;
};

#endif
