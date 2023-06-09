import { useState } from "react";
import { Uploader, Viewer } from "./components";
import { AppContext } from "./context";

export function App() {
  const [file, setFile] = useState<File | undefined>(undefined);

  return (
    <AppContext.Provider value={{ file, setFile }}>
      <div className="container">
        {file ? <Viewer /> : <Uploader />}
      </div>
    </AppContext.Provider>
  );
}
