import './Button.css';

interface ButtonProps {
  label: string;
}

export const Button = ({
  label
}: ButtonProps) => {
  return (
    <button className="button">{label}</button>
  );
}