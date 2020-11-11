#ifndef ENTRYEDIT_H
#define ENTRYEDIT_H

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringListModel>

#include "schedule.h"

class EntryEdit : public QDialog {
    Q_OBJECT

    ScheduleModel* schedule_;
    QModelIndex const& index_;

    QStringListModel* groups_;
    QStringListModel* classes_;
    QStringListModel* teachers_;

    QComboBox* group_;
    QComboBox* class_;
    QComboBox* teacher_;

    auto canSave() const -> bool {
        // all set or none set
        return (group_->currentIndex() 	 != -1  &&
                class_->currentIndex() 	 != -1  &&
                teacher_->currentIndex() != -1) ||
               (group_->currentIndex()   == -1  &&
                class_->currentIndex()   == -1  &&
                teacher_->currentIndex() == -1);
    }

public:
    EntryEdit(ScheduleModel* schedule,
              QModelIndex const& index,
              QStringListModel* groups,
              QStringListModel* classes,
              QStringListModel* teachers,
              QWidget* parent = nullptr)
        : QDialog(parent)
        , schedule_(schedule)
        , index_(index)
        , groups_(groups)
        , classes_(classes)
        , teachers_(teachers)
        , group_(new QComboBox { this })
        , class_(new QComboBox { this })
        , teacher_(new QComboBox { this })
    {
        group_->setModel(groups_);
        class_->setModel(classes_);
        teacher_->setModel(teachers_);

        {
            auto entry = schedule->entry(index.row(), index.column());
            if (entry.isEmpty()) {
                group_->setCurrentIndex(-1);
                class_->setCurrentIndex(-1);
                teacher_->setCurrentIndex(-1);
            } else {
                group_->setCurrentIndex(group_->findText(entry.group_));
                class_->setCurrentIndex(class_->findText(entry.class_));
                teacher_->setCurrentIndex(teacher_->findText(entry.teacher_));
            }
        }

        auto g = new QWidget { this };
        {
            auto layout = new QHBoxLayout { g };
            layout->addWidget(new QLabel { "group: " });
            layout->addWidget(group_);
            g->setLayout(layout);
        }
        auto c = new QWidget { this };
        {
            auto layout = new QHBoxLayout { c };
            layout->addWidget(new QLabel { "class: " });
            layout->addWidget(class_);
            c->setLayout(layout);
        }
        auto t = new QWidget { this };
        {
            auto layout = new QHBoxLayout { t };
            layout->addWidget(new QLabel { "teacher: " });
            layout->addWidget(teacher_);
            t->setLayout(layout);
        }
        auto buttons = new QWidget { this };
        {
            auto layout = new QHBoxLayout { buttons };

            auto clear 	= new QPushButton { "Clear", buttons };
            connect(clear, &QPushButton::clicked, this, [this] {
                group_->setCurrentIndex(-1);
                class_->setCurrentIndex(-1);
                teacher_->setCurrentIndex(-1);
            });

            auto save 	= new QPushButton { "Save", buttons };
            connect(save, &QPushButton::clicked, this, [this] {
                if (canSave()) {
                    schedule_->setEntry(group_->currentText(),
                                        class_->currentText(),
                                        teacher_->currentText(),
                                        index_.row(),
                                        index_.column());
                    accept();
                }
            });

            auto changedSelection = [this, save] (int) { save->setEnabled(canSave()); };
            connect(group_,   qOverload<int>(&QComboBox::currentIndexChanged), save, changedSelection);
            connect(class_,   qOverload<int>(&QComboBox::currentIndexChanged), save, changedSelection);
            connect(teacher_, qOverload<int>(&QComboBox::currentIndexChanged), save, changedSelection);
            save->setEnabled(canSave());

            auto cancel = new QPushButton { "Cancel", buttons };
            connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

            layout->addWidget(clear);
            layout->addWidget(save);
            layout->addWidget(cancel);
            buttons->setLayout(layout);
        }

        auto layout = new QVBoxLayout { this };
        layout->addWidget(g);
        layout->addWidget(c);
        layout->addWidget(t);
        layout->addWidget(buttons);
        setLayout(layout);
    }
};

#endif // ENTRYEDIT_H
