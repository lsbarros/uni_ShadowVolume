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

		#pragma region INCLUEDES

		//essencial - INCLUDES
		#include <GL/glew.h> 
		//#include <glm/glm/glm.hpp>

		#include <stdio.h>
		#include <stdarg.h>

		//meshs
		#include <assimp/cimport.h>
		#include <assimp/scene.h> 
		#include <assimp/postprocess.h> 
		#include <assert.h>
		#include "gl_utils.h"
		#include "maths_funcs.h"
		#include "stb_image.h" 

		#pragma endregion 



		#pragma region DEFINES

		#define MESH_FILE "suzanne.obj"
		#define MESH_FILE2 "triangulo.obj"
		#define MESH_FILE3 "Deer.obj"
		#define NMAP_IMG_FILE "brickwork_normal-map.png"

		#define PLAIN_VS "plain.vert"
		#define PLAIN_FS "plain.frag"
		#define VERTEX_SHADER_FILE "test_vs.glsl"
		#define FRAGMENT_SHADER_FILE "test_fs.glsl"

		#define DEBUG_VS "ss_quad.vert"
		#define DEBUG_FS "ss_quad.frag"
		#define DEPTH_VS "depth.vert"
		#define DEPTH_FS "depth.frag"

		#define NUM_OBJS 6

		#pragma endregion


		#pragma region Init Variaveis 


		//padrao da gl_utils
		int g_gl_width = 1024;
		int g_gl_height = 768;
		GLFWwindow* g_window = NULL;



		//RELACIONADOS AO CHAO DO CENARIO
		//program para o ground do cenario
		GLuint g_plain_sp;
		// location model matrix  > para camera
		GLint g_plain_M_loc;
		// location view matrix > para camera
		GLint g_plain_V_loc;
		// location projection matrix > para camera
		GLint g_plain_P_loc;

		//vertex buffer obj para o ground
		GLuint VBO;

		int obj_point_count;
		GLuint objVAO;

		int obj_point_count2;
		GLuint objVAO2;

		int obj_point_count3;
		GLuint objVAO3;

		//cont dos pontos do ground
		int g_Ground_Scene_point_count;
		//VAO do ground
		GLuint g_Ground_Scene_vao;

		//CASTERS PARA A LUZ - ainda nao funcionando / testes
		//shadow caster view matriz
		GLint g_plain_caster_V_loc;
		//shadow caster proj matrix
		GLint g_plain_caster_P_loc;
		// shadow caster cor
		GLint g_plain_colour_loc;
		//shadow caster shaders
		GLint g_plain_shad_resolution_loc;

		int g_shadow_size = 1024;

		// shadow caster view e projecao das matrizes. em pratica, cada light source
		mat4 g_caster_V;
		mat4 g_caster_P;


		//RELACIONADOS AO OBJ DO CENARIO - Monkey
		//location matrix
		GLint matr_mat_location;
		//view matrix
		GLint view_mat_location;
		//projection matrix
		GLint proj_mat_location;


		//RELACIONADOS A CAMERA
		//view e projecao das matrizes da camera
		mat4 g_camera_V;
		mat4 g_camera_P;

		mat4 g_objs_Ms[NUM_OBJS];

		//RELACIONADOS A TEXTURA
		// array of vertex points
		GLfloat* g_vp = NULL;
		// array of vertex normals
		GLfloat* g_vn = NULL;
		// array of texture coordinates
		GLfloat* g_vt = NULL;
		GLfloat* g_vtans = NULL;
		int g_point_count = 0;

		//SOMBRA
		GLuint g_depth_sp;
		GLint g_depth_M_loc;
		GLint g_depth_V_loc;
		GLint g_depth_P_loc;
		//framebuffer
		GLuint g_depth_fb;
		GLuint g_depth_fb_tex;

		#pragma endregion


		//strings
		using namespace std;

		//load mesh - funcao do antons const char* file_name
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

		/* setup framebuffer that renders depth to a texture */
		void init_shadow_fb() {
			// create framebuffer
			glGenFramebuffers(1, &g_depth_fb);
			glBindFramebuffer(GL_FRAMEBUFFER, g_depth_fb);

			// create texture for framebuffer
			glGenTextures(1, &g_depth_fb_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_depth_fb_tex);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_DEPTH_COMPONENT,
				g_shadow_size,
				g_shadow_size,
				0,
				GL_DEPTH_COMPONENT,
				GL_UNSIGNED_BYTE,
				NULL
				);
			// bi-linear filtering might help, but might make it less accurate too
			//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			// clamp to edge. clamp to border may reduce artifacts outside light frustum
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach depth texture to framebuffer
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depth_fb_tex, 0);

			// tell framebuffer not to use any colour drawing outputs
			GLenum draw_bufs[] = { GL_NONE };
			glDrawBuffers(1, draw_bufs);

			// this *should* avoid a GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER error that
			// comes on mac - invalid framebuffer due to having only a depth buffer and
			// no colour buffer specified.
			glReadBuffer(GL_NONE);

			// bind default framebuffer again
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		/* do a rendering pass writing just the depth to a texture */
		void render_shadow_casting() {
			/* similar to epsilon offset */
			/*
			glEnable (GL_POLYGON_OFFSET_FILL);
			glPolygonOffset (-1,-1); */

			// bind framebuffer that renders to texture instead of screen
			glBindFramebuffer(GL_FRAMEBUFFER, g_depth_fb);
			// set the viewport to the size of the shadow map
			glViewport(0, 0, g_shadow_size, g_shadow_size);
			// clear the shadow map to black (or white)
			glClearColor(0.0, 0.0, 0.0, 1.0);
			// no need to clear the colour buffer
			glClear(GL_DEPTH_BUFFER_BIT);
			// bind out shadow-casting shader from the previous section
			glUseProgram(g_depth_sp);
			// send in the view and projection matrices from the light
			glUniformMatrix4fv(g_depth_V_loc, 1, GL_FALSE, g_caster_V.m);
			glUniformMatrix4fv(g_depth_P_loc, 1, GL_FALSE, g_caster_P.m);
			// model matrix does nothing for the monkey - make it an identity matrix

			//VAO 1
			glBindVertexArray(objVAO);
			for (int i = 0; i < 4; i++) {
				glUniformMatrix4fv(g_depth_M_loc, 1, GL_FALSE, g_objs_Ms[i].m);
				glDrawArrays(GL_TRIANGLES, 0, obj_point_count);
			}

			//VAO 2
			glBindVertexArray(objVAO2);
			for (int i = 0; i < 1; i++) {
				glUniformMatrix4fv(g_depth_M_loc, 1, GL_FALSE, g_objs_Ms[i].m);
				glDrawArrays(GL_TRIANGLES, 0, obj_point_count2);
			}

			//VAO 3
			glBindVertexArray(objVAO3);
			for (int i = 0; i < 1; i++) {
				glUniformMatrix4fv(g_depth_M_loc, 1, GL_FALSE, g_objs_Ms[i].m);
				glDrawArrays(GL_TRIANGLES, 0, obj_point_count3);
			}


			// bind the default framebuffer again
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glDisable(GL_POLYGON_OFFSET_FILL);
		}


		//Desenha o chao
		void Ground_Scene() {

			GLuint points_vbo;

			GLfloat gp_pos[] = {
				-20.0, -1.0, -20.0,
				-20.0, -1.0, 20.0,
				20.0, -1.0, 20.0,
				20.0, -1.0, 20.0,
				20.0, -1.0, -20.0,
				-20.0, -1.0, -20.0
			};
			g_Ground_Scene_point_count = sizeof(gp_pos) / sizeof(GLfloat) / 3;

			/* CRIACAO DO VAO E VBO */
			glGenVertexArrays(1, &g_Ground_Scene_vao);
			glBindVertexArray(g_Ground_Scene_vao);

			glGenBuffers(1, &points_vbo);

			glBindBuffer(GL_ARRAY_BUFFER, points_vbo);

			glBufferData(
				GL_ARRAY_BUFFER, sizeof(gp_pos), gp_pos, GL_STATIC_DRAW
				);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
		}

		// a posição dos objetos na cena.
		vec3 obj_pos_wor[] = {
			vec3(-4.0, 0.0, 0.0),
			vec3(2.0, 0.0, 3.0),
			vec3(-2.0, 0.0, -2.0),
			vec3(1.5, 1.0, -6.0)
		};


		void create_shadow_caster() {
			// create a view matrix for the shadow caster
			vec3 light_pos(10.0f, 10.0f, 0.0f);
			vec3 light_target(0.0f, 0.0f, 0.0f);
			vec3 up_dir(normalise(vec3(-1.0f, 1.0f, 0.0f)));
			g_caster_V = look_at(light_pos, light_target, up_dir);

			// create a projection matrix for the shadow caster
			float near = 1.0f;
			float far = 30.0f;
			float fov = 50.0f;
			float aspect = 3.0f;
			g_caster_P = perspective(fov, aspect, near, far);
		}


		void Sombra()
		{
			g_depth_sp = create_programme_from_files(DEPTH_VS, DEPTH_FS);
			g_depth_M_loc = glGetUniformLocation(g_depth_sp, "M");
			g_depth_V_loc = glGetUniformLocation(g_depth_sp, "V");
			g_depth_P_loc = glGetUniformLocation(g_depth_sp, "P");

			create_shadow_caster();

			//shader para o chao
			glUseProgram(g_plain_sp);
			//cameras
			glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(g_plain_P_loc, 1, GL_FALSE, g_camera_P.m);
			//caster de sombra - futuro
			glUniformMatrix4fv(g_plain_caster_V_loc, 1, GL_FALSE, g_caster_V.m);
			glUniformMatrix4fv(g_plain_caster_P_loc, 1, GL_FALSE, g_caster_P.m);
			glUniform1f(g_plain_shad_resolution_loc, (GLfloat)g_shadow_size);

			for (int i = 0; i < 6; i++) {
				g_objs_Ms[i] = translate(identity_mat4(), obj_pos_wor[i]);
			}
			render_shadow_casting();
		}



		void TirarSombra()
		{
			g_caster_P = perspective(0, 0, 0, 0);

			//shader para o chao
			glUseProgram(g_plain_sp);
			//cameras
			glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(g_plain_P_loc, 1, GL_FALSE, g_camera_P.m);
			//caster de sombra - futuro
			glUniformMatrix4fv(g_plain_caster_V_loc, 1, GL_FALSE, g_caster_V.m);
			glUniformMatrix4fv(g_plain_caster_P_loc, 1, GL_FALSE, g_caster_P.m);
			glUniform1f(g_plain_shad_resolution_loc, (GLfloat)g_shadow_size);

			for (int i = 0; i < 6; i++) {
				g_objs_Ms[i] = translate(identity_mat4(), obj_pos_wor[i]);
			}
			render_shadow_casting();
		}



		//start
		int main() {

		#pragma region Start

			//-------------start e glfw funcs
			assert(start_gl());

			//init 
			init_shadow_fb();



			glEnable(GL_CULL_FACE);

			// enable depth-testing
			glEnable(GL_DEPTH_TEST);
			// depth-testing interprets a smaller value as "closer"
			glDepthFunc(GL_LESS);
			// cull back face
			glCullFace(GL_BACK);
			// set counter-clock-wise vertex order to mean the front
			glFrontFace(GL_CCW);
			// grey background to help spot mistakes
			glClearColor(0.2, 0.2, 0.2, 1.0);

			glViewport(0, 0, g_gl_width, g_gl_height);

		#pragma endregion

			g_depth_sp = create_programme_from_files(DEPTH_VS, DEPTH_FS);
			g_depth_M_loc = glGetUniformLocation(g_depth_sp, "M");
			g_depth_V_loc = glGetUniformLocation(g_depth_sp, "V");
			g_depth_P_loc = glGetUniformLocation(g_depth_sp, "P");

			//sombras
			//	create_shadow_caster();


		#pragma region Texturas - Shaders
			/////////////////////////////- PARTE DAS TEXTURAS - 

			//vao
			/*GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);*/


			//arquivo obj
			//assert(load_mesh(MESH_FILE));


			obj_point_count = 0;
			assert(load_mesh(MESH_FILE, &objVAO, &obj_point_count));

			obj_point_count2 = 0;
			assert(load_mesh(MESH_FILE2, &objVAO2, &obj_point_count2));

			obj_point_count3 = 0;
			assert(load_mesh(MESH_FILE3, &objVAO3, &obj_point_count3));

			//program de texturas - shaders
			GLuint shader_programme = create_programme_from_files(
				VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE);


			// load normal map para textura
			GLuint nmap_tex;
			assert(load_texture(NMAP_IMG_FILE, &nmap_tex));

			//-------------------------------------------------------------------------------------------FORMAS
			//gramado
			Ground_Scene();

			//-------------SHaders
			//shaders do gramado
			g_plain_sp = create_programme_from_files(PLAIN_VS, PLAIN_FS);

		#pragma endregion


		#pragma region Camera
			//-----------------------------------------------------------------------------------------CAMERA
			// proximidade da camera
			float near = 0.1f;

			// longitividade da camera
			float far = 100.0f;

			// camp de visao da camera
			float fov = 67.0f; // convert 67 degrees to radians;

			//aspect ratio = o "quadrado" que sera apresentado - largura/altura
			float aspect = (float)g_gl_width / (float)g_gl_height;

			//seto a perspectiva da camera
			g_camera_P = perspective(fov, aspect, near, far);

			// 1 unit per second
			float cam_speed = 5.0f;
			// 10 degrees per second
			float cam_heading_speed = 100.0f;
			// don't start at zero, or we will be too close
			vec3 cam_pos(0.0f, 1.0f, 5.0f);
			versor quaternion = quat_from_axis_deg(0.0f, 0.0f, 1.0f, 0.0f);

			//foward da camera
			vec4 fwd(0.0f, 0.0f, -1.0f, 0.0f);
			//right
			vec4 rgt(1.0f, 0.0f, 0.0f, 0.0f);
			//up
			vec4 up(0.0f, 1.0f, 0.0f, 0.0f);
			//translacao
			mat4 T = translate(identity_mat4(), cam_pos);
			//rotacao
			mat4 R = quat_to_mat4(quaternion);
			//view
			g_camera_V = inverse(R) * inverse(T);

		#pragma endregion


		#pragma region Rendering
			//---------------Defaults do rendering


			//model matrix - obj
			int model_mat_location = glGetUniformLocation(shader_programme, "model");
			//view matrix - obj
			int view_mat_location = glGetUniformLocation(shader_programme, "view");
			//view proj - obj
			int proj_mat_location = glGetUniformLocation(shader_programme, "proj");

			//program obj
			glUseProgram(shader_programme);
			//matrizes e camera para o obj
			//glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, identity_mat4().m);

			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, g_camera_P.m);

			for (int i = 0; i < 4; i++) {
				g_objs_Ms[i] = translate(identity_mat4(), obj_pos_wor[i]);
			}


			//chao
			// --------------- pegar todas variaveis uniformees no vertice / glsl
			//uniform matrix no glsl
			g_plain_M_loc = glGetUniformLocation(g_plain_sp, "M");
			//uniform matrix view no glsl
			g_plain_V_loc = glGetUniformLocation(g_plain_sp, "V");
			//uniform matrix projection no glsl
			g_plain_P_loc = glGetUniformLocation(g_plain_sp, "P");

			//uniform caster_view no glsl
			g_plain_caster_V_loc = glGetUniformLocation(g_plain_sp, "caster_V");
			//uniform caster_projection no glsl
			g_plain_caster_P_loc = glGetUniformLocation(g_plain_sp, "caster_P");
			//cor
			g_plain_colour_loc = glGetUniformLocation(g_plain_sp, "colour");

			//resolucao do shader no glsl
			g_plain_shad_resolution_loc =
				glGetUniformLocation(g_plain_sp, "shad_resolution");




			//shader para o chao
			glUseProgram(g_plain_sp);
			//cameras
			glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(g_plain_P_loc, 1, GL_FALSE, g_camera_P.m);
			//caster de sombra - futuro
			glUniformMatrix4fv(g_plain_caster_V_loc, 1, GL_FALSE, g_caster_V.m);
			glUniformMatrix4fv(g_plain_caster_P_loc, 1, GL_FALSE, g_caster_P.m);
			glUniform1f(g_plain_shad_resolution_loc, (GLfloat)g_shadow_size);

			for (int i = 0; i < 6; i++) {
				g_objs_Ms[i] = translate(identity_mat4(), obj_pos_wor[i]);
			}

		#pragma endregion 


		#pragma region Loop
			//////////////////////////////////////////////////////////// DEPOIS DE TUDO SETADO, START NO LOOP
			while (!glfwWindowShouldClose(g_window)) {



				// atualiza os timers = precisa para uso dos teclados
				static double previous_seconds = glfwGetTime();
				double current_seconds = glfwGetTime();
				double elapsed_seconds = current_seconds - previous_seconds;
				previous_seconds = current_seconds;
				_update_fps_counter(g_window);



				render_shadow_casting();



				// wipe the drawing surface clear
				glViewport(0, 0, g_gl_width, g_gl_height);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glClear(GL_DEPTH_BUFFER_BIT);




				/* chao */
				glUseProgram(g_plain_sp);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, g_depth_fb_tex);


				glUniform3f(g_plain_colour_loc, 0.0, 1.0, 0.0); /* verde */
				glBindVertexArray(g_Ground_Scene_vao);
				glUniformMatrix4fv(g_plain_M_loc, 1, GL_FALSE, identity_mat4().m);
				glDrawArrays(GL_TRIANGLES, 0, g_Ground_Scene_point_count);




				/* macaco */
				glUseProgram(shader_programme);

				//glActiveTexture(GL_TEXTURE0);
				//glBindTexture(GL_TEXTURE_2D, g_depth_fb_tex);

				glBindVertexArray(objVAO);
				for (int i = 0; i < 4; i++)
				{
					glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, g_objs_Ms[i].m);
					glDrawArrays(GL_TRIANGLES, 0, obj_point_count);

				}

				glBindVertexArray(objVAO2);
				glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, g_objs_Ms[0].m);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, obj_point_count2);

				glBindVertexArray(objVAO3);
				glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, g_objs_Ms[0].m);
				glDrawArrays(GL_TRIANGLES, 0, obj_point_count3);


				//atualizar eventos 
				glfwPollEvents();

				// control keys
				bool cam_moved = false;
				float cam_yaw = 0.0f; // y-rotation in degrees
				float cam_pitch = 0.0f; // y-rotation in degrees
				float cam_roll = 0.0f; // y-rotation in degrees
				vec3 move(0.0, 0.0, 0.0);


				//Sombra
				if (glfwGetKey(g_window, GLFW_KEY_P)) {
					Sombra();
				}
				if (glfwGetKey(g_window, GLFW_KEY_O)) {
					TirarSombra();
				}
				//Sombra



				if (glfwGetKey(g_window, GLFW_KEY_A)) {
					move.v[0] -= cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_D)) {
					move.v[0] += cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_Q)) {
					move.v[1] += cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_E)) {
					move.v[1] -= cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_W)) {
					move.v[2] -= cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				if (glfwGetKey(g_window, GLFW_KEY_S)) {
					move.v[2] += cam_speed * elapsed_seconds;
					cam_moved = true;
				}
				// rotations are else-if to prevent roll when pitch and yawing
				if (glfwGetKey(g_window, GLFW_KEY_LEFT)) {
					cam_yaw += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					// create a quaternion representing change in heading (the yaw)
					versor q_yaw = quat_from_axis_deg(cam_yaw, 0.0f, 1.0f, 0.0f);
					// add yaw rotation to the camera's current orientation
					quaternion = q_yaw * quaternion;
				}
				else if (glfwGetKey(g_window, GLFW_KEY_RIGHT)) {
					cam_yaw -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_yaw = quat_from_axis_deg(cam_yaw, 0.0f, 1.0f, 0.0f);
					quaternion = q_yaw * quaternion;
				}
				else if (glfwGetKey(g_window, GLFW_KEY_UP)) {
					cam_pitch += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_pitch = quat_from_axis_deg(cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					quaternion = q_pitch * quaternion;
				}
				else if (glfwGetKey(g_window, GLFW_KEY_DOWN)) {
					cam_pitch -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_pitch = quat_from_axis_deg(cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					quaternion = q_pitch * quaternion;
				}
				else if (glfwGetKey(g_window, GLFW_KEY_Z)) {
					cam_roll -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_roll = quat_from_axis_deg(cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]);
					quaternion = q_roll * quaternion;
				}
				else if (glfwGetKey(g_window, GLFW_KEY_C)) {
					cam_roll += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					versor q_roll = quat_from_axis_deg(cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]);
					quaternion = q_roll * quaternion;
				}
				// update view matrix
				if (cam_moved) {
					// re-calculate local axes so can move fwd in dir cam is pointing
					R = quat_to_mat4(quaternion);
					fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
					rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
					up = R * vec4(0.0, 1.0, 0.0, 0.0);

					cam_pos = cam_pos + vec3(fwd) * -move.v[2];
					cam_pos = cam_pos + vec3(0.0f, 1.0f, 0.0f) * move.v[1];
					cam_pos = cam_pos + vec3(rgt) * move.v[0];
					mat4 T = translate(identity_mat4(), cam_pos);
					g_camera_V = inverse(R) * inverse(T);
					//glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);
					//glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, g_camera_V.m);
					glUseProgram(g_plain_sp);
					glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);

					glUseProgram(shader_programme);
					glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, g_camera_V.m);
				}

				if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
					glfwSetWindowShouldClose(g_window, 1);
				}
				// put the stuff we've been drawing onto the display
				glfwSwapBuffers(g_window);
			}
		#pragma endregion

			// close GL context and any other GLFW resources
			glfwTerminate();
			return 0;
		}