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

    auto w = MainWindow {};
    w.show();

    return a.exec();
}
