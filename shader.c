#include <mksn-gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *load_source(char *filename)
{
	char *data;
	int len;
	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "error: cannot open '%s'\n", filename);
		return NULL;
	}
	fseek(file, 0, 2);
	len = ftell(file);
	fseek(file, 0, 0);
	data = malloc(len+1);
	fread(data, 1, len, file);
	data[len] = 0;
	fclose(file);
	return data;
}

int compile_shader(char *vertfile, char *fragfile)
{
	char log[800];
	char *vsrc, *fsrc;
	int prog, vs, fs, len;

	vsrc = load_source(vertfile);
	fsrc = load_source(fragfile);

	if (!vsrc || !fsrc)
		return 0;

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const char **)&vsrc, 0);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &len);
	if (len > 1) {
		glGetShaderInfoLog(vs, sizeof log, &len, log);
		fprintf(stderr, "%s:\n%s", vertfile, log);
	}

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const char **)&fsrc, 0);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &len);
	if (len > 1) {
		glGetShaderInfoLog(fs, sizeof log, &len, log);
		fprintf(stderr, "%s:\n%s", fragfile, log);
	}

	free(vsrc);
	free(fsrc);

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 1) {
		glGetProgramInfoLog(prog, sizeof log, &len, log);
		fprintf(stderr, "%s/%s:\n%s", vertfile, fragfile, log);
	}

	
	return prog;
}

#if 0

static void print_shader_log(char *kind, int shader)
{
	int len;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	char *log = malloc(len + 1);
	glGetShaderInfoLog(shader, len, NULL, log);
	fprintf(stderr, "--- glsl %s shader compile results ---\n%s\n", kind, log);
	free(log);
}

static void print_program_log(int program)
{
	int len;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
	char *log = malloc(len + 1);
	glGetProgramInfoLog(program, len, NULL, log);
	fprintf(stderr, "--- glsl program link results ---\n%s\n", log);
	free(log);
}

int compile_shader(const char *vert_src, const char *frag_src)
{
	int status;

	int vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vert_src, NULL);
	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
	if (!status)
		print_shader_log("vertex", vert);

	int frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &frag_src, NULL);
	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
	if (!status)
		print_shader_log("fragment", frag);

	int prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (!status)
		print_program_log(prog);

	glDeleteShader(vert);
	glDeleteShader(frag);

	return prog;
}
#endif
