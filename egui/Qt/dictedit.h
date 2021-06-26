#ifndef DICTEDIT_H
#define DICTEDIT_H

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>

class DictEdit : public QDialog {
    Q_OBJECT

    QListView* view;
    QStringListModel* originalModel;
    QPushButton* add, * del, * sav;

public:
    DictEdit(QStringListModel* model, QWidget* parent = nullptr)
        : QDialog(parent)
        , view(new QListView { this })
        , originalModel(model)
        , add(new QPushButton { "&Add" ,   this })
        , del(new QPushButton { "&Delete", this })
        , sav(new QPushButton { "&Save",   this })
    {
        view->setModel(new QStringListModel { originalModel->stringList(), this });

        auto layout = new QVBoxLayout { this };
        layout->addWidget(view);

        auto box = new QWidget { this };
        auto buttons = new QHBoxLayout { box };

        connect(add, &QPushButton::clicked, view, [this] {
            auto model = static_cast<QStringListModel*>(view->model());
            if (model->insertRow(model->rowCount())) {
                auto index = model->index(model->rowCount() - 1);
                view->setCurrentIndex(index);
                view->edit(index);
            }
            del->setEnabled(!model->stringList().empty());
        });
        buttons->addWidget(add);

        connect(del, &QPushButton::clicked, view, [this] {
            auto index = view->currentIndex();
            auto model = static_cast<QStringListModel*>(view->model());
            if (index.isValid() && index.row() < model->stringList().size())
                model->removeRows(index.row(), 1);
            del->setEnabled(!model->stringList().empty());
        });
        del->setEnabled(!model->stringList().empty());
        buttons->addWidget(del);

        connect(sav, &QPushButton::clicked, this, [this] {
           auto model = static_cast<QStringListModel*>(view->model());
           // ghetto remove duplicates
           auto list = model->stringList();
           auto set = QSet<QString> { list.begin(), list.end() };
           list = QStringList { set.begin(), set.end() };
           // remove empty
           list.removeAll(QString {});
           list.sort();
           originalModel->setStringList(list);
           accept();
        });
        buttons->addWidget(sav);

        box->setLayout(buttons);
        layout->addWidget(box);
        setLayout(layout);
    }
};

#endif // DICTEDIT_H
