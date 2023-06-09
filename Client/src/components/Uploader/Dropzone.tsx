import { ChangeEvent, ReactNode, useCallback, DragEvent, useRef } from 'react';
import { UploaderFile } from '../../types';
import { isEvtWithFiles } from '../../utils';
import './Dropzone.css';

interface DropzoneProps {
  accept?: string;
  disabled?: boolean;
  children?: ReactNode;
  onUpload: (file: UploaderFile, e: ChangeEvent<HTMLInputElement> | DragEvent<HTMLElement>) => void
}

export const Dropzone = ({ 
  accept, 
  disabled, 
  children, 
  onUpload
}: DropzoneProps) => {

  const inputRef = useRef<HTMLInputElement>(null)

  const handleClick = useCallback(() => inputRef.current?.click(), [])

  const handleInputChange: React.ChangeEventHandler<HTMLInputElement> =useCallback((event) => {
      event.preventDefault()
      if (!isEvtWithFiles(event)) {
        return
      }

      const newFiles: UploaderFile[] = Array.from(event.target.files!).map((f) => ({ file: f }))
      const file = newFiles[0];

      onUpload(file, event)

      event.target.value = ''
    },
    [onUpload]
  )

  return (
    <div 
      className="dropzone"
      onClick={handleClick}
    >
        <input
          ref={inputRef}
          type="file"
          accept='dxf'
          disabled={disabled}
          onChange={handleInputChange}
        />
        {children}
    </div>
  );
}