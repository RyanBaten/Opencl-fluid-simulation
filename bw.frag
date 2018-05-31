varying vec4 pos;

const float y_scale = 8;
const float color_min = 0.1;

void main() {
  vec4 color = normalize(pos);
  gl_FragColor = vec4(color.y*y_scale + color_min, color.y*y_scale + color_min,color.y*y_scale + color_min,1);
}
