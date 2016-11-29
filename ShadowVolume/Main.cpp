/*

					tentativa projeto GB
					HUAEHAUEHUAHSUHA

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
					#version 330

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

					#version 330

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

#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

//#include "ogldev_util.h"
#include "ogldev_math_3d.h"

//int do VBO
GLuint VBO;

//int scale - para variavel uniform
//GLuint gScaleLocation;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

static void RenderSceneCB()
{
	glClear(GL_COLOR_BUFFER_BIT);

	//variaveis uniformes 
	//diz-se que a escala e statica
	//static float Scale = 0.0f;
	//Scale += 0.001f;
	//glUniform1 seta para a escala do shader este valor estatico
	//glUniform1f(gScaleLocation, sinf(Scale));
	//variaveis uniformes 

	//posicao dos vertex que carregamos no buffer, tratado na posicao 0 pela pipeline
	glEnableVertexAttribArray(0);

	//novamente o bind do buffer para preparar para chamadas de desenhar
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//dizemos como interpretar os dados dentro do buffer : index do atributo, numero de componentes (X,Y,Z) = 3, tipo de dado de cada componente
	//'stride': numero de bytes entre instancias do atributo no buffer : quando tem apenas 1 atributo, passa-se 0, se nao passa-se o tamanho dos dados
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	///#desenhar ponto
	//desenhar a geometria: 
	//glDrawArrays(GL_POINTS, 0, 1);

	///#desenhar triangulo
	//em vez de passar 1, passa-se 3, extendemos o array para conter 3 vertices	
	glDrawArrays(GL_TRIANGLES, 0, 3);


	//é bom desabilitar os atributos quando nao usados diretamente, se nao esta usando shader pode dar problemas
	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}


static void InitializeGlutCallbacks()
{
	//basico para o callback de display
	glutDisplayFunc(RenderSceneCB);

	//parte de variaveis uniformes
	//matenos em idle o mundo
	//glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
	///#desenhar ponto
	//vertice e posicao do ponto na tela
	//Vector3f Vertices[1];
	//ponto no meio da tela
	//Vertices[0] = Vector3f(0.0f, 0.0f, 0.0f);

	///#desenhar triangulo
	//desenhar um triangulo
	Vector3f Vertices[3];
	Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
	Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
	Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);


	//alocamos um int globalmente para manusear o VBO
	glGenBuffers(1, &VBO);

	//ligamos ao buffer o VBO e dizemos que este buffer conterá um array de vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//depois de ligar, preenchemos com dados: array de vertices, tamanho dos dados em bytes, endereco do array, - GL_STATIC_DRAW < para que nao tenha troca de conteudos
	//GL_DYNAMIC_DRAW < para que tenha
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//criacao do shader: GL_VERTEX_SHADER  ou GL_FRAGMENT_SHADER
	//o processo de compilacao e o mesmo para os 2
	GLuint ShaderObj = glCreateShader(ShaderType);


	//processo de erro
	if (ShaderObj == 0) {
		fprintf(stderr, "Erro ao criar o ShaderType %d\n", ShaderType);
		exit(0);
	}


	//antes de copilar o shader, especificamos sua fonte
	//shaderText > vem o VS ou FS
	const GLchar* p[1];
	//passamos para um char os arquivos glsl
	p[0] = pShaderText;

	//tamanho dos textos do vs/fs
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);

	//pega o shader como parametro
	glShaderSource(ShaderObj, 1, p, Lengths);

	//compila o shader
	glCompileShader(ShaderObj);

	//verifica se deu tudo ok na compilacao
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	//tratamento de erro
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Erro em compilar o ShaderType %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	//anexamos o objeto do shader compilado no programa
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{

	//inicio do processo de configuracao dos shaders criando um program object. Ligaremos todos os shaders juntos neste objeto
	GLuint ShaderProgram = glCreateProgram();


	//processo de erro
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	//configuracao dos vs/fs
	string vs, fs;

	//funcao para ler os arquivos
	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	};
	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	};

	//funcao para adicionar o shader no programa 
	//passamos o program object atual, o arquivo fs/vs, e seu tipo de shader
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	//depois de anexar o shader no program, o link ira liga-los
	glLinkProgram(ShaderProgram);

	//processos de erro
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//verifica se deu tudo ok no link
	//possiveis erros baseados na combinacao dos shaders
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Erro ao linkar o ShaderProgram : '%s'\n", ErrorLog);
		exit(1);
	}

	//valida os shaders - verifica se o atual estado dos shaders esta ok
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalidade ShaderProgram: '%s'\n", ErrorLog);
		exit(1);
	}

	//setamos que este programa ficara em toda chamada de desenho ate substituir
	glUseProgram(ShaderProgram);


	//pega do glsl a variavel "gScale" uniform para este shader
	//essa variavel esta iniciada no glsl
	//gScaleLocation = glGetUniformLocation(ShaderProgram, "gScale");
	//0xFFFFFFFF - se o shader conter uma posicao no buffer, seja 0,1,2, etc... caso algum erro ocorra, ele tera a posicao -1, que é equivalente á 0xFFFFFFFF
	//verifica se nao tenham erros sobre este shader no buffer
	//assert(gScaleLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	//Double buffer - background e front / Color buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Projeto Shadow Volume");

	//inicia os callbacks
	InitializeGlutCallbacks();

	// verificar algum erro , pega ENUM direto da Glew
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Erro: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	//callback de rendering dos frames
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//funcao para criacao do VBO
	CreateVertexBuffer();

	//compilar os shaders
	CompileShaders();

	//main loop da opengl
	glutMainLoop();

	return 0;
}