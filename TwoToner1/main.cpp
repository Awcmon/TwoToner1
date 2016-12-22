#include <SDL.h>
#undef main
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb_image_write.h>

#include <iostream>
#include <algorithm>

#include <string>
#include <fstream>
#include <streambuf>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

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

	uniform mat4 scale;

	void main() {
		Color = vec3(1.0, 1.0, 1.0);
		Texcoord = texcoord;
		gl_Position = scale * vec4(position, 0.0, 1.0);
	}
);

const GLchar* fragmentSource = GLSL(
	//ins, outs
	in vec3 Color;
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D tex;
	uniform float threshold;

	//constants
	float sqrt3 = 1.73205080757;
		//color settings
	vec3 colFilled = vec3(0.4, 0.4, 0.4);
	vec3 colBlank = vec3(0.6, 0.6, 0.6);
	//vec3 colFilled = vec3(0.68235, 0.5921568627, 0.4666667);
	//vec3 colBlank = vec3(0.7960784314, 0.7019607843, 0.568627451);
		//vignette settings
	float roundedRectW = 0.7;
	float roundedRectH = 0.7;
	float roundedRectRadius = 0.1; //radius beyond the rectangle

	//x,y of center of rect
	float sqrDistFromRect(vec2 p, vec2 r, float w, float h)
	{
		float dx = max(abs(p.x - r.x) - w / 2.0, 0.0);
		float dy = max(abs(p.y - r.y) - h / 2.0, 0.0);
		return dx * dx + dy * dy;
	}

	float sqr(float a)
	{
		return a*a;
	}

	void main() {
		//sqr'd dist from the rect at the center of the screen
		float sqrdist = sqrDistFromRect(Texcoord.xy, vec2(0.5, 0.5), roundedRectW, roundedRectH);
		//If within boundaries of rounded rectangle
		if(sqrdist < sqr(roundedRectRadius))
		{
			//Colorize
			if (length(texture(tex, Texcoord).xyz) > threshold*sqrt3)
			{
				outColor = vec4(colBlank, 1.0);
			}
			else
			{
				outColor = vec4(colFilled, 1.0);
			}
		}
		else 
		{
			//Colorize
			//(1.0-(sqrdist*100.0))*threshold*sqrt3 is enough for going from 1 to 0 polynomially past the rounded rectangle's bounds
			//sqrtdist - 0.01 is to compensate for the 0.1 dist buffer so we get distance from rounded rectangle rather than distance from rectangle. max takes care of the negative.
			//having the fade go from 1.0 to 0.5 of threshold polynomially works much better visually
			float tHold = (0.5 + (0.5 - (max(sqrdist - 0.01, 0)*80.0)))*threshold*sqrt3;
			if (length(texture(tex, Texcoord).xyz) > tHold)
			{
				outColor = vec4(0.6, 0.6, 0.6, 1.0);
			}
			else
			{
				outColor = vec4(0.4, 0.4, 0.4, 1.0);
			}

			//float tHold = (0.5 + (0.5 - (max(sqrdist - 0.01, 0)*80.0)))*threshold*sqrt3;
			//outColor = vec4(tHold, tHold, tHold, 1.0);
		}
	}
);

/*
const GLchar* fragmentSource = GLSL(
	in vec3 Color;
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D tex;
	uniform float threshold;

	float sqrt3 = 1.73205080757;

	void main() {
		if (length(texture(tex, Texcoord).xyz) > threshold*sqrt3)
		{
			outColor = vec4(0.6, 0.6, 0.6, 1.0);
		}
		else
		{
			outColor = vec4(0.4, 0.4, 0.4, 1.0);
		}
	}
);
*/

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

	/* Window fits the entire pic to largest possible extent of the screen (1920x1080), nothing more
	float ratioX = 1920.0f / (float)x;
	float ratioY = 1080.0f / (float)y;
	int resX = (int)((float)x*std::min(ratioX, ratioY));
	int resY = (int)((float)y*std::min(ratioX, ratioY));
	*/
	
	float resX = 1920.0f;
	float resY = 1080.0f;
	float xf = (float)x;
	float yf = (float)y;
	float ratioX = resX/xf;
	float ratioY = resY/yf;
	float scaleX = (xf*std::min(ratioX, ratioY))/resX;
	float scaleY = (yf*std::min(ratioX, ratioY))/resY;

	std::cout << resX << "\n";
	std::cout << resY << "\n";
	std::cout << scaleX << "\n";
	std::cout << scaleY << "\n";

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//SDL_Window* window = SDL_CreateWindow("OpenGL", 0, 0, resX, resY, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
	SDL_Window* window = SDL_CreateWindow("OpenGL", 0, 0, 1920,1080, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
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

	// Create and compile the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

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


	// Calculate transformation
	GLint uniScale = glGetUniformLocation(shaderProgram, "scale");
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));
	glUniformMatrix4fv(uniScale, 1, GL_FALSE, glm::value_ptr(scale));

	SDL_Event e;
	while (true)
	{
		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) 
			{ 
				break; 
			}
			else if (e.type == SDL_KEYUP) 
			{ 
				switch( e.key.keysym.sym ) 
				{ 
				case SDLK_s: 
					std::cout << "fuk";
					break; 
				} 
			}
		}

		int mx, my;
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			SDL_GetMouseState(&mx, &my);
		}

		GLint threshold = glGetUniformLocation(shaderProgram, "threshold");
		glUniform1f(threshold, (float)mx/(float)resX);

		// Clear the screen to black
		glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
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
