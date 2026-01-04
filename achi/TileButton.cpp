#include "TileButton.h"
#include <QMouseEvent>

TileButton::TileButton(int idx , const QString &name , int maxCount , QWidget *parent)
    : QPushButton(parent)
    , m_index(idx)
    , m_name(name)
    , m_max(maxCount)
{
    setMinimumHeight(34);
    refresh();
}

int TileButton::index() const { return m_index; }
int TileButton::count() const { return m_count; }

void TileButton::setCount(int v)
{
    if (v < 0) v = 0;
    if (v > m_max) v = m_max;
    if (v == m_count) return;

    m_count = v;
    refresh();
    emit countChanged(m_index , m_count);
}

void TileButton::refresh()
{
    setText(QString("%1  %2").arg(m_name).arg(m_count));
}

void TileButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) setCount(m_count + 1);
    if (e->button() == Qt::RightButton) setCount(m_count - 1);
}
