import { Button } from '../Button';
import { Dropzone } from './Dropzone';
import { useContext } from 'react';
import { AppContext } from '../../context';
import { UploaderFile } from '../../types';
import './Uploader.css';

export const Uploader = () => {
  const { setFile } = useContext(AppContext);

  const onUpload = (file: UploaderFile) => {
    setFile?.(file.file);
  }

  return (
    <div className='uploader'>
      <Dropzone 
        onUpload={onUpload}
      >
        <Button label='Загрузить DXF' />
      </Dropzone>
    </div>
  );
}