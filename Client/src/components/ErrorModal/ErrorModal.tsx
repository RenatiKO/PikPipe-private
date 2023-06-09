import { useRef } from 'react';
import './ErrorModal.css';
import { Modal } from '../../elements/Modal';

interface Props {
  setIsOpen: (value: boolean) => void;
  errors?: string[];
}

export const ErrorModal = ({
  setIsOpen,
  errors
}: Props) => {
  const modal = useRef<HTMLDivElement>(null);

  const close = () => setIsOpen(false);

  return (
    <Modal onOverlayClick={close}>
      <div ref={modal} className="errorModal">
        <span className='errorModal__title'>Ошибка</span>
        <div className='errors'>
          {errors?.map(x => <span className='error'>{x}</span>)}
        </div>
      </div>
    </Modal>
  );
}