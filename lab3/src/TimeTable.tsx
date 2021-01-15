import SlotButton from './SlotButton';
import { Schedule } from './data';
import { days, times } from './data';

interface Props {
    schedule?:  Schedule;
    getEditor: (time: number, day: number) => () => void;
    rooms: string[];
    selectedRoom?: string;
    onRoomChange: (room: string) => void;
}

export default function TimeTable(props: Props) {
    interface RoomSelectProps {
        value: string;
        rooms: string[];
        onChange: (room: string) => void;
    }

    const RoomSelect = ({ value, rooms, onChange }: RoomSelectProps) => {
        return (
            <div className="input-group">
                <div className="input-group-prepend">
                    <label className="input-group-text" htmlFor="room-select">room</label>
                </div>
                <select id="room-select"
                    className="custom-select"
                    value={value} onChange={(e) => onChange(e.target.value)}
                >
                    <option key='' disabled hidden />
                    {rooms.map(r => <option key={r} value={r}>{r}</option>)}
                </select>
            </div>
        )
    }

    const Head = () => (
        <thead>
            <tr>
                <th className="text-center" key="room-select">
                    <RoomSelect 
                        value={props.selectedRoom ?? ''} 
                        rooms={props.rooms} 
                        onChange={props.onRoomChange}
                    />
                </th>
                {days.map(d => <th className="text-center" key={d}>{d}</th>)}
            </tr>
        </thead>
    );

    const display = (time: number, day: number) => {
        if (props.schedule)
            // error cannot read property 'group' of undefined
            return props.schedule![time]?.[day]?.group ?? ''
        else
            return ''
    }

    const Body = () => (
        <tbody>
        {times.map((time, timeIndex) => (
            <tr key={time}>
                <th key={time}>{time}</th>
                {days.map((day, dayIndex) => (
                    <td className="text-center" key={day}>
                        <SlotButton 
                            display={display(timeIndex, dayIndex)} 
                            edit={props.getEditor(timeIndex, dayIndex)} 
                        />
                    </td>
                ))}
            </tr>
        ))}
        </tbody>
    )

    return (
        <div className="center container-fluid table-responsive">
            <table className="table table-default table-fixed">
                <Head />
                <Body />
            </table>
        </div>
    );
  };