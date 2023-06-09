export function toHex(d: number) {
  let hex = Number(d).toString(16);
  hex = "000000".slice(0, 6 - hex.length) + hex;
  return hex;
}