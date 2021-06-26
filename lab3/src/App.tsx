import { useState, useEffect }              from 'react';
import SlotEdit                             from './SlotEdit';
import TimeTable                            from './TimeTable'
import { State, Schedule, Schedules, Slot, times, days } from './data'
import ListEdit                             from './ListEdit';

export default function App() {
  let [state, setState] = useState(new State());
  useEffect(() => {
    const f = async () => {
      setState(await State.fetchState())
    }
    f();
  }, []);

  const setRoom = (room: string) => setState(state => ({ ...state, room }));

  // if room not set and there are rooms - if there are no rooms setting it to
  // undefined will cause infinite rerender
  if (!state.room && state.rooms.length)
    setRoom(state.rooms[0]);

  // if room is set but invalid set it to first room from rooms
  if (state.room && !state.rooms.includes(state.room!))
    setRoom(state.rooms[0]);

  const setSlot = (time: number, day: number, updated?: Slot) => setState(state => {
    if (!state.room)
      return state;
    
    if (0 > time || time >= times.length)
      return state;
    
    if (0 > day || day >= days.length)
      return state;

    const schedule = state.schedules.getSchedule(state.room);
    // unfortunately can't do cool immutable upgrades because they shrink
    // the array and I don't know what to do with it
    // updated could actually be undefined but that's what we want
    if (updated === undefined)
        delete schedule[time][day];
    else
        schedule[time][day] = updated!;
    //schedule[time][day] = updated!;

    return { 
      ...state, 
      schedules: new Schedules(state.schedules).set(state.room, schedule) //[
      //  ...schedule.slice(0, time),
      //  [
      //    ...schedule[time].slice(0, day),
      //    updated,
      //    ...schedule.slice(day + 1)
      //  ],
      //  ...schedule.slice(time + 1)
      //] as Schedule)
    };
  });

  type SlotEditor = { room: string, time: number, day: number };
  type ListEditor = { name: string, list: string[], save: (list: string[]) => void };
  type Editor = SlotEditor | ListEditor | undefined;

  const isSlotEditor = (e: Editor): e is SlotEditor => {
    return e !== undefined && ['room', 'time', 'day'].every(p => p in e);
  }

  const isListEditor = (e: Editor): e is ListEditor => {
    return e !== undefined && ['name', 'list', 'save'].every(p => p in e);
  }

  let [editing, setEditing] = useState<Editor>();
  const cancel = () => setEditing(undefined);

  const schedule = state.schedules.getSchedule(state.room!);

  const Navbar = () => {
    const removeBy = (pred: (slot: Slot) => boolean) => {
        return setState(state => ({ 
            ...state, 
            schedules: new Schedules(
                Array.from(state.schedules).map(([room, schedule]) => {
                    schedule.forEach((time, timeIndex) => time.forEach((slot, slotIndex) => {
                        if (pred(slot))
                            delete schedule[timeIndex][slotIndex];
                    }));
                    return [room, schedule];
                    //return [
                    //    room,
                    //    schedule.map(time => time.map(slot => {
                    //        if (!pred(slot))
                    //            return slot;
                    //        else
                    //            return undefined;
                    //    })) as Schedule
                    //];
                })
            )
        }));
    }
      // TODO: tu potrzeba też usunąć sloty i updatować selected room
    //const setGroups   = (groups:   string[]) => setState(state => ({ ...state, groups }));
    const setGroups   = (groups:   string[]) => {
        setState({ ...state, groups });
        removeBy(slot => !state.groups.includes(slot.group));
    }
    //const setLectures = (lectures: string[]) => setState(state => ({ ...state, lectures }));
    const setLectures = (lectures: string[]) => {
        setState({ ...state, lectures });
        removeBy(slot => !state.lectures.includes(slot.lecture));
    }
    //const setLectures = (lectures: string[]) => setState(state => {
    //    return { 
    //        ...state, 
    //        lectures, 
    //        schedules: new Schedules(
    //            Array.from(state.schedules).map(([room, schedule]) => {
    //                return [
    //                    room,
    //                    schedule.map(time => time.map(slot => {
    //                        if (state.lectures.includes(slot.lecture))
    //                            return slot;
    //                        else
    //                            return undefined;
    //                    }))
    //                ];
    //            })
    //        )
    //    };
    //});


    const setRooms    = (rooms:    string[]) => setState(state => {
        let schedules = new Schedules(state.schedules);
        for (const room of state.rooms)
            if (!rooms.includes(room))
                schedules.delete(room);
        // if selected room is invalid it will be fixed at the start of App
        return { ...state, schedules, rooms };
    });
    //const setTeachers = (teachers: string[]) => setState(state => ({ ...state, teachers }));
    const setTeachers = (teachers: string[]) => {
        setState({ ...state, teachers });
        removeBy(slot => !state.teachers.includes(slot.teacher));
    }

    return (
      <nav className="navbar navbar-dark bg-dark">
        <a className="navbar-brand" onClick={cancel} href="/"><b>planner</b></a>
        <span className="btn-group">
          <a 
            className="btn btn-dark"
            href="/" 
            onClick={async () => {
              setState(await State.fetchState());
              cancel();
            }}
          >load</a>
          <a 
            className="btn btn-dark"
            href="/"
            onClick={async () => {
              fetch('http://localhost:8080/data', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: State.toJson(state)
              });
              cancel();
            }}
          >save</a>
        </span>
        <span className="btn-group">
          <button 
            className="btn btn-dark"
            onClick={() => {
                cancel()
                setEditing({ name: 'groups', list: [...state.groups],   save: setGroups });
            }}
          >groups</button>
          <button 
            className="btn btn-dark"
            onClick={() => {
                cancel();
                setEditing({ name: 'lectures', list: [...state.lectures], save: setLectures });
            }}
          >lectures</button>
          <button 
            className="btn btn-dark"
            onClick={() => {
                cancel();
                setEditing({ name: 'rooms', list: [...state.rooms],    save: setRooms });
            }}
          >rooms</button>
          <button 
            className="btn btn-dark"
            onClick={() => {
                cancel();
                setEditing({ name: 'teachers', list: [...state.teachers], save: setTeachers });
            }}
          >teachers</button>
        </span>
      </nav>
    );
  };

  if (isSlotEditor(editing)) {  
    const save = (slot: Slot) => setEditing(editing => {
      if (isSlotEditor(editing))
        setSlot(editing.time, editing.day, slot);

      return undefined;
    })

    const clear = () => setEditing(editing => {
      if (isSlotEditor(editing))
        setSlot(editing.time, editing.day, undefined);
      return undefined;
    });

    const room = state.room!;

    const value = schedule[editing.time][editing.day];
    return (
      <>
        <Navbar />
        <SlotEdit
          room={room}
          value={value}
          groups={state.groups}
          lectures={state.lectures}
          teachers={state.teachers}
          save={save}
          clear={clear}
          cancel={cancel}
        />
      </>
    );
  } else if(isListEditor(editing)) {
    return (
      <>
        <Navbar />
        <ListEdit
          name={editing.name}
          list={editing.list}
          save={editing.save}
          cancel={cancel}
        />
      </>
    )
  } else {
    const editor = (time: number, day: number) => {
      if (state.room)
        setEditing({ time, day, room: state.room! });
    }
    return (
      <>
        <Navbar />
        <TimeTable 
          schedule={schedule} 
          getEditor={(time, day) => () => editor(time, day)}
          rooms={state.rooms}
          selectedRoom={state.room}
          onRoomChange={setRoom}
        />
      </>
    );
  }
}
