/*

		 projeto GB


		 //base
		 1 Processo de vertex : responsavel pelo shader de cada vertex que passa na pipeline >
		 2 Processo de geometria >
		 3 Bounds/Binds >
		 4 Rasterizacao (tarefa de converter uma imagem vetorial em uma imagem raster (pixels ou pontos) para a saída em vídeo) >
		 5 Processo de fragmentos


		 Noções gerais:
		 * usa-se w=1 para points e w=0 para vetores porque pontos podem ser transladados e vetores nao

		 * variaveis uniformes: tipo de variaveis de shader -> existem variaveis atributos que contem os dados de caxa vertex, entao
		 cada vez que chamar a funcao para desenhar ele recarrega os novos valores do vertex buffer para cada invocacao de shader.
		 Enquanto que, o valor uniforma nas variaveis mantem constante em todo o desenho. Provavelmente deixa mais leve para redesenhar algo que nao sera desenhado novamente.
		 Nao temos acesso direto ao conteudo dos shader e as variaveis se mantem inacessiveis.
		 Pelo que entendi tu "trava" o que tu quer que nao vai ser mudado ou para mover algo e nao ocorra erros de mudar algo de posicao enquanto translada.




		 SHADER.VS:

		 //----------------especifica a versao da glsl - se o compilador nao ter o suporte vai dar erro
		 #version 400

		 //----------------declaramos que um vertex especifico que é um Vetor de 3 floats será conhecido como "Position" no shader
		 //----------------vertex especifico: cada invocacao do shader na GPU o valor do novo vertex vindo do buffer sera substituido

		 //----------------layout (location = 0) : cria um binding entre o nome do atributo e o buffer do atributo, caso o vertex contenha varios parametros (posicao, normais, textura, etc)
		 //----------------o compilador saber no buffer onde esta mapeado para declarar o shader especifico
		 layout (location = 0) in vec3 Position;

		 //----------------pode-se criar inumeros shaders, mas apenas 1 funcao principal para executa-los em cada estagio  (VS, GS, FS)
		 void main()
		 {

		 //---------------- gl_Position: uma variavel construida especialmente para conter a posicao do vertex (x,y,z,w). O rasterizador procurará por esta variavel e usara na no espaço da tela
		 gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z, 1.0);

		 }


		 SHADER.FS:

		 #version 400

		 //---------------- usualmente a funcao do fragment shader é determinar a cor do pixel
		 //---------------- ele pode alterar ou descartar o valor em Z, que vai alterar o resultado do Z-Test
		 //---------------- Vec4 = RGBA
		 out vec4 FragColor;

		 void main()
		 {

		 //---------------- FRAMEBUFFER - cor
		 FragColor = vec4(1.0, 0.0, 0.0, 1.0);

		 }





		 SHADER.VS COM VARIAVEIS UNIFORMES:
		 #version 330

		 layout (location = 0) in vec3 Position;

		 //---------------- daqui se tira a viariavel que nao vai mudar a escala do shader
		 uniform float gScale;

		 void main()
		 {

		 gl_Position = vec4(gScale * Position.x, gScale * Position.y, Position.z, 1.0);


		 }





		 */



//essencial
#include <GL/glew.h> 

#include <glm/glm/glm.hpp>

#include <stdio.h>
#include <stdarg.h>
using namespace std;

//meshs
#include <assimp/cimport.h>
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <assert.h>

#include "camera.h"
#include "ogldev_pipeline.h"
#include "ogldev_keys.h"


#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

//dfine do mesh
#define MESH_FILE "monkey2.obj"

//camera
Camera* pGameCamera = NULL;

//projecao
PersProjInfo gPersProjInfo;

//vertex buffer obj
GLuint VBO;
//index buffer obj
GLuint IBO;
//world location
GLuint gWVPLocation;


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



static void CreateVertexBuffer()
{
	Vector3f Vertices[4];
	Vertices[0] = Vector3f(-1.0f, -1.0f, 0.5773f);
	Vertices[1] = Vector3f(0.0f, -1.0f, -1.15475f);
	Vertices[2] = Vector3f(1.0f, -1.0f, 0.5773f);
	Vertices[3] = Vector3f(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	unsigned int Indices[] = { 0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2 };

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}


void KeyBoard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	pGameCamera->OnKeyboard(key);
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
	GLFWwindow* janela = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Shadow Volume", NULL, NULL);

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
	/*	GLuint monkey_vao;
		int monkey_point_count = 0;
		assert(load_mesh(MESH_FILE, &monkey_vao, &monkey_point_count));*/

	//BUFFER DOS VERTICES

	//buffer vazio para os vertices
	//vertex buffer obj
	//GLuint vbo = 0;
	//	glGenBuffers(1, &vbo);
	//bind no buffer
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//passa o tamanho do buffer - tamanho dos pontos
	//glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//BUFFER DE ATRIBUTOS, REFERENTE AO BUFFER DOS VERTICES

	//atributos dos vertices, dizer os vertices que quer usar
	//vertex atribute obj


	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);




	//dizer que vai usar o buffer vbo 
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//define o layout do buffer de vertices - a partir da coord 0, desenhar 3 pontos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


	//AQUI ENTRAR OS SHADERS
	//vertex_shader e fragment_shader.glsl - arquivos glsl sao textos interpretados pela opengl

	//pode criar diretamente os glsl aqui e usar

	//arquivos dentro da pasta shaders NAO estao sendo usados!
	//pode criar como esta a baixo para usar


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


	gWVPLocation = glGetUniformLocation(shader_executar, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);


	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CW); // GL_CCW for counter clock-wise

	//CAMERA



	//pos da camera
	pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

	
	gPersProjInfo.FOV = 60.0f;
	gPersProjInfo.Height = WINDOW_HEIGHT;
	gPersProjInfo.Width = WINDOW_WIDTH;
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;

	//DESENHO
	CreateVertexBuffer();
	CreateIndexBuffer();


	
	//enquanto a janela estiver ativa
	while (!glfwWindowShouldClose(janela))
	{
		//limpa os buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


		//programa sendo executado
		glUseProgram(shader_executar);
		glBindVertexArray(vao);

		static float Scale = 0.0f;

		Scale += 0.1f;

		Pipeline p;
		p.Rotate(0.0f, Scale, 0.0f);
		p.WorldPos(0.0f, 0.0f, 3.0f);
		p.SetCamera(*pGameCamera);
		p.SetPerspectiveProj(gPersProjInfo);

		glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetWVPTrans());

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);

		glfwPollEvents();

		glfwSetKeyCallback(janela, KeyBoard);

		//pega dos buffer o que desenhamos e joga no display
		glfwSwapBuffers(janela);


	}

	glfwTerminate();
	return 0;
}

