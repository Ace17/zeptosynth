#include <SDL.h>
#include <SDL_opengles2.h>
#include <stdint.h>
#include <stdio.h>

#include "SDL.h"
#include "profiler.h"
#include "unique.h"

namespace
{

const char* const vertex_shader = R"(
attribute vec2 attr_vertex;
attribute vec4 attr_color;
varying vec4 var_color;

void main(void)
{
  vec4 pos = vec4(attr_vertex.x, attr_vertex.y, 0, 1);
  var_color = attr_color;
  gl_Position = pos;
}
)";

const char* const fragment_shader = R"(
precision mediump float;

varying vec4 var_color;

void main()
{
  gl_FragColor = var_color;
}
)";

enum
{
  attrib_position,
  attrib_color,
};

void safeGl(const char* call, const char* path, int line)
{
  auto err = glGetError();
  if(err)
  {
    fprintf(stderr, "[%s:%d] %s returned %d\n", path, line, call, err);
    exit(1);
  }
}

#define SAFEGL(call)                                                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
    call;                                                                                                              \
    safeGl(#call, __FILE__, __LINE__);                                                                                 \
  } while(0)

int createShader(int type, const char* code)
{
  auto vs = glCreateShader(type);

  SAFEGL(glShaderSource(vs, 1, &code, nullptr));
  SAFEGL(glCompileShader(vs));

  GLint status;
  SAFEGL(glGetShaderiv(vs, GL_COMPILE_STATUS, &status));
  if(!status)
    throw Exception{"Shader compilation failed"};

  return vs;
}

bool keepGoing = true;

void readInput()
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
      keepGoing = false;
  }
}

struct Vertex
{
  float x, y;
  float r, g, b, a;
};

void drawScreen(GLuint vbo)
{
  static uint32_t time = 0;

  time++;

  SAFEGL(glClearColor(0, 0, 0, 1));
  SAFEGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  const float lfo1 = float(sin(time * 0.017) * 0.6);
  const float lfo2 = float(sin(time * 0.031) * 0.6);

  const Vertex vertices[] = {
        {/* xy */ +1, +1, /* rgba */ 1, 0, 0, 1},
        {/* xy */ +1, -1, /* rgba */ 0, 1, 0, 1},
        {/* xy */ -1, -1, /* rgba */ 0, 0, 1, 1},

        {/* xy */ -0.5, -0.5f + lfo1, /* rgba */ 1, 1, 1, 1},
        {/* xy */ -0.5, +0.5, /* rgba */ 1, 1, 1, 1},
        {/* xy */ +0.5f + lfo2, +0.5, /* rgba */ 1, 1, 1, 1},
  };

  SAFEGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

  SAFEGL(glEnableVertexAttribArray(attrib_position));
  SAFEGL(glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0 * sizeof(float))));

  SAFEGL(glEnableVertexAttribArray(attrib_color));
  SAFEGL(glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float))));

  SAFEGL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW));

  SAFEGL(glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(*vertices)));
}

void showDriverInfo()
{
  printf("Available video drivers: ");
  for(int i = 0; i < SDL_GetNumVideoDrivers(); ++i)
    printf("%s ", SDL_GetVideoDriver(i));
  printf("\n");

  printf("Available render drivers: ");
  for(int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
  {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(i, &info);
    printf("%s ", info.name);
  }
  printf("\n");

  printf("Using video driver: %s\n", SDL_GetCurrentVideoDriver());
}

void safeMain()
{
  SDL_Init(SDL_INIT_VIDEO);

  showDriverInfo();

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  auto window = SDL_CreateWindow("zeptosynth", 0, 0, 800, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if(!window)
    throw Exception{SDL_GetError()};

  auto context = SDL_GL_CreateContext(window);
  if(!context)
    throw Exception{SDL_GetError()};

  auto vs = createShader(GL_VERTEX_SHADER, vertex_shader);
  auto fs = createShader(GL_FRAGMENT_SHADER, fragment_shader);

  auto program = glCreateProgram();
  SAFEGL(glAttachShader(program, vs));
  SAFEGL(glAttachShader(program, fs));

  SAFEGL(glBindAttribLocation(program, attrib_position, "pos"));
  SAFEGL(glLinkProgram(program));

  SAFEGL(glUseProgram(program));

  GLuint vbo;
  SAFEGL(glGenBuffers(1, &vbo));

  while(keepGoing)
  {
    readInput();
    drawScreen(vbo);

    SDL_GL_SwapWindow(window);
    SDL_Delay(20);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

}

int main()
{
  try
  {
    safeMain();
    return 0;
  }
  catch(Exception const& e)
  {
    fprintf(stderr, "Fatal: %s\n", e.msg);
    return 1;
  }
}
