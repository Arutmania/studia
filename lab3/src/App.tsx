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

  if (!state.room && state.rooms.length)
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
    schedule[time][day] = updated!;

    return { 
      ...state, 
      schedules: new Schedules(state.schedules).set(state.room, schedule) //[
      //   ...schedule.slice(0, time),
      //   [
      //     ...schedule[time].slice(0, day),
      //     updated,
      //     ...schedule.slice(day)
      //   ],
      //   schedule.slice(time)
      // ] as Schedule)
    };
  });

  type SlotEditor = { room: string, time: number, day: number };
  type ListEditor = { list: string[], save: (list: string[]) => void };
  type Editor = SlotEditor | ListEditor | undefined;

  const isSlotEditor = (e: Editor): e is SlotEditor => {
    return e !== undefined && ['room', 'time', 'day'].every(p => p in e);
  }

  const isListEditor = (e: Editor): e is ListEditor => {
    return e !== undefined && 'list' in e && 'save' in e;
  }

  let [editing, setEditing] = useState<Editor>();
  const cancel = () => setEditing(undefined);

  const schedule = state.schedules.getSchedule(state.room!);

  const Navbar = () => {
    const setGroups   = (groups:   string[]) => setState(state => ({ ...state, groups }));
    const setLectures = (lectures: string[]) => setState(state => ({ ...state, lectures }));
    const setRooms    = (rooms:    string[]) => setState(state => ({ ...state, rooms }));
    const setTeachers = (teachers: string[]) => setState(state => ({ ...state, teachers }));

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
            onClick={() => setEditing({ list: state.groups,   save: setGroups })}
          >groups</button>
          <button 
            className="btn btn-dark"
            onClick={() => setEditing({ list: state.lectures, save: setLectures })}
          >lectures</button>
          <button 
            className="btn btn-dark"
            onClick={() => setEditing({ list: state.rooms,    save: setRooms })}
          >rooms</button>
          <button 
            className="btn btn-dark"
            onClick={() => setEditing({ list: state.teachers, save: setTeachers })}
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