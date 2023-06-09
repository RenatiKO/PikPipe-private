import { DxfViewer } from "dxf-viewer"

//расширяемый DxfViewer
export interface IDxfViewer extends DxfViewer {
  bounds: {
    minX: number;
    minY: number;
    maxX: number;
    maxY: number;
  }
}

export type UploaderError = 'FILE_INVALID_TYPE'

export type UploaderFile = {
  file: File
  error?: UploaderError
}

export enum Tag {
  RISER = 'Стояк',
  TOILET = 'Туалет',
  SINK = 'Раковина',
  BATH = 'Ванна',
  SHOWER = 'Душ'
}

export type Dot = Dot2d & {
  z: number;
  tag: Tag;
}

export type Dot2d = {
  x: number;
  y: number;
}

export type Dot3d = {
  x: number;
  y: number;
  z: number;
}

export interface NodeDto {
  point: Dot3d;
  next: Dot3d;
  diameter: string;
  id: string;
}

export interface Node {
  from: Dot3d;
  to?: Dot3d;
  linewidth: number;
  id: string;
}

export type NodeFitting = Dot3d & {
  Node: number;
  fittings: Fitting[];
}

export interface Fitting {
  name: string;
  type: string;
}

export type DataSheet = Dot3d & {
  id: number;
  fitting: string;
}
