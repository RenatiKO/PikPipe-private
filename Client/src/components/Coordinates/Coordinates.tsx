import './Coordinates.css'

interface Props {
  x: number;
  y: number;
  z: number;
}

export const Coordinates = ({ x, y, z }: Props) => {
  return (
    <div className='coordinates'>
      <input type='text' className='dot' value={Math.round(x)} />
      <input type='text' className='dot' value={Math.round(y)} />
      <input type='text' className='dot' value={Math.round(z)} />
    </div>
  );
}