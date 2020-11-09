#ifndef DICTEDIT_H
#define DICTEDIT_H

#include <QDialog>
#include <QStringListModel>
#include <QListView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>

class DictEdit : public QDialog {
    Q_OBJECT

    QListView* view;

public:
    DictEdit(QStringListModel* model, QWidget* parent = nullptr)
        : QDialog(parent)
        , view(new QListView {}){
        view->setModel(model);
        //view->setResizeMode(QListView::ResizeMode::Adjust);

        auto layout = new QVBoxLayout {};
        layout->addWidget(view);

        auto buttons = new QHBoxLayout {};

        auto add = new QPushButton { "&Add" };
        connect(add, &QPushButton::clicked, view, [this] {
            auto model = static_cast<QStringListModel*>(view->model());
            if (model->insertRow(model->rowCount())) {
                auto index = model->index(model->rowCount() - 1);
                //model->setData(index, "");
                view->setCurrentIndex(index);
                view->edit(index);
            }
        });
        buttons->addWidget(add);

        // TODO grey out the delete button if there is nothing to delete
        auto del = new QPushButton { "&Delete" };
        connect(del, &QPushButton::clicked, view, [this] {
            auto index = view->currentIndex();
            auto model = static_cast<QStringListModel*>(view->model());
            if (index.isValid() && index.row() < model->stringList().size())
                model->removeRows(index.row(), 1);
        });
        buttons->addWidget(del);

        auto can = new QPushButton { "&Cancel" };
        connect(can, &QPushButton::clicked, this, &QDialog::reject);
        buttons->addWidget(can);

        auto box = new QWidget {};
        box->setLayout(buttons);
        layout->addWidget(box);
        setLayout(layout);

        //adjustSize();
    }
};

#endif // DICTEDIT_H
