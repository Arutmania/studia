import { useState } from 'react';
import { Slot } from './data'

interface Props {
    room:     string;
    value?:   Slot;
    groups:   string[];
    lectures: string[];
    teachers: string[];

    save:   (slot: Slot) => void;
    clear:  () => void;
    cancel: () => void;
}

export default function SlotEdit(props: Props) {
    let [group,   setGroup]   = useState(props.value?.group   ?? '');
    let [lecture, setLecture] = useState(props.value?.lecture ?? '');
    let [teacher, setTeacher] = useState(props.value?.teacher ?? '');
    
    interface SelectProps {
        id:       string;
        value?:   string;
        list:     string[];
        onChange: (event: React.ChangeEvent<HTMLSelectElement>) => void;
    }

    const Select = ({ id, value, list, onChange }: SelectProps) => {
        return (
            <select className="custom-select col-sm-4" id={id} value={value} onChange={onChange}>
                <option disabled hidden key='' />
                {list.map(elem => <option value={elem} key={elem}>{elem}</option>)}
            </select>
        );
    };

    // const clear = () => [setGroup, setLecture, setTeacher].forEach(f => f(''));
    // props.save()

    const handle = (f: any) => {
        return (e: React.ChangeEvent<HTMLSelectElement>) => f(e.target.value);
    };

    return (
        <form className="center container container-fluid form-horizontal">
            <label>{props.room}</label>
            <div className="form-group">
                <label className="control-label col-sm-2" htmlFor="group">group</label>
                <Select id="group" value={group} list={props.groups} onChange={handle(setGroup)} />
            </div>

            <div className="form-group">
                <label className="control-label col-sm-2" htmlFor="lecture">lecture</label>
                <Select id="lecture" value={lecture} list={props.lectures} onChange={handle(setLecture)} />
            </div>

            <div className="form-group">
                <label className="control-label col-sm-2" htmlFor="teacher">teacher</label>
                <Select id="teacher" value={teacher} list={props.teachers} onChange={handle(setTeacher)} />
            </div>

            <div className="form-group">
                <button 
                    className="btn btn-sm btn-success col-sm-2" 
                    onClick={() => props.save({ group, lecture, teacher })}
                    disabled={!group || !lecture || !teacher}
                >save</button>
                <button
                    className="btn btn-sm btn-danger col-sm-2"
                    onClick={() => props.clear()}
                >clear</button>
                <button
                    className="btn btn-sm btn-warning col-sm-2"
                    onClick={props.cancel}
                >cancel</button>
            </div>
        </form>
    )
}
