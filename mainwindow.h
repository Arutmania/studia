#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "schedule.h"
#include "dictedit.h"
#include "entryedit.h"

#include <QMainWindow>
#include <QStringListModel>
#include <QAbstractTableModel>
#include <QListView>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QDebug>

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

        // jak nie ma pokoi to się jebie?
        connect(ui->comboBox,
                &QComboBox::currentTextChanged,
                static_cast<ScheduleModel*>(ui->tableView->model()),
                &ScheduleModel::setActiveRoom);

        connect(ui->action_Classes, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &classes, this };
            connect(de, &QDialog::accepted, ui->tableView->model(), [this] {
                auto schedule = static_cast<ScheduleModel*>(ui->tableView->model());
                schedule->removeInvalidClasses(classes.stringList());
            });
            de->show();
        });

        connect(ui->action_Groups, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &groups, this };
            connect(de, &QDialog::accepted, ui->tableView->model(), [this] {
                auto schedule = static_cast<ScheduleModel*>(ui->tableView->model());
                schedule->removeInvalidGroups(groups.stringList());
            });
            de->show();
        });

        connect(ui->action_Rooms, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &rooms, this };
            connect(de, &QDialog::accepted, ui->tableView->model(), [this] {
                auto schedule = static_cast<ScheduleModel*>(ui->tableView->model());
                auto rooms    = static_cast<QStringListModel*>(ui->comboBox->model());
                schedule->removeInvalidRooms(rooms->stringList());
            });
            de->show();
        });

        connect(ui->action_Teachers, &QAction::triggered, this, [this] {
            auto de = new DictEdit { &teachers, this };
            connect(de, &QDialog::accepted, ui->tableView->model(), [this] {
                auto schedule = static_cast<ScheduleModel*>(ui->tableView->model());
                schedule->removeInvalidTeachers(teachers.stringList());
            });
            de->show();
        });

        connect(ui->tableView, &QTableView::doubleClicked, this, [this] (auto const& index){
            if (ui->comboBox->currentIndex() != -1) {
                auto w = new EntryEdit { static_cast<ScheduleModel*>(ui->tableView->model()), index, &groups, &classes, &teachers, this };
                w->show();
            } else {
                // this seems to give error when closed with mouse and apparently is a well known bug?
                // qt.qpa.xcb: QXcbConnection: XCB error: 3 (BadWindow), sequence: 1397, resource id: 8884191, major code: 40 (TranslateCoords), minor code: 0
                // welp, not my fault. everything werks tho
                QMessageBox::warning(this, "planner", "no room selected");
            }
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
