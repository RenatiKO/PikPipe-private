import { createContext } from "react";

export interface IAppContext {
  file?: File;
  setFile?: (file: File | undefined) => void;
}

export const AppContext = createContext<IAppContext>({})