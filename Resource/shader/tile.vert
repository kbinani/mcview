attribute vec2 textureCoordIn;
attribute vec4 position;
uniform float blocksPerPixel;
uniform float Xr;
uniform float Zr;
uniform float width;
uniform float height;
uniform float Cx;
uniform float Cz;
varying vec2 textureCoordOut;
void main() {
    textureCoordOut = textureCoordIn;

    float Xp = position.x;
    float Yp = position.y;
    float Xm = Xr + Xp * 512.0;
    float Zm = Zr + Yp * 512.0;
    float Xw = (Xm - Cx) / blocksPerPixel + width / 2.0;
    float Yw = (Zm - Cz) / blocksPerPixel + height / 2.0;
    float Xg = 2.0 * Xw / width - 1.0;
    float Yg = 1.0 - 2.0 * Yw / height;

    gl_Position = vec4(Xg, Yg, position.z, position.w);
}
