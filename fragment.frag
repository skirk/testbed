#version 430

in vec2 TexCoord;
out vec4 color;

layout(binding=0) uniform sampler2D diffuseTex;

void main() {

	vec4 texcolor = texture(diffuseTex, TexCoord);
	color = texcolor;
}
