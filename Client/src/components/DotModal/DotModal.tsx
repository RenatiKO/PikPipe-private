import { useState, ChangeEvent, useRef, useLayoutEffect } from 'react';
import { Dot, Dot2d, Tag } from '../../types';
import './DotModal.css';
import { Modal } from '../../elements/Modal';

interface Props {
  setIsOpen: (value: boolean) => void;

  x: number;
  y: number;
  addDot: (dot: Dot) => void;
  clientDot?: Dot2d;
}

export const DotModal = ({
  setIsOpen,
  x: propsX, y: propsY,
  addDot: propsAddDot,
  clientDot
}: Props) => {
  const modal = useRef<HTMLDivElement>(null);

  const [x, setX] = useState<string>(propsX.toString());
  const [y, setY] = useState<string>(propsY.toString());
  const [z, setZ] = useState<string | undefined>(undefined);

  const changeX = (e: ChangeEvent<HTMLInputElement>) => setX(e.target.value);
  const changeY = (e: ChangeEvent<HTMLInputElement>) => setY(e.target.value);
  const changeZ = (e: ChangeEvent<HTMLInputElement>) => setZ(e.target.value);

  const close = () => setIsOpen(false);

  const [tag, setTag] = useState<Tag | undefined>(undefined)
  const onTagChange = (e: ChangeEvent<HTMLSelectElement>) => setTag(e.target.value as Tag);

  const addDot = () => {
    if (z === undefined) return;
    if (tag === undefined) return;

    const dot: Dot = { x: +x, y: +y, z: +z, tag };

    propsAddDot(dot);
    close();
  }

  useLayoutEffect(() => {
    if (modal.current && clientDot) {
      const { width, height } = modal.current.getBoundingClientRect();

      const keyframes = new KeyframeEffect(
        modal.current,
        [
          { top: `${clientDot.y - height/2}px`, left: `${clientDot.x - width/2}px`, transform: 'scale(0.5)' },
          { top: `${clientDot.y - height/2}px`, left: `${clientDot.x - width/2}px`, transform: 'scale(1)' },
        ],
        { duration: 100, easing: 'ease', fill: 'forwards' }
      );

      const animation = new Animation(keyframes, document.timeline);
      animation.play();
    }
  }, [])

  const disabled = z === undefined || z === '' || tag === undefined;

  return (
    <Modal onOverlayClick={close} onAgree={addDot}>
      <div ref={modal} className="modal__content">
        <div className="modal__item">
          <span>x</span><input type='text' value={x ?? propsX} onChange={changeX} /><span>мм</span>
        </div>
        <div className="modal__item">
          <span>y</span><input type='text' value={y ?? propsY} onChange={changeY} /><span>мм</span>
        </div>
        <div className="modal__item">
          <span>z</span><input type='text' value={z ?? ''} onChange={changeZ} /><span>мм</span>
        </div>
        <div className="modal__item">
          <span>tag</span>
          <select value={tag ?? ''} onChange={onTagChange}>
            <option hidden value=''></option>
            <option>{Tag.RISER}</option>
            <option>{Tag.TOILET}</option>
            <option>{Tag.SINK}</option>
            <option>{Tag.BATH}</option>
            <option>{Tag.SHOWER}</option>
          </select>
        </div>
        <button disabled={disabled} onClick={addDot}>Добавить</button>
      </div>
    </Modal>
  );
}