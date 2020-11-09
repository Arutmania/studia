#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QAbstractTableModel>
#include <QMap>
#include <QString>
#include <QVector>

#include <array>

// model should hold a schedule
// selecting different room will change the schedule being held
// by the model and send the update signal
class ScheduleModel : public QAbstractTableModel {
    Q_OBJECT
    // map from room to schedule
    struct TimeSlot {
        QString group_, class_, teacher_;
        auto isEmpty() const -> bool
        { return group_.isEmpty() && class_.isEmpty() && teacher_.isEmpty(); }
        auto clear() -> void
        { group_.clear(); class_.clear(); teacher_.clear(); }
    };


    // TODO selecting in the drop down the room should signal this to change the selected room
    QString room;

    enum { NUM_DAYS = 5, NUM_TIMES = 10 };
    using Day  = std::array<TimeSlot, NUM_TIMES>;
    using Week = std::array<Day, NUM_DAYS>;

    QMap<QString, Week> schedules;

public:

    using QAbstractTableModel::QAbstractTableModel;

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

        // TODO is this dangling - i don't think it is
        auto const& slot = schedules[room][index.column()][index.row()];

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

public slots:
    void setActiveRoom(QString const& r) {
        room = r;
        emit dataChanged(index(0, 0),
             index(NUM_TIMES - 1, NUM_DAYS - 1),
             { Qt::DisplayRole });
    }
};

#endif // SCHEDULE_H
