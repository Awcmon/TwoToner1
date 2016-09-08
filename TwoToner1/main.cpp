#include <SDL.h>
#undef main
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <algorithm>

#include <string>
#include <fstream>
#include <streambuf>

/*List of libraries*******
SDL2 //windowing and input
GLEW //OpenGL Extentions
stb_image //image loading
GLM //transforms and matrices
*************************/

#define GLSL(src) "#version 150 core\n" #src

std::string readFromFile(std::string file)
{
	std::ifstream ifs(file, std::ifstream::in);
	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	std::cout << content;
	return content;
}

const GLchar* vertexSource = GLSL(
	in vec2 position;
	in vec2 texcoord;
	out vec3 Color;
	out vec2 Texcoord;
	void main() {
		Color = vec3(1.0, 1.0, 1.0);
		Texcoord = texcoord;
		gl_Position = vec4(position, 0.0, 1.0);
	}
);

const GLchar* fragmentSource = GLSL(
	in vec3 Color;
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D tex;
	void main() {
		if (length(texture(tex, Texcoord).xyz) > 0.9)
		{
			outColor = vec4(0.6, 0.6, 0.6, 1.0);
		}
		else
		{
			outColor = vec4(0.4, 0.4, 0.4, 1.0);
		}
	}
);

int main(int argc, char *argv[])
{
	//const GLchar* vertexSource = readFromFile("twotone.vert").c_str();
	//const GLchar* fragmentSource = readFromFile("twotone.frag").c_str();

	if (argc < 2)
	{
		return 0;
	}

	int x, y, n;
	unsigned char *data = stbi_load(argv[1], &x, &y, &n, 4);

	float ratioX = 1920.0f / (float)x;
	float ratioY = 1080.0f / (float)y;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_Window* window = SDL_CreateWindow("OpenGL", 0, 0, (int)((float)x*std::min(ratioX,ratioY)), (int)((float)y*std::min(ratioX, ratioY)), SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	//Crate VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat vertices[] = {
		//  Position   Color             Texcoords
		-1.0f,  1.0f, 0.0f, 0.0f, // Top-left
		1.0f,  1.0f, 1.0f, 0.0f, // Top-right
		1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
		-1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Create and compile the vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Create and compile the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	// Load texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	/*
	int width, height;
	unsigned char* image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	*/

	/*
	int x, y, n;
	unsigned char *data = stbi_load("test.jpg", &x, &y, &n, 4);
	*/
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	SDL_Event windowEvent;
	while (true)
	{
		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT) break;
		}

		// Clear the screen to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw a rectangle from the 2 triangles using 6 indices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
	}

	glDeleteTextures(1, &tex);

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
