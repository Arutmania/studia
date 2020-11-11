#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QAbstractTableModel>
#include <QMap>
#include <QString>
#include <QVector>
#include <QStyledItemDelegate>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>

#include <array>

class ScheduleModel : public QAbstractTableModel {
    Q_OBJECT

public:
    struct TimeSlot {
        QString group_, class_, teacher_;
        auto isEmpty() const -> bool
        { return group_.isEmpty() && class_.isEmpty() && teacher_.isEmpty(); }
        auto clear() -> void
        { group_.clear(); class_.clear(); teacher_.clear(); }
    };

private:

    enum { NUM_DAYS = 5, NUM_TIMES = 9 };
    using Day  = std::array<TimeSlot, NUM_TIMES>;
    using Week = std::array<Day, NUM_DAYS>;

    QMap<QString, Week> schedules;

public:
    QString room;

    using QAbstractTableModel::QAbstractTableModel;

    auto setEntry(QString const& g, QString const& c, QString const& t, int row, int column) -> void {
        if (row < NUM_TIMES || column < NUM_DAYS) {
            for (auto& schedule : schedules)
                if (auto& entry = schedule[column][row]; entry.group_ == g || entry.class_ == c || entry.teacher_ == t)
                    entry.clear();
            schedules[room][column][row] = { g, c, t };
            emit dataChanged(index(row, column), index(row, column), { Qt::DisplayRole });
        }
    }

    auto entry(int row, int column) const -> TimeSlot {
        return schedules[room][column][row];
    }

    auto rowCount(QModelIndex const& parent = QModelIndex())
    const -> int override {
        Q_UNUSED(parent);
        return NUM_TIMES;
    }

    auto columnCount(QModelIndex const& parent = QModelIndex())
    const -> int override {
        Q_UNUSED(parent);
        return NUM_DAYS;
    }

    auto data(QModelIndex const& index, int role = Qt::DisplayRole)
    const -> QVariant override {
        if (!index.isValid() || index.row() > 10 || index.column() > 5)
            return QVariant {};

        if (role != Qt::DisplayRole)
            return QVariant {};

        // TODO why is this supposedly dangling (if auto const& would have been used)
        //auto const& slot = schedules[room][index.column()][index.row()];
        auto slot = schedules[room][index.column()][index.row()];

        if (slot.isEmpty())
            return QString {};
        else
            return QStringLiteral("%1: %2")
                .arg(slot.group_)
                .arg(slot.class_);
    }

    auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)
    const -> QVariant override {
        static char const* days[] = {
            "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
        };

        static char const* times[] = {
            "08:00-08-45", "08:55-09:40", "09:50-10:35",
            "10:55-11:40", "11:50-12:35", "12:45-13:30",
            "13:40-14:25", "14:35-15:20", "15:30-16:15"
        };

        if (role != Qt::DisplayRole)
            return QVariant {};

        switch (orientation) {
        case Qt::Horizontal:
            if (section < NUM_DAYS)
                return QString(days[section]);
            else
                break;
        case Qt::Vertical:
            if (section < NUM_TIMES)
                return QString(times[section]);
            else
                break;
        }
        return QVariant {};
    }

    void removeInvalidClasses(QStringList const& cs) {
        // here we could try some more elaborate logic to only update the view when necessary and only the parts that changed but why bother
        for (auto& schedule : schedules)
            for (auto& day : schedule)
                for (auto& slot : day)
                    if (!cs.contains(slot.class_))
                        slot.clear();
        emit dataChanged(index(0, 0), index(NUM_TIMES - 1, NUM_DAYS - 1), { Qt::DisplayRole  });
    }

    void removeInvalidGroups(QStringList const& gs) {
        for (auto& schedule : schedules)
            for (auto& day : schedule)
                for (auto& slot : day)
                    if (!gs.contains(slot.group_))
                        slot.clear();
        emit dataChanged(index(0, 0), index(NUM_TIMES - 1, NUM_DAYS - 1), { Qt::DisplayRole  });
    }

    void removeInvalidRooms(QStringList const& rs) {
        // TODO tutaj chyba nie muszę robić nic więcej bo room powinnien być updateowany przez combobox i też wtedy dataChanged jest emitowane
        // pretty inefficient but you know whatever and I'm pretty sure no dangling iterators
        for (auto const& key : schedules.keys())
            if (!rs.contains(key))
                schedules.remove(key);
    }

    void removeInvalidTeachers(QStringList const& ts) {
        for (auto& schedule : schedules)
            for (auto& day : schedule)
                for (auto& slot : day)
                    if (!ts.contains(slot.teacher_))
                        slot.clear();
        emit dataChanged(index(0, 0), index(NUM_TIMES - 1, NUM_DAYS - 1), { Qt::DisplayRole  });
    }

public slots:
    void setActiveRoom(QString const& r) {
        room = r;
        emit dataChanged(index(0, 0),
            index(NUM_TIMES - 1, NUM_DAYS - 1),
            { Qt::DisplayRole });
    }
};

#endif // SCHEDULE_H
