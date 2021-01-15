import { useState } from 'react';

interface Props {
    list:   string[];
    save:   (list: string[]) => void;
    cancel: () => void;
}

export default function ListEdit(props: Props) {
    // unique nonempty sorted
    const filter = (list: string[]) => [...list].sort().filter(
        (e, i, a) => i === a.length || a[i + 1] !== e && e !== ''
    );

    let [list,  setList]  = useState(filter(props.list));
    let [input, setInput] = useState('');

    const add = (elem: string) => setList(state => {
        return filter([elem, ...state]);
    });

    const remove = (elem: string) => setList(state => {
        const index = state.indexOf(elem);
        if (index > -1) 
            return [...state.slice(0, index), ...state.slice(index + 1)];
        else
            return state;
    });

    const cancel = props.cancel;
    const save = () => {
        props.save(list);
        cancel();
    };

    return (
        <div className="center container table-responsive">
            <table className="table table-stripped">
                {list.map(elem => (
                    <tr>
                        <td key={elem}>{elem}</td>
                        <td key={`delete-${elem}`}>
                            <button 
                                className="btn btn-sm btn-danger btn-block"
                                onClick={() => remove(elem)}
                            >
                                remove
                            </button>
                        </td>
                    </tr>
                ))}
                <tr>
                    <td>
                        <input 
                            className="form-control"
                            type="text"
                            value={input}
                            onChange={(e) => setInput(e.target.value)}
                        />
                    </td>
                    <td>
                        <button 
                            className="btn btn-sm btn-success btn-block"
                            disabled={input === ''}
                            onClick={() => {
                                add(input);
                                setInput('');
                            }}
                        >
                            add
                        </button>
                    </td>
                </tr>
                <tr>
                    <td colSpan={2}>
                        <div className="row">
                            <div className="col-sm-6">
                                <button 
                                    className="btn btn-sm, btn-primary btn-block"
                                    onClick={save}
                                >
                                    save
                                </button>
                            </div>
                            <div className="col-sm-6">
                                <button 
                                    className="btn btn-sm, btn-warning btn-block"
                                    onClick={cancel}
                                >
                                    cancel
                                </button>
                            </div>
                        </div>
                    </td>
                </tr>
            </table>
        </div>
    )
}