import { useMemo } from 'react';
import { Dot2d, NodeFitting } from '../../types';
import { Coordinates } from '../Coordinates';
import { Export } from '../Export';
import './Fittings.css';

interface Props {
  min: Dot2d | null;
  active: boolean;
  onChangeFittingsActive: (value: boolean) => void;
  fittings: NodeFitting[];
}

export const Fittings = ({
  min,
  active,
  onChangeFittingsActive,
  fittings
}: Props) => {
  
  const toggle = () => onChangeFittingsActive(!active);

  const normalFittings = useMemo(() => {
    if (!min) return fittings;

    return fittings.map(node => ({ ...node, x: node.x - min.x, y: node.y - min.y }));
  }, [fittings, min]);

  const hasFittings = !!normalFittings.length;

  return (
    <div className={`fittings ${active ? 'active': ''}`}>
      <div className='fittings__title'>
        <span className='fittings__toggle' onClick={toggle}>{`${active ? '→': '←'}`}</span>
        <span className='fittings_name'>Фиттинги</span>
      </div>
      <div className='fittings__content'>
        <div className='fittings__table'>
          {normalFittings.map((node,index) => (
            <div key={String(index)} className='fittings__row'>
              <span className='fittings_title'>Точка '{node.Node}':</span>
              <div className='fittings__column'>
                <Coordinates x={node.x} y={node.y} z={node.z} />
                {node.fittings.map((x, index) => (
                  <span key={x.name + x.type + index}><span className='fittings__type'>name: </span>{x.name}<span className='fittings__type'>  type: </span>{x.type}</span>
                ))}
              </div>
            </div>
          ))}
        </div>
        {hasFittings && <Export data={normalFittings} /> }
      </div>
    </div>
  );
}