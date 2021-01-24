#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "schedule.h"
#include "dictedit.h"
#include "entryedit.h"

#include <QAbstractTableModel>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow) {
        ui->setupUi(this);

        ui->comboBox->setModel(&rooms);
        ui->tableView->setModel(new ScheduleModel { this });
        ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        connect(ui->comboBox,
                &QComboBox::currentTextChanged,
                static_cast<ScheduleModel*>(ui->tableView->model()),
                &ScheduleModel::setActiveRoom);

        connect(ui->action_Load, &QAction::triggered, this, [this] {
            auto filename = QFileDialog::getOpenFileName(this,
                                                         "Load from JSON",
                                                         ".",
                                                         "JSON documents (*.json)");
            if (filename.isNull())
                return;
            loadJson(filename);
        });

        connect(ui->action_Save, &QAction::triggered, this, [this] {
            auto filename = QFileDialog::getSaveFileName(this,
                                                         "Save to JSON",
                                                         "."
                                                         "JSON documents (*.json)");
            // documentation doesn't seem to mention what happens when user cancels the dialog
            // but extrapolating from what is said in the QFileDialog::getOpenFileName
            // a null string is returned
            if (filename.isNull())
                return;
            saveJson(filename);
        });

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
                auto w = new EntryEdit {
                    static_cast<ScheduleModel*>(ui->tableView->model()),
                        index,
                        &groups,
                        &classes,
                        &teachers,
                        this
                };
                w->show();
            } else {
                // this seems to give error when closed with mouse and apparently is a well known bug?
                //  qt.qpa.xcb: QXcbConnection: XCB error: 3 (BadWindow),
                //  sequence: 1397, resource id: 8884191, major code: 40 (TranslateCoords), minor code: 0
                // welp, not my fault. everything werks tho
                QMessageBox::warning(this, "planner", "no room selected");
            }
        });
    }

    ~MainWindow() {
        delete ui;
    }

    bool loadJson(QString const& filename) {
        auto file = QFile { filename };
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << QStringLiteral("couldn't open file: %1").arg(filename);
            return false;
        }
        auto object = QJsonDocument::fromJson(file.readAll()).object();
        file.close();

        auto schedule = static_cast<ScheduleModel*>(ui->tableView->model());

        auto toStringList = [] (auto const* arrayName, auto const& jsonArray) {
            auto list = QStringList {};
            for (auto const& elem : jsonArray) {
                if (!elem.isString()) {
                    qDebug() << QStringLiteral("elements of array %1 should be strings").arg(arrayName);
                    continue;
                }
                list << elem.toString();
            }
            return list;
        };

        auto rooms_ = object["rooms"].toArray();
        rooms.setStringList(toStringList("rooms", rooms_));
        if (rooms.stringList().empty())
            qDebug() << "required array \"rooms\" is empty or doesn't exist";

        auto groups_ = object["groups"].toArray();
        groups.setStringList(toStringList("groups", groups_));
        if (groups.stringList().empty())
            qDebug() << "required array \"groups\" is empty or doesn't exist";

        auto classes_ = object["classes"].toArray();
        classes.setStringList(toStringList("classes", classes_));
        if (classes.stringList().empty())
            qDebug() << "required array \"classes\" is empty or doesn't exist";

        auto teachers_ = object["teachers"].toArray();
        teachers.setStringList(toStringList("teachers", teachers_));
        if (teachers.stringList().empty())
            qDebug() << "required array \"teachers\" is empty or doesn't exist";

        // activity from JSON also needs lists of entries that are allowed to exist
        schedule->activitiesFromJson(
            object["activities"].toArray(),
            rooms.stringList(),
            groups.stringList(),
            classes.stringList(),
            teachers.stringList()
        );
        return true;
    }

    bool saveJson(QString const& filename) {
        auto object          = QJsonObject {};
        object["rooms"]      = QJsonArray::fromStringList(rooms.stringList());
        object["groups"]     = QJsonArray::fromStringList(groups.stringList());
        object["classes"  ]  = QJsonArray::fromStringList(classes.stringList());
        object["teachers"]   = QJsonArray::fromStringList(teachers.stringList());
        object["activities"] = static_cast<ScheduleModel*>(ui->tableView->model())->activitiesToJson();

        auto file = QFile { filename };
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << QStringLiteral("couldn't open %1 for writing").arg(filename);
            return false;
        }
        file.write(QJsonDocument(object).toJson());
        return true;
    }

private:
    Ui::MainWindow *ui;

    QStringListModel rooms, groups, classes, teachers;
};

#endif // MAINWINDOW_H
