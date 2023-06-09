import * as THREE from "three";
import { Line2, LineGeometry, LineMaterial } from "three-fatline";
//@ts-ignore
import { CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import { Dot2d, Node } from "../types";
import { toHex } from "./toHex";

interface Dot extends Dot2d {
  size: number;
  color: number;
}

export function getDot({ x, y, size, color }: Dot): THREE.Points<THREE.BufferGeometry, THREE.PointsMaterial> {
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute( 'position', new THREE.Float32BufferAttribute( [x, y, 0], 3 ) );
  const material = new THREE.PointsMaterial( { size, color } );
  const dot = new THREE.Points( geometry, material );
  
  return dot;
}

export interface Label {
  x: number;
  y: number;
  nodes: string[];
}

export function getLabels(nodes: Node[], color: number) {
  const labels: Label[] = [];
  
  nodes.forEach(node => {
    const x = Math.round(node.from.x)
    const y = Math.round(node.from.y)

    const label = labels.find(label => label.x === x && label.y === y);
    if (label) {
      label.nodes.push(node.id);
    } else {
      labels.push({ x, y, nodes: [node.id] });
    }
  });

  const stringColor = toHex(color)

  return labels.map(x => {
    const div = document.createElement( 'div' );
    div.className = 'label';
    div.textContent = x.nodes.join('/');
    div.style.backgroundColor = 'transparent';
    div.style.fontSize = '26px';
    div.style.fontWeight = '700';
    div.style.textShadow = '2px 2px 4px #000000';
    div.style.color = `#${stringColor}`;
    const label = new CSS2DObject( div );
    label.position.set( x.x, x.y, 0 );

    return label;
  });
}

interface Line {
  from: Dot2d;
  to: Dot2d;
  linewidth: number;
  color: number;
}

export function getLine({ from, to, linewidth, color }: Line): Line2 {
  const geometry = new LineGeometry();
  geometry.setPositions([from.x, from.y, 0, to.x, to.y, 0]);
  const material = new LineMaterial( { linewidth: linewidth*0.001, color } );
  const line = new Line2( geometry, material );

  return line;
}
