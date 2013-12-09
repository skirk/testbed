#version 430

out vec4 col;
in vec4 ocl;


void main() {

	col = ocl;//vec4(ocl, 1.0); // vec4(col.x + 0.5, col.yz, 1.0);
}
	
