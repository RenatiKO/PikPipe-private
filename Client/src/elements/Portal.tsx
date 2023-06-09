import { PropsWithChildren } from "react";
import { createPortal } from "react-dom"

export const Portal = ({ children }: PropsWithChildren) => {
  return createPortal(children, document.getElementById('portal') as HTMLElement);
}