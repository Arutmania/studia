#include "mainwindow.h"
#include "dictedit.h"

#include <QApplication>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QStringListModel>


auto main(int argc, char *argv[]) -> int {
    auto a = QApplication { argc, argv };

    //auto list = QStringList {};
    //list << "123" << "456" << "789";
    //auto model = new QStringListModel { list };
    //auto de = DictEdit { model };
    //de.show();

    auto w = MainWindow {};
    w.show();

    return a.exec();
}

/*
auto main (int argc, char* argv[]) -> int {
    auto a = QApplication(argc, argv);

    auto splitter = new QSplitter {};
    auto filesystem = new QFileSystemModel {};

    filesystem->setRootPath(QDir::currentPath());

    auto tree = new QTreeView(splitter);
    tree->setModel(filesystem);
    tree->setRootIndex(filesystem->index(QDir::currentPath()));

    auto list = new QListView(splitter);
    list->setModel(filesystem);
    list->setRootIndex(filesystem->index(QDir::currentPath()));
    list->setSelectionModel(tree->selectionModel());

    splitter->setWindowTitle("Two views onto the same file system model");
    splitter->show();
    return a.exec();
}
*/
