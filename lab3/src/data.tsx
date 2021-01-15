export const days = [
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
];

export const times = [
    "08:00-08:45", "08:55-09:40", "09:50-10:35",
    "10:55-11:40", "11:50-12:35", "12:45-13:30",
    "13:40-14:25", "14:35-15:20", "15:30-16:15",
];

export class Slot {
    group:   string = '';
    teacher: string = '';
    lecture: string = '';
}

export type Schedule = Slot[][];

export class Schedules extends Map<string, Schedule> {
    getSchedule = (room: string) => {
        if (!this.has(room)) {
            this.set(
                room,
                [...new Array(times.length)].map(() => new Array(days.length))
            );
        }
        return this.get(room)!;
    }
}

export class State {
    lectures: string[] = [];
    groups:   string[] = [];
    teachers: string[] = [];
    rooms:    string[] = [];

    room?: string;
    schedules: Schedules = new Schedules();

    static toJson(state: State): string {
        let json: { [key: string]: any } = {};

        json.lectures = state.lectures;
        json.groups = state.groups;
        json.teachers = state.teachers;
        json.rooms = state.rooms;
        json.activities = [];

        state.schedules.forEach((schedule, room) => {
            schedule.forEach((t, i) => t.forEach((s, j) => {
                json.activities.push({
                    ...s, slot: i, day: j, room
                });
            }))
        });

        return JSON.stringify(json);
    }

    static fromJson(json: string): State {
        try {
            let state = new State();
            const data = JSON.parse(json);

            for (const key of ['lectures', 'groups', 'rooms', 'teachers']) {
                if (!Array.isArray(data[key]))
                    throw new Error(`array '${key}' is required`);
                state[key as keyof typeof state] = data[key];
            }

            if (!Array.isArray(data.activities))
                throw new Error(`array 'activities' is required`);
            for (const activity of data.activities) {
                let { lecture, day, group, room, slot, teacher } = activity;
                if (!state.lectures.includes(lecture))
                    throw new Error(`invalid lecture '${lecture}'`);
                if (!Number.isInteger(day) || 0 > day || day >= days.length)
                    throw new Error(`invalid day: '${day}' - must be integer in [0, 5)`);
                if (!state.groups.includes(group))
                    throw new Error(`invalid group '${group}'`);
                if (!state.rooms.includes(room))
                    throw new Error(`invalid room: '${room}'`);
                if (!Number.isInteger(slot) || 0 > slot || slot >= times.length)
                    throw new Error(`invalid slot: '${slot}' - must be integer in [0, 9)`);
                if (!state.teachers.includes(teacher))
                    throw new Error(`invalid teacher '${teacher}'`);

                state.schedules.getSchedule(activity.room)[slot as number][day as number] = {
                    group, teacher, lecture
                };
            }

            return state;
        } catch (e) {
            console.error(e);
            return new State();
        }
    }

    static async fetchState() {
        return fetch('http://localhost:8080/data')
            .then(response => response.text())
            .then(json => State.fromJson(json));
    }
}