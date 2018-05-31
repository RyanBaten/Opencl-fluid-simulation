varying vec4 pos;

const float width = 0.025;
const float scale = 2;

void main() {
  if ((fract(pos[0]*scale)>width) && (fract(pos[2]*scale)>width)) {
    gl_FragColor = vec4(0.42,0.77,0.85,1.0);
  } else {
    gl_FragColor = vec4(0.0,0.0,0.0,0.0);
  }
}
