out highp float fragmentdepth;
 
void main() {
    fragmentdepth = gl_FragCoord.z;
}