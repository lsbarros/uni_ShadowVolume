
//essencial
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdarg.h>
using namespace std;

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
	const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"void main () {"
		" gl_Position = vec4 (vp, 1.0);"
		"}";

	//fragment_shader.glsl < mesma coisa
	const char* fragment_shader =
		"#version 400\n"
		"out vec4 frag_colour;"
		"void main () {"
		" frag_colour = vec4 (0.5, 0.8, 0.5, 1.0);"
		"}";


	//JUNTA OS ARQUIVOS COM OS BUFFERS
	//vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	//fragment shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fs, 1, &fragment_shader, NULL);
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