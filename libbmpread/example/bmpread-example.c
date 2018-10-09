/******************************************************************************
* libbmpread - tiny, fast bitmap (.bmp) image file loader                     *
*              <https://github.com/chazomaticus/libbmpread>                   *
* Copyright (C) 2005, 2012, 2016, 2018 Charles Lindsay <chaz@chazomatic.us>   *
*                                                                             *
*  This software is provided 'as-is', without any express or implied          *
*  warranty.  In no event will the authors be held liable for any damages     *
*  arising from the use of this software.                                     *
*                                                                             *
*  Permission is granted to anyone to use this software for any purpose,      *
*  including commercial applications, and to alter it and redistribute it     *
*  freely, subject to the following restrictions:                             *
*                                                                             *
*  1. The origin of this software must not be misrepresented; you must not    *
*     claim that you wrote the original software. If you use this software    *
*     in a product, an acknowledgment in the product documentation would be   *
*     appreciated but is not required.                                        *
*  2. Altered source versions must be plainly marked as such, and must not be *
*     misrepresented as being the original software.                          *
*  3. This notice may not be removed or altered from any source distribution. *
******************************************************************************/


#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmpread.h"


static GLuint texture = 0;


static void Error(int error, const char * description)
{
    fprintf(stderr, "Error: %s\n", description);
    exit(1);

    (void)error; /* Unused. */
}

static void Paint(GLFWwindow * window)
{
    int width;
    int height;

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.5f);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, 0.0f);
    glEnd();

    glPopMatrix();
}

int main(int argc, char * argv[])
{
    const char * file = NULL;
    unsigned int flags = 0;
    bmpread_t bmp;

    GLFWwindow * window;

    int i;

    puts("Example utility for libbmpread");
    puts("Copyright (C) 2005, 2012, 2016, 2018 Charles Lindsay <chaz@chazomatic.us>");
    puts("");

    for(i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "--alpha"))
        {
            flags |= BMPREAD_ALPHA;
            continue;
        }
        if(!strcmp(argv[i], "--any-size"))
        {
            flags |= BMPREAD_ANY_SIZE;
            continue;
        }
        if(!strcmp(argv[i], "--help"))
        {
            printf("Usage: %s [--alpha] [--any-size] <bmpfile>\n", argv[0]);
            puts("Loads <bmpfile> and attempts to display it on an OpenGL quad, stretched across");
            puts("the entire window, using GLFW.  If the image looks correct, libbmpread works!");
            puts("Alpha channels are ignored unless you pass --alpha.  The image must have power-");
            puts("of-two dimensions unless you pass --any-size.");
            return 0;
        }

        if(file) Error(0, "too many arguments; see --help");
        file = argv[i];
    }
    if(!file) Error(0, "missing bmpfile argument; see --help");

    printf("Loading %s...", file);
    if(!bmpread(file, flags, &bmp)) Error(0, "bmpread() failed");
    puts("OK");

    glfwSetErrorCallback(Error);
    glfwInit();
    atexit(glfwTerminate);

    glfwWindowHint(GLFW_VISIBLE, 0);
    glfwWindowHint(GLFW_DOUBLEBUFFER, 0);
    window = glfwCreateWindow(bmp.width, bmp.height, file, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetWindowRefreshCallback(window, Paint);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5f, 0.5f, -0.5f, 0.5f, 0.1f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, ((bmp.flags & BMPREAD_ALPHA) ? 4 : 3),
                 bmp.width, bmp.height, 0,
                 ((bmp.flags & BMPREAD_ALPHA) ? GL_RGBA : GL_RGB),
                 GL_UNSIGNED_BYTE, bmp.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glfwShowWindow(window);

    while(!glfwWindowShouldClose(window))
        glfwWaitEvents();

    glfwDestroyWindow(window);
    bmpread_free(&bmp);

    return 0;
}
