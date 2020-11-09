/*#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListView>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // todo to potencjalnie deleteuje coś co nie było newowane
    ui->comboBox->setModel(&rooms);

    connect(ui->action_Rooms, &QAction::triggered, this, [this] {
        auto window = new QWidget {};
        auto horizontal = new QVBoxLayout {};
        auto list = new QListView {};
        list->setModel(&rooms);
        horizontal->addWidget(list);
        auto vertical = new QHBoxLayout {};
        vertical->addWidget(new QPushButton { "&New" });
        vertical->addWidget(new QPushButton { "&Delete" });
        auto buttons = new QWidget {};
        buttons->setLayout(vertical);
        horizontal->addWidget(buttons);
        window->setLayout(horizontal);
        window->show();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
*/
