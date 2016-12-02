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
		#include "gl_utils.h"
		#include "maths_funcs.h"
		#include "camera.h"
		#include "ogldev_pipeline.h"
		//#include "lighting_technique.h"


		#define WINDOW_WIDTH 1024
		#define WINDOW_HEIGHT 768

		#define MESH_FILE "monkey2.obj"
        #define NMAP_IMG_FILE "brickwork_normal-map.png"

		#define PLAIN_VS "plain.vert"
		#define PLAIN_FS "plain.frag"
		#define VERTEX_SHADER_FILE "test_vs.glsl"
		#define FRAGMENT_SHADER_FILE "test_fs.glsl"

		#define DEBUG_VS "ss_quad.vert"
		#define DEBUG_FS "ss_quad.frag"
		#define DEPTH_VS "depth.vert"
		#define DEPTH_FS "depth.frag"
		#include "stb_image.h" 
		#define NUM_SPHERES 4


		GLuint g_plain_sp;
		GLint g_plain_M_loc; /* model matrix location */
		GLint g_plain_V_loc; /* virtual camera view matrix location */
		GLint g_plain_P_loc; /* virtual camera projection matrix location */

		GLint matr_mat_location; /* model matrix location */
		GLint view_mat_location; /* virtual camera view matrix location */
		GLint proj_mat_location; /* virtual camera projection matrix location */

		GLint g_plain_caster_V_loc; /* shadow caster view matrix location */
		GLint g_plain_caster_P_loc; /* shadow caster projection matrix location */
		GLint g_plain_colour_loc; /* a uniform to switch colour */
		GLint g_plain_shad_resolution_loc;

		int g_shadow_size = 1024;


		//camera
		/* the virtual camera's view and projection matrices */
		mat4 g_camera_V;
		mat4 g_camera_P;
		Camera* pGameCamera = NULL;

		/* shadow caster's view and projection matrices. in practice each light source
		will be a shadow caster have a set of these. we only have one caster here */
		mat4 g_caster_V;
		mat4 g_caster_P;

		GLuint g_depth_fb;
		GLuint g_depth_fb_tex;

		//projecao
		PersProjInfo gPersProjInfo;

		//vertex buffer obj
		GLuint VBO;
		//index buffer obj
		GLuint IBO;
		//world location
		GLuint gWVPLocation;
		GLfloat* g_vp = NULL; // array of vertex points
		GLfloat* g_vn = NULL; // array of vertex normals
		GLfloat* g_vt = NULL; // array of texture coordinates
		GLfloat* g_vtans = NULL;
		int g_point_count = 0;


		bool load_mesh(const char* file_name) {
			const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate |
				aiProcess_CalcTangentSpace);
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

			// get first mesh only
			const aiMesh* mesh = scene->mMeshes[0];
			printf("    %i vertices in mesh[0]\n", mesh->mNumVertices);
			g_point_count = mesh->mNumVertices;

			// allocate memory for vertex points
			if (mesh->HasPositions()) {
				printf("mesh has positions\n");
				g_vp = (GLfloat*)malloc(g_point_count * 3 * sizeof(GLfloat));
			}
			if (mesh->HasNormals()) {
				printf("mesh has normals\n");
				g_vn = (GLfloat*)malloc(g_point_count * 3 * sizeof(GLfloat));
			}
			if (mesh->HasTextureCoords(0)) {
				printf("mesh has texture coords\n");
				g_vt = (GLfloat*)malloc(g_point_count * 2 * sizeof(GLfloat));
			}
			if (mesh->HasTangentsAndBitangents()) {
				printf("mesh has tangents\n");
				g_vtans = (GLfloat*)malloc(g_point_count * 4 * sizeof(GLfloat));
			}

			for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
				if (mesh->HasPositions()) {
					const aiVector3D* vp = &(mesh->mVertices[v_i]);
					g_vp[v_i * 3] = (GLfloat)vp->x;
					g_vp[v_i * 3 + 1] = (GLfloat)vp->y;
					g_vp[v_i * 3 + 2] = (GLfloat)vp->z;
				}
				if (mesh->HasNormals()) {
					const aiVector3D* vn = &(mesh->mNormals[v_i]);
					g_vn[v_i * 3] = (GLfloat)vn->x;
					g_vn[v_i * 3 + 1] = (GLfloat)vn->y;
					g_vn[v_i * 3 + 2] = (GLfloat)vn->z;
				}
				if (mesh->HasTextureCoords(0)) {
					const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
					g_vt[v_i * 2] = (GLfloat)vt->x;
					g_vt[v_i * 2 + 1] = (GLfloat)vt->y;
				}
				if (mesh->HasTangentsAndBitangents()) {
					const aiVector3D* tangent = &(mesh->mTangents[v_i]);
					const aiVector3D* bitangent = &(mesh->mBitangents[v_i]);
					const aiVector3D* normal = &(mesh->mNormals[v_i]);

					// put the three vectors into my vec3 struct format for doing maths
					vec3 t(tangent->x, tangent->y, tangent->z);
					vec3 n(normal->x, normal->y, normal->z);
					vec3 b(bitangent->x, bitangent->y, bitangent->z);
					// orthogonalise and normalise the tangent so we can use it in something
					// approximating a T,N,B inverse matrix
					vec3 t_i = normalise(t - n * dot(n, t));

					// get determinant of T,B,N 3x3 matrix by dot*cross method
					float det = (dot(cross(n, t), b));
					if (det < 0.0f) {
						det = -1.0f;
					}
					else {
						det = 1.0f;
					}

					// push back 4d vector for inverse tangent with determinant
					g_vtans[v_i * 4] = t_i.v[0];
					g_vtans[v_i * 4 + 1] = t_i.v[1];
					g_vtans[v_i * 4 + 2] = t_i.v[2];
					g_vtans[v_i * 4 + 3] = det;
				}
			}

			aiReleaseImport(scene);

			printf("mesh loaded\n");

			return true;
		}
	
		bool load_texture(const char* file_name, GLuint* tex) {
			int x, y, n;
			int force_channels = 4;
			unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
			if (!image_data) {
				fprintf(stderr, "ERROR: could not load %s\n", file_name);
				return false;
			}
			// NPOT check
			if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
				fprintf(
					stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
					);
			}
			int width_in_bytes = x * 4;
			unsigned char *top = NULL;
			unsigned char *bottom = NULL;
			unsigned char temp = 0;
			int half_height = y / 2;

			for (int row = 0; row < half_height; row++) {
				top = image_data + row * width_in_bytes;
				bottom = image_data + (y - row - 1) * width_in_bytes;
				for (int col = 0; col < width_in_bytes; col++) {
					temp = *top;
					*top = *bottom;
					*bottom = temp;
					top++;
					bottom++;
				}
			}
			glGenTextures(1, tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *tex);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				x,
				y,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				image_data
				);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			GLfloat max_aniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
			// set the maximum!
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
			return true;
		}

	
		void KeyBoard(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			pGameCamera->OnKeyboard(key);
		}

		int g_Ground_Scene_point_count;
		GLuint g_Ground_Scene_vao;

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

			/* create VBO and VAO here */
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
		//start
		int main() {

			//-------------start e glfw funcs
			assert(start_gl());

			// cull face
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



			assert(load_mesh("suzanne.obj"));

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			GLuint points_vbo;
			if (NULL != g_vp) {
				glGenBuffers(1, &points_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
				glBufferData(
					GL_ARRAY_BUFFER, 3 * g_point_count * sizeof(GLfloat), g_vp, GL_STATIC_DRAW
					);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);
			}

			GLuint normals_vbo;
			if (NULL != g_vn) {
				glGenBuffers(1, &normals_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
				glBufferData(
					GL_ARRAY_BUFFER, 3 * g_point_count * sizeof(GLfloat), g_vn, GL_STATIC_DRAW
					);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(1);
			}

			GLuint texcoords_vbo;
			if (NULL != g_vt) {
				glGenBuffers(1, &texcoords_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
				glBufferData(
					GL_ARRAY_BUFFER, 2 * g_point_count * sizeof(GLfloat), g_vt, GL_STATIC_DRAW
					);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(2);
			}

			GLuint tangents_vbo;
			if (NULL != g_vtans) {
				glGenBuffers(1, &tangents_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, tangents_vbo);
				glBufferData(
					GL_ARRAY_BUFFER,
					4 * g_point_count * sizeof(GLfloat),
					g_vtans,
					GL_STATIC_DRAW
					);
				glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(3);
			}

			GLuint shader_programme = create_programme_from_files(
				"test_vs.glsl", "test_fs.glsl");
			/* load the mesh using assimp */
		//	GLuint monkey_vao;
			//int monkey_point_count = 0;
			//assert(load_mesh(MESH_FILE, &monkey_vao, &monkey_point_count));



			//-------------Formas
			//gramado
			Ground_Scene();

			//-------------SHaders
			//shaders do gramado
			g_plain_sp = create_programme_from_files(PLAIN_VS, PLAIN_FS);

			////shaders do macaco
			//GLuint shader_programme = create_programme_from_files(
			//	VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE
			//	);


			//---------Camera
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
			vec4 fwd(0.0f, 0.0f, -1.0f, 0.0f);
			vec4 rgt(1.0f, 0.0f, 0.0f, 0.0f);
			vec4 up(0.0f, 1.0f, 0.0f, 0.0f);
			mat4 T = translate(identity_mat4(), cam_pos);
			mat4 R = quat_to_mat4(quaternion);
			g_camera_V = inverse(R) * inverse(T);


			int model_mat_location = glGetUniformLocation(shader_programme, "model");
			int view_mat_location = glGetUniformLocation(shader_programme, "view");
			int proj_mat_location = glGetUniformLocation(shader_programme, "proj");
			glUseProgram(shader_programme);
			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, identity_mat4().m);
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, g_camera_P.m);

			// load normal map image into texture
			GLuint nmap_tex;
			assert(load_texture(NMAP_IMG_FILE, &nmap_tex));

			


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


			//---------------Defaults do rendering
			glUseProgram(g_plain_sp);
			glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_camera_V.m);
			glUniformMatrix4fv(g_plain_P_loc, 1, GL_FALSE, g_camera_P.m);
			glUniformMatrix4fv(g_plain_caster_V_loc, 1, GL_FALSE, g_caster_V.m);
			glUniformMatrix4fv(g_plain_caster_P_loc, 1, GL_FALSE, g_caster_P.m);
			glUniform1f(g_plain_shad_resolution_loc, (GLfloat)g_shadow_size);




			//////////////////////////////////////////////////////////// DEPOIS DE TUDO SETADO, START NO LOOP
			while (!glfwWindowShouldClose(g_window)) {


				// atualiza os timers = precisa para uso dos teclados
				static double previous_seconds = glfwGetTime();
				double current_seconds = glfwGetTime();
				double elapsed_seconds = current_seconds - previous_seconds;
				previous_seconds = current_seconds;
				_update_fps_counter(g_window);


				// wipe the drawing surface clear
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport(0, 0, g_gl_width, g_gl_height);


				glUseProgram(g_plain_sp);

				/* ground plane (receives shadows) */
				glUniform3f(g_plain_colour_loc, 0.0, 1.0, 0.0); /* green */
				glBindVertexArray(g_Ground_Scene_vao);
				glUniformMatrix4fv(g_plain_M_loc, 1, GL_FALSE, identity_mat4().m);
				glDrawArrays(GL_TRIANGLES, 0, g_Ground_Scene_point_count);


				glUseProgram(shader_programme);
				glBindVertexArray(vao);
				// draw points 0-3 from the currently bound VAO with current in-use shader
				glDrawArrays(GL_TRIANGLES, 0, g_point_count);
				glUniformMatrix4fv(matr_mat_location, 1, GL_FALSE, identity_mat4().m);



				glfwPollEvents();



				// control keys
				bool cam_moved = false;
				float cam_yaw = 0.0f; // y-rotation in degrees
				float cam_pitch = 0.0f; // y-rotation in degrees
				float cam_roll = 0.0f; // y-rotation in degrees
				vec3 move(0.0, 0.0, 0.0);
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
				/* switch between looking from virtual camera and shadow caster matrices */
				/*	if (glfwGetKey(g_window, GLFW_KEY_SPACE)) {
				glUniformMatrix4fv(g_plain_V_loc, 1, GL_FALSE, g_caster_V.m);
				glUniformMatrix4fv(g_plain_P_loc, 1, GL_FALSE, g_caster_P.m);
				}*/




				/* draw ss quad */
				//	glActiveTexture(GL_TEXTURE0);
				//glBindTexture(GL_TEXTURE_2D, g_depth_fb_tex);
				//glUseProgram(g_debug_sp);
				//glBindVertexArray(g_ss_quad_vao);
				//glDrawArrays(GL_TRIANGLES, 0, g_ss_quad_point_count);

				if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
					glfwSetWindowShouldClose(g_window, 1);
				}
				// put the stuff we've been drawing onto the display
				glfwSwapBuffers(g_window);
			}

			// close GL context and any other GLFW resources
			glfwTerminate();
			return 0;
		}