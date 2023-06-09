import { PropsWithChildren, useRef, useEffect, useCallback, KeyboardEvent, MouseEvent } from "react";
import { Portal } from "./Portal";
import './Modal.css'

interface ModalProps {
  onOverlayClick?: () => void;
  onAgree?: () => void;
}

export const Modal = ({ 
  onOverlayClick: propsOnOverlayClick,
  onAgree,
  children 
}: PropsWithChildren<ModalProps>) => {
  const modalRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    modalRef.current?.focus();
  }, []);

  const onOverlayClick = useCallback((e: MouseEvent) => {
    if (e.target === e.currentTarget) {
      propsOnOverlayClick?.();   
    }
  }, [propsOnOverlayClick])

  const onKeyDown = useCallback((e: KeyboardEvent<HTMLDivElement>) => {
    if (e.keyCode === 27) {
      return propsOnOverlayClick?.();
    }

    if (e.key === 'Enter') {
      return onAgree?.()
    }
  }, [propsOnOverlayClick, onAgree]);

  return (
    <Portal>
      <div tabIndex={0} ref={modalRef} onClick={onOverlayClick} onKeyDown={onKeyDown} className="modal">
        {children}
      </div>
    </Portal>
  );
}