
//essencial
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>

#include <stdio.h>
#include <stdarg.h>
using namespace std;

//meshs
#include <assimp/cimport.h>
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <assert.h>

//dfine do mesh
#define MESH_FILE "monkey2.obj"

/* copia o shader para um plano de arquivo de texto pra array de char */
bool AbrirArquivo(const char* file_name, char* shader_str, int max_len) {
	FILE* file = fopen(file_name, "r");
	if (!file) {
		//gl_log_err("ERROR: opening file for reading: %s\n", file_name);
		return false;
	}
	size_t cnt = fread(shader_str, 1, max_len - 1, file);
	if ((int)cnt >= max_len - 1) {
		//gl_log_err("WARNING: file %s too big - truncated.\n", file_name);
	}
	if (ferror(file)) {
		//gl_log_err("ERROR: reading shader file %s\n", file_name);
		fclose(file);
		return false;
	}
	// append \0 to end of file string
	shader_str[cnt] = 0;
	fclose(file);
	return true;
}

/* load a mesh using the assimp library */
bool load_mesh(const char* file_name, GLuint* vao, int* point_count) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate);
	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	printf("  %i animations\n", scene->mNumAnimations);
	printf("  %i cameras\n", scene->mNumCameras);
	printf("  %i lights\n", scene->mNumLights);
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	/* get first mesh in file only */
	const aiMesh* mesh = scene->mMeshes[0];
	printf("    %i vertices in mesh[0]\n", mesh->mNumVertices);

	/* pass back number of vertex points in mesh */
	*point_count = mesh->mNumVertices;

	/* generate a VAO, using the pass-by-reference parameter that we give to the
	function */
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	/* we really need to copy out all the data from AssImp's funny little data
	structures into pure contiguous arrays before we copy it into data buffers
	because assimp's texture coordinates are not really contiguous in memory.
	i allocate some dynamic memory to do this. */
	GLfloat* points = NULL; // array of vertex points
	GLfloat* normals = NULL; // array of vertex normals
	GLfloat* texcoords = NULL; // array of texture coordinates
	if (mesh->HasPositions()) {
		points = (GLfloat*)malloc(*point_count * 3 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vp = &(mesh->mVertices[i]);
			points[i * 3] = (GLfloat)vp->x;
			points[i * 3 + 1] = (GLfloat)vp->y;
			points[i * 3 + 2] = (GLfloat)vp->z;
		}
	}
	if (mesh->HasNormals()) {
		normals = (GLfloat*)malloc(*point_count * 3 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vn = &(mesh->mNormals[i]);
			normals[i * 3] = (GLfloat)vn->x;
			normals[i * 3 + 1] = (GLfloat)vn->y;
			normals[i * 3 + 2] = (GLfloat)vn->z;
		}
	}
	if (mesh->HasTextureCoords(0)) {
		texcoords = (GLfloat*)malloc(*point_count * 2 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
			texcoords[i * 2] = (GLfloat)vt->x;
			texcoords[i * 2 + 1] = (GLfloat)vt->y;
		}
	}

	/* copy mesh data into VBOs */
	if (mesh->HasPositions()) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			3 * *point_count * sizeof(GLfloat),
			points,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		free(points);
	}
	if (mesh->HasNormals()) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			3 * *point_count * sizeof(GLfloat),
			normals,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);
		free(normals);
	}
	if (mesh->HasTextureCoords(0)) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			2 * *point_count * sizeof(GLfloat),
			texcoords,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(2);
		free(texcoords);
	}
	if (mesh->HasTangentsAndBitangents()) {
		// NB: could store/print tangents here
	}

	aiReleaseImport(scene);
	printf("mesh loaded\n");

	return true;
}

int main()

{
	//comeco

	//se nao tem um start
	if (!glfwInit())
	{
		fprintf(stderr, "Problemas no GLFW\n");
		return 1;
	}

	//inicializar a janela
	GLFWwindow* janela = glfwCreateWindow(800, 600, "Shadow Volume", NULL, NULL);

	//se nao consguiu criar janela
	if (!janela)
	{
		fprintf(stderr, "Problemas ao abrir a janela\n");
		//destroi tudo sobre a janela
		glfwTerminate();
		return 1;
	}

	//especifica em qual janela o contexto da opengl 
	glfwMakeContextCurrent(janela);

	//start na glew
	glewExperimental = GL_TRUE;
	glewInit();



	// habilita depth-testing
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	//interpreta valores menores como perto da tela
	glDepthFunc(GL_LESS);




	/* load the mesh using assimp */
	GLuint monkey_vao;
	int monkey_point_count = 0;
	assert(load_mesh(MESH_FILE, &monkey_vao, &monkey_point_count));







	//pontos para desenhar
	GLfloat points[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f
	};

	//BUFFER DOS VERTICES

	//buffer vazio para os vertices
	//vertex buffer obj
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	//bind no buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//passa o tamanho do buffer - tamanho dos pontos
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//BUFFER DE ATRIBUTOS, REFERENTE AO BUFFER DOS VERTICES

	//atributos dos vertices, dizer os vertices que quer usar
	//vertex atribute obj
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	//dizer que vai usar o buffer vbo 
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//define o layout do buffer de vertices - a partir da coord 0, desenhar 3 pontos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


	//AQUI ENTRAR OS SHADERS
	//vertex_shader e fragment_shader.glsl - arquivos glsl sao textos interpretados pela opengl

	//pode criar diretamente os glsl aqui e usar

	//arquivos dentro da pasta shaders NAO estao sendo usados!
	//pode criar como esta a baixo para usar

	//vertex_shader.glsl < mesma coisa
	/*const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"void main () {"
		" gl_Position = vec4 (vp, 1.0);"
		"}";*/

	//fragment_shader.glsl < mesma coisa
	/*const char* fragment_shader =
		"#version 400\n"
		"out vec4 frag_colour;"
		"void main () {"
		" frag_colour = vec4 (0.5, 0.8, 0.5, 1.0);"
		"}";*/

	//USAR COM ARQUIVOS
	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	const GLchar* ponteiro;

	//passa o arquivo para as variaveis
	AbrirArquivo("vertex_shader.glsl", vertex_shader, 1024 * 256);
	AbrirArquivo("fragment_shader.glsl", fragment_shader, 1024 * 256);


	//JUNTA OS ARQUIVOS COM OS BUFFERS
	//vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	//obrigatoriamente precisa pegar o ponteiro das variaveis
	ponteiro = (const GLchar*)vertex_shader;

	glShaderSource(vs, 1, &ponteiro, NULL);
	glCompileShader(vs);

	//fragment shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	//obrigatoriamente precisa pegar o ponteiro das variaveis
	ponteiro = (const GLchar*)fragment_shader;

	glShaderSource(fs, 1, &ponteiro, NULL);
	glCompileShader(fs);


	//EXECUTAR OS SHADERS NA GPU
	GLuint shader_executar = glCreateProgram();
	glAttachShader(shader_executar, fs);
	glAttachShader(shader_executar, vs);
	glLinkProgram(shader_executar);



	//DESENHO
	//enquanto a janela estiver ativa
	while (!glfwWindowShouldClose(janela))
	{
		//limpa os buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//programa sendo executado
		glUseProgram(shader_executar);

		//os vertex
		glBindVertexArray(vao);

		glBindVertexArray(monkey_vao);
		glDrawArrays(GL_TRIANGLES, 0, monkey_point_count);

		//desenhar modo triangulo, a partir do ponto 0 
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//atualizar eventos que nao sao desenhos, tipo teclado, etc
		glfwPollEvents();

		//pega dos buffer o que desenhamos e joga no display
		glfwSwapBuffers(janela);

	}





	glfwTerminate();
	return 0;
}

