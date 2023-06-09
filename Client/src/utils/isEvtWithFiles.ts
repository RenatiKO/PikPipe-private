const isChangeEvent = (
  event: React.DragEvent<HTMLElement> | React.ChangeEvent<HTMLElement>
): event is React.ChangeEvent<HTMLInputElement> => {
  return !(event as React.DragEvent<HTMLElement>).dataTransfer
}

export function isEvtWithFiles(
  event: React.DragEvent<HTMLElement> | React.ChangeEvent<HTMLInputElement>
) {
  if (isChangeEvent(event)) {
    return !!event.target && !!event.target.files
  }

  // https://developer.mozilla.org/en-US/docs/Web/API/DataTransfer/types
  // https://developer.mozilla.org/en-US/docs/Web/API/HTML_Drag_and_Drop_API/Recommended_drag_types#file
  return Array.prototype.some.call(
    event.dataTransfer.types,
    (type) => type === 'Files' || type === 'application/x-moz-file'
  )
}