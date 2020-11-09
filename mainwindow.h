#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "schedule.h"
#include "dictedit.h"

#include <QMainWindow>
#include <QStringListModel>
#include <QAbstractTableModel>
#include <QListView>
#include <QPushButton>
#include <QDialog>

/*
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
*/

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow) {
        ui->setupUi(this);

        ui->comboBox->setModel(&rooms);
        ui->tableView->setModel(new ScheduleModel { this });

        connect(ui->comboBox,
                &QComboBox::currentTextChanged,
                static_cast<ScheduleModel*>(ui->tableView->model()),
                &ScheduleModel::setActiveRoom);

        connect(ui->action_Classes, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &classes, this };
            de->show();
        });
        connect(ui->action_Groups, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &groups, this };
            de->show();
        });
        connect(ui->action_Rooms, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &rooms, this };
            de->show();
        });
        connect(ui->action_Teachers, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &teachers, this };
            de->show();
        });
    }

    ~MainWindow() {
        delete ui;
    }

private:
    Ui::MainWindow *ui;

    // list of: rooms, groups, classes, teachers
    QStringListModel rooms, groups, classes, teachers;
};

#endif // MAINWINDOW_H
