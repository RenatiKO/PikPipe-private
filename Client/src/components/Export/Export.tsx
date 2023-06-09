import { DataSheet, NodeFitting } from "../../types";
import * as XLSX from 'xlsx'
import { useContext, useMemo } from "react";
import { AppContext } from "../../context";
import './Export.css';

interface Props {
  data: NodeFitting[];
}

export const Export = ({ data }: Props) => {
  const { file } = useContext(AppContext);

  const sheetData = useMemo(() => getSheetData(data), [data]);

  const onExport = () => {
    const wb = XLSX.utils.book_new();
    const ws = XLSX.utils.json_to_sheet(sheetData);

    XLSX.utils.book_append_sheet(wb, ws, "Фиттинги");
    XLSX.writeFile(wb, `${file?.name.split('.')[0]}.xlsx`)
  }

  return (
    <span onClick={onExport} className='export'>Экспорт</span>
  );
}

function getSheetData(nodes: NodeFitting[]): DataSheet[] {
  const sheet: DataSheet[] = [];

  nodes.forEach(node => {
    node.fittings.forEach(fitting => {
      const row = {
        id: node.Node, x: Math.round(node.x), y: Math.round(node.y), z: Math.round(node.z), fitting: `${fitting.name} - ${fitting.type}`
      }

      sheet.push(row);
    })
  })

  return sheet;
}