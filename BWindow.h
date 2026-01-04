#pragma once

#include <QMainWindow>
#include <QVector>

class StartWindow;

class QLabel;
class QTextEdit;
class QPushButton;
class QStackedWidget;
class QCheckBox;
class QWidget;

class TileButton;

class BWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BWindow(QWidget *parent = nullptr);

    void setStartWindow(StartWindow *sw);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void goStart();
    void goA();
    void goB();

private:
    QWidget *buildTileGroup(QWidget *parent);
    QWidget *buildPanelB(QWidget *parent);

    void refreshCountLabel();
    void clearAllTiles();
    void runB();

private:
    StartWindow *m_startWindow = nullptr;

    QVector<TileButton*> tileButtons;
    QLabel *labCount = nullptr;
    QTextEdit *txtOutput = nullptr;

    QPushButton *btnRunB = nullptr;
};
