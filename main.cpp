#include "mainwindow.h"
#include "dictedit.h"

#include <QApplication>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QStringListModel>

/*
 * powtarzające się wejścia
 * wyszarzanie przycisku jak nie można edytować
 * usuwanie pustych wejść
 *
 * resizeable
 *
 * usuwanie dziejąch się w tym samych czasie klas, grup, nauczycieli itp.
 * usunięcie nauczyciela, klasy, itp. powinno wyczyścić wejścia palnera z tym nauczycielem, klasą, itp.
 *
 * wyłączenie double clicka jak nie jest wybrany pokój
 *
 * tłumaczenie, json
 *
 * entry edit otwiera się z wybranymi comboboxami,
 * jak któregoś nie ma to save się nie wyłącza
 * jeśli entry jest puste to entry edit też powinien być, jeśli już coś jest to powinien mieć to zaznaczone
 *
 * dictionary edit powinnie robić dodawanie od double clicku, enter powininien akceptować i zaczynać edycję kolejnego
 * powinien usuwać puste i duplikaty
 */

/*
 * czy table view jest o jeden za duze?
 */

auto main(int argc, char *argv[]) -> int {
    auto a = QApplication { argc, argv };

    auto w = MainWindow {};
    w.show();

    return a.exec();
}
