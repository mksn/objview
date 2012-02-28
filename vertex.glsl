#version 120
uniform mat4 BoneMatrix[80];
attribute vec4 in_BlendIndex;
attribute vec4 in_BlendWeight;

void main() {
	int index = int(in_BlendIndex.x);
	vec4 vertex = BoneMatrix[index] * gl_Vertex;
//vertex = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
}
