varying vec4 pos;

const float y_scale = 8;

void main() {
  vec4 color = normalize(pos);
  gl_FragColor = vec4(color.x,color.y*y_scale,color.z,1);
}
