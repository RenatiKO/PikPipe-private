import { Portal } from '../../elements/Portal';
import './Loader.css';

export const Loader = () => {
  return (
    <Portal>
      <div className='loader'>
        <div className='loader__content' />
      </div>
    </Portal>
  )
}