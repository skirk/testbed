#version 430

in vec2 pos;
in vec2 uv;

out vec2 TexCoord;
void main() {

	TexCoord = uv;
	gl_Position = vec4(pos, 0.0, 1.0);

}
