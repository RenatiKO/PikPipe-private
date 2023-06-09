import { DxfViewer, DxfViewerLoadParams, DxfViewerOptions, LayerInfo } from 'dxf-viewer'
import { useCallback, useContext, useEffect, useMemo, useRef, useState } from "react";
import * as THREE from 'three'
//@ts-ignore
import { CSS2DRenderer } from 'three/addons/renderers/CSS2DRenderer.js';
import { v4 } from 'uuid';
import { AppContext } from "../../context";
import { Dot, Dot2d, Node, IDxfViewer, NodeFitting, Tag } from '../../types';
import { getDot, getLabels, getLine } from '../../utils';
import { DotModal } from '../DotModal';
import { errors as errorsText } from './errors';
import { ErrorModal } from '../ErrorModal';
import { Fittings } from '../Fittings';
import { Menu } from "../Menu";
import font from './Roboto-LightItalic.ttf';
import './Viewer.css'

const dotColor = 0x00ff00;
const lineColor = 0x005eff;
const menuWidth = 280;

export const Viewer = () => {  
  const panelRef = useRef<HTMLDivElement>(null);
  const drawingRef = useRef<HTMLDivElement>(null);

  // offset
  const min = useRef<Dot2d | null>(null);
  const max = useRef<Dot2d | null>(null);

  const camera = useRef<THREE.Camera | null>(null);
  const renderer = useRef<THREE.WebGLRenderer | null>(null);
  const scene = useRef<THREE.Scene | null>(null);
  const layers = useRef<LayerInfo[] | null>(null);

  const [origin, setOrigin] = useState<Dot2d | null>(null);
  const [canvas, setCanvas] = useState<HTMLCanvasElement | null>(null);

  const sizes = useRef<THREE.Object3D<THREE.Event>[] | null>(null)

  const { file } = useContext(AppContext);
  const id = useMemo(() => v4(), [file]);

  const reset = () => {
    setDots([]);
    setIsHideSizes(false);
    setIsHideAxis(false);

    newCalc();

    setNodes([]);
    setFittings([]);
  }

  const clear = () => {
    newCalc();
    setNodes([]);
    setFittings([]);
  }

  const newCalc = () => {
    if (labelRenderer.current && renderer.current && scene.current && camera.current) {
      document.body.removeChild( labelRenderer.current.domElement );
      labelRenderer.current = null;

      renderer.current?.render(scene.current, camera.current);        
    }

    setIsFittingsActive(false);
  }

  // load scene
  useEffect(() => {
    reset();

    const load = async () => {
      if (file && drawingRef.current) {
        
        //инициализируем канвас
        const dxfViewer = new DxfViewer(drawingRef.current, { 
          autoResize: true,
          clearAlpha: 0,
          antialias: false,
        } as DxfViewerOptions) as IDxfViewer;

        // загружаем файл
        await dxfViewer.Load({
          url: URL.createObjectURL(file),
          fonts: [font]
        } as DxfViewerLoadParams);

        layers.current = Array.from(dxfViewer.GetLayers());

        dxfViewer.SetSize( window.innerWidth - menuWidth, window.innerHeight );

        //устанавливаем рефы
        scene.current = dxfViewer.GetScene();
        camera.current = dxfViewer.GetCamera();
        renderer.current = dxfViewer.GetRenderer();
        setCanvas(dxfViewer.GetCanvas());

        sizes.current = scene.current.children.slice(0,4);
        
        const origin = dxfViewer.GetOrigin()

        const _min = { x: dxfViewer.bounds.minX-origin.x, y: dxfViewer.bounds.minY-origin.y };
        min.current = _min;
        const _max = { x: dxfViewer.bounds.maxX-origin.x, y: dxfViewer.bounds.maxY-origin.y };
        max.current = _max;

        setOrigin({ x: origin.x + _min.x, y: origin.y + _min.y })

        console.log('РАЗМЕР ЧЕРТЕЖА: ', _max.x - _min.x, 'x', _max.y - _min.y);

        const axesHelper = new THREE.AxesHelper( 4000 );
        axesHelper.position.x = _min.x;
        axesHelper.position.y = _min.y;
        scene.current.add( axesHelper );

        renderer.current?.render(scene.current, camera.current);        
      }
    }

    load();
  }, [file]);

  const [dots, setDots] = useState<Dot[]>([]);
  const [isErrorModalOpen, setIsErrorModalOpen] = useState<boolean>(false);
  const openErrorModal = () => setIsErrorModalOpen(true);
  const errors = (() => {
    if (layers.current) {
      const myWall = layers.current.find(x => x.name === 'MY_WALL') ? null : errorsText.myWall;
      const dotsLength = dots.length >= 2 ? null : errorsText.dotsLength;
      const riser = dots.find(x => x.tag === Tag.RISER) ? null : errorsText.riser;

      return [myWall, dotsLength, riser].filter(Boolean);
    }

    return []
  })()

  // sizes
  const [isHideSizes, setIsHideSizes] = useState<boolean>(true);
  const toggleIsHideSizes = (value: boolean) => setIsHideSizes(value); 
  useEffect(() => {
    if (scene.current && sizes.current && renderer.current && camera.current) {
      for(let y of sizes.current) {
        if (isHideSizes) {
          scene.current.remove(y)
        } else {
          scene.current.add(y)
        }
      }

      renderer.current.render(scene.current, camera.current); 
    }
  }, [isHideSizes]);

  // axis
  const [isHideAxis, setIsHideAxis] = useState<boolean>(false);
  const toggleIsHideAxis = (value: boolean) => setIsHideAxis(value);
  useEffect(() => {
    if (scene.current && sizes.current && renderer.current && camera.current && min.current) {
      if (isHideAxis) {
        const axis = scene.current.children.find(x => x.type === 'AxesHelper');
        if (!axis) return;

        scene.current.remove(axis);
      } else {
        const axesHelper = new THREE.AxesHelper( 4000 );
        axesHelper.position.x = min.current.x;
        axesHelper.position.y = min.current.y;
        scene.current.add( axesHelper );
      }

      renderer.current.render(scene.current, camera.current); 
    }
  }, [isHideAxis]);

  // grids
  const [grid, setGrid] = useState<[number, number]>([2,10])
  useEffect(() => {
    if (panelRef.current) {
      panelRef.current.style.setProperty('--grid-small-size', `${grid[0] * 10}px`);
      panelRef.current.style.setProperty('--grid-big-size', `${grid[1] * 10}px`);
    }
  }, [grid])

  const [nodes, setNodes] = useState<Node[]>([]);
  const [fittings, setFittings] = useState<NodeFitting[]>([]);

  const labelRenderer = useRef<CSS2DRenderer | null>(null);

  // remove dots
  useEffect(() => {
    if (renderer.current && scene.current && camera.current) {
      for (let child of scene.current.children) {
        if (child.type === "Dots") {
          scene.current.remove(child);
        }
      }

      renderer.current.render(scene.current, camera.current); 
    }
  }, [dots]);
  // draw dots
  useEffect(() => {
    if (renderer.current && scene.current && camera.current) {
      // hack for hide/remove dots [to do: normal remove]
      for (let child of scene.current.children) {
        if (child.name === "Dots") {
          child.position.z = 1;
        }
      }

      renderer.current.render(scene.current, camera.current); 

      dots.forEach(x => {
        //@ts-ignore
        const dot = getDot({ x: x.x + min.current.x, y: x.y + min.current.y, size: 10, color: dotColor });
        dot.name = 'Dots';
        scene.current?.add( dot );
      })

      renderer.current.render(scene.current, camera.current); 
    }

  }, [dots])

  // remove nodes
  useEffect(() => {
    if (renderer.current && scene.current && camera.current) {
      for (let child of scene.current.children) {
        if (child.type === "Nodes") {
          scene.current.remove(child);
        }
      }

      renderer.current.render(scene.current, camera.current); 
    }
  }, [nodes]);
  // draw nodes
  useEffect(() => {
    if (renderer.current && scene.current && camera.current) {
      // hack for hide/remove dots [to do: normal remove]
      for (let child of scene.current.children) {
        if (child.name === "Nodes") {
          child.position.z = 1;
        }
      }

      const labels = getLabels(nodes, dotColor);
      labels.forEach(label => {
        label.name = "Nodes";
        scene.current?.add( label );
      });

      nodes.forEach((node, index) => {
        const dot = getDot({ x: node.from.x, y: node.from.y, size: 10, color: dotColor });
        dot.name = 'Nodes';
        scene.current?.add( dot );

        if (index === nodes.length - 1) {
          if (node.to) {
            const dot = getDot({ x: node.to.x, y: node.to.y, size: 10, color: dotColor });
            dot.name = 'Nodes';
            scene.current?.add( dot );
          }
        }

        if (node.to === undefined) return;
        const line = getLine({ from: node.from, to: node.to, linewidth: mapLineWidth(node.linewidth), color: lineColor});
        line.name = 'Nodes';
        scene.current?.add( line );
      })
  
      if (nodes.length) {
        labelRenderer.current = new CSS2DRenderer();
        labelRenderer.current.setSize( window.innerWidth - menuWidth, window.innerHeight );
        labelRenderer.current.domElement.style.position = 'absolute';
        labelRenderer.current.domElement.style.pointerEvents = 'none';
        labelRenderer.current.domElement.style.top = '0px';
        labelRenderer.current.domElement.style.left = `${menuWidth}px`;
        document.body.appendChild( labelRenderer.current.domElement );
  
        labelRenderer.current.render( scene.current, camera.current );
      }

      renderer.current.render(scene.current, camera.current); 
    }
  }, [nodes])

  // modal
  const [isViewChanged, setIsViewChanged] = useState<boolean>(false);
  const [isOpenModal, setIsOpenModal] = useState<boolean>(false);
  const [metaDot, setMetaDot] = useState<Dot2d | undefined>(undefined)
  const [clientDot, setClientDot] = useState<Dot2d | undefined>(undefined);

  // events
  const openModal = useCallback((e: CustomEvent) => {
    if (min.current) {
      if (isViewChanged) {
        setIsViewChanged(false);
        return
      }
  
      const position = e.detail.position;
  
      setClientDot({ x: e.detail.domEvent.clientX, y: e.detail.domEvent.clientY });
      setMetaDot({ x: Math.round(position.x - min.current.x), y: Math.round(position.y - min.current.y) });
      setIsOpenModal(true);
    }
  }, [isViewChanged])
  const onViewChanged = useCallback(() => {
    setIsViewChanged(true);

    if (labelRenderer.current) {
      labelRenderer.current.render( scene.current, camera.current );
    }
  }, [])
  useEffect(() => {
    if (canvas) {
      //@ts-ignore
      canvas.addEventListener("__dxf_pointerup", openModal);
      canvas.addEventListener('__dxf_viewChanged', onViewChanged);

      return () => {
        if (canvas) {
          //@ts-ignore
          canvas.removeEventListener("__dxf_pointerup", openModal);
          canvas.removeEventListener('__dxf_viewChanged', onViewChanged);
        }
      }
    }
  }, [file, openModal, onViewChanged, canvas]);

  // dots handlers
  const addDot = (dot: Dot) => setDots(prev => [dot, ...prev]);
  const removeDot = (index: number) => () => setDots(prev => prev.filter((_x, i) => i !== index));

  const [isFittingsActive, setIsFittingsActive] = useState<boolean>(false);

  const isClearable = !!nodes.length;
  
  return (
    <>
      <div className="viewer">
        <Menu 
          origin={origin}
          min={min.current}
          grid={grid} 
          onChangeGrid={setGrid}
          dots={dots}
          removeDot={removeDot}
          isHideSizes={isHideSizes}
          toggleIsHideSizes={toggleIsHideSizes}
          isHideAxis={isHideAxis}
          toggleIsHideAxis={toggleIsHideAxis}
          onChangeNodes={setNodes}
          onChangeFittings={setFittings}
          onChangeFittingsActive={setIsFittingsActive}
          clearLabels={newCalc}
          isClearable={isClearable}
          clear={clear}
          errors={errors as string[]}
          openErrorModal={openErrorModal}
        />
        <div ref={panelRef} className="viewer__content">
          <div key={id} ref={drawingRef} className="drawing" />
        </div>
        <Fittings
          min={min.current}
          active={isFittingsActive}
          onChangeFittingsActive={setIsFittingsActive}
          fittings={fittings}
        />
      </div>
      {isOpenModal && (
        <DotModal
          setIsOpen={setIsOpenModal}
          x={metaDot?.x as number}
          y={metaDot?.y as number}
          addDot={addDot}
          clientDot={clientDot}
        />
      )}
      {isErrorModalOpen && (
        <ErrorModal 
          setIsOpen={setIsErrorModalOpen}
          errors={errors as string[]}
        />
      )}
    </>
  );
}

function mapLineWidth(linewidth: number) {
  if (linewidth === 110) return 11;
  return 4;
}