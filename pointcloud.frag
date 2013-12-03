#version 430

in vec3 col;
out vec4 color;


void main() {

	color = vec4(col.x + 0.5, col.yz, 1.0);
}
	
