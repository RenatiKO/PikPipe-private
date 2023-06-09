import { ChangeEvent, useContext, useState } from 'react';
import { AppContext } from '../../context';
import { Dot, Dot2d, Node, NodeDto, NodeFitting, Tag, UploaderFile } from '../../types';
import { isEvtWithFiles } from '../../utils';
import { Coordinates } from '../Coordinates';
import { Loader } from '../Loader';
import { api } from './api';
import './Menu.css'

interface MenuProps {
  origin: Dot2d | null;
  min: Dot2d | null;
  dots: Dot[];
  grid: [number, number];
  onChangeGrid: (net: [number, number]) => void;
  removeDot: (index: number) => () => void;
  isHideSizes: boolean;
  toggleIsHideSizes: (value: boolean) => void;
  isHideAxis: boolean;
  toggleIsHideAxis: (value: boolean) => void;
  onChangeNodes: (nodes: Node[]) => void;
  onChangeFittings: (fittings: NodeFitting[]) => void;
  onChangeFittingsActive: (value: boolean) => void;
  clearLabels: () => void;
  isClearable?: boolean;
  clear: () => void;
  errors: string[];
  openErrorModal: () => void;
}

export const Menu = ({
  origin,
  min,
  dots,
  grid,
  onChangeGrid,
  removeDot,
  isHideSizes,
  toggleIsHideSizes: propsToggleIsHideSizes,
  isHideAxis,
  toggleIsHideAxis: propsToggleIsHideAxis,
  onChangeNodes,
  onChangeFittings,
  onChangeFittingsActive,
  clearLabels,
  isClearable,
  clear,
  errors,
  openErrorModal,
}: MenuProps) => {

  const { setFile, file } = useContext(AppContext);

  const handleChangeGrid = (e: ChangeEvent<HTMLSelectElement>) => {
    const grid = e.target.value.split('x').map(x => +x) as [number, number];

    onChangeGrid(grid);
  }

  const onUpload: React.ChangeEventHandler<HTMLInputElement> = (event) => {
    event.preventDefault()
    if (!isEvtWithFiles(event)) {
      return
    }

    const newFiles: UploaderFile[] = Array.from(event.target.files!).map((f) => ({ file: f }))
    const file = newFiles[0];

    setFile?.(file.file);

    event.target.value = ''
  }

  const toggleIsHideSizes = (e: ChangeEvent<HTMLInputElement>) => propsToggleIsHideSizes(e.target.checked);
  const toggleIsHideAxis = (e: ChangeEvent<HTMLInputElement>) => propsToggleIsHideAxis(e.target.checked);

  const [isLoad, setIsLoad] = useState<boolean>(false);

  //расчет
  const calculate = async () => {
    if (errors.length) {
      openErrorModal();
      return;
    }

    if (!origin) {
      console.warn('Неопределен origin');
      return;
    }

    if (!min) {
      console.warn('Неопределен размер чертежа');
      return;
    }

    if (!file) {
      console.warn('Отсутствует файл чертежа');
      return;
    }

    clearLabels();
    const normalDots = dots
      .map(x => ({ 
        ...x, 
        x: x.x + origin.x, 
        y: x.y + origin.y,
        tag: mapTag(x.tag)
    }));

    const formData = new FormData();
    formData.append('dots', JSON.stringify(normalDots));
    formData.append('file', file);

    setIsLoad(true);
    try {
      const response = await api.calc(formData);
      if (response) {
        console.log(response)

        const nodes = parseNodes(JSON.parse(response[0]), origin, min);
        const fittings: NodeFitting[] = 
          JSON.parse(response[1]).map((x: NodeFitting) => {
            const dot = nodes.find(node => +x.Node === +node.id);
    
            return { ...x, x: dot?.from.x, y: dot?.from.y, z: dot?.from.z };
          })
        
        onChangeNodes(nodes)
        onChangeFittings(fittings)
        onChangeFittingsActive(true);
        setIsLoad(false)
      }
    } catch(e) {
      console.log(e);
      setIsLoad(false)
      openErrorModal()
    }
  }

  return (
    <>
      <div className="menu">
        <div className='menu__options'>
          <span className='logo'>pipey</span>
          <div className='menu__item row row__space'>
            <span className='menu__title'>Сетка</span>
            <select onChange={handleChangeGrid} value={grid.join('x')}>
              <option>1x5</option>
              <option>2x10</option>
              <option>4x16</option>
            </select>
          </div>
          <div className='menu__item row row__space'>
            <span className='menu__title'>Скрыть размеры*</span>
            <input type='checkbox' checked={isHideSizes} onChange={toggleIsHideSizes} />
          </div>
          <div className='menu__item row row__space'>
            <span className='menu__title'>Скрыть оси</span>
            <input type='checkbox' checked={isHideAxis} onChange={toggleIsHideAxis} />
          </div>
          <div className='menu__item column'>
            <span className='menu__title'>Точки подключения<br></br>[x y z] мм</span>
            <div className='column dots'>
              {dots.map((x, index) => (
                <div key={x.tag + x.x + x.y} className='column'>
                  <div className='row row__space'>
                    <span className='tag'>{x.tag}</span>
                    <span onClick={removeDot(index)} className='menu__close'>x</span>
                  </div>
                  <Coordinates x={x.x} y={x.y} z={x.z} />
                </div>
              ))}
            </div>
          </div>
        </div>
        <div className='menu__links'>
          {isClearable && <div className='menu__item menu__clear'>
            <span className='menu__title' onClick={clear}>Очистить</span>
          </div>}
          <label className='menu__item menu__load'>
            <span className='menu__title'>Загрузить</span>
            <input onChange={onUpload} type='file' />
          </label>
          <div className='menu__item menu__calc'>
            <span className='menu__title' onClick={calculate}>Расчет</span>
          </div>
        </div>
      </div>
      {isLoad && <Loader />}
    </>
  );
}

function mapTag(tag: Tag) {
  if (tag === Tag.RISER) return 0;
  if (tag === Tag.TOILET) return 1;
  if (tag === Tag.SINK) return 2;
  if (tag === Tag.BATH) return 3;
  if (tag === Tag.SHOWER) return 4;
}

function parseNodes(nodes: NodeDto[], origin: Dot2d, min: Dot2d): Node[] {
  const offset = { x: origin.x - min.x, y: origin.y - min.y };

  return nodes.map(x => {
    const isNoWhere = +x.next.x === 0 && +x.next.y === 0 && +x.next.z === 0;

    return {
      id: x.id,
      linewidth: +x.diameter,
      from: { x: +x.point.x - offset.x, y: +x.point.y - offset.y, z: +x.point.z },
      ...(!isNoWhere && { to: { ...x.next, x: +x.next.x - offset.x, y: +x.next.y - offset.y } })
    }
  });
}