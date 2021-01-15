interface Props {
    display?: string;
    edit: () => void;
}

export default function SlotButton(props: Props) {
    return (
        <button 
            className="btn btn-block" 
            onClick={props.edit}
        >{props.display}</button>
    );
}