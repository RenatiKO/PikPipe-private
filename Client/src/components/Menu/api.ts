export const api = {
  calc: (request: FormData) => {
    return fetch('http://45.147.179.102:8888/calc', {
//	return fetch('http://127.0.0.1:8888/calc', {
      method: 'POST',
      body: request
    }).then(res => res.json());
  }
}
