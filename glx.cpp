/**
 * (c) INEO Systrans 2013
 *
 * @file    glx.cpp
 * @brief   Application OpenGL minimaliste pour Linux (GLX) pour tester la Borne LCD Ucineo23
 *
 * Basé sur l'exemple http://www.talisman.org/opengl-1.1/Reference/glXIntro.html
 *
 * @author  20/12/2013 SRO
*/
#include <GL/glx.h>
#include <GL/gl.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>


#define DOUBLE_BUFFERING
#ifndef DOUBLE_BUFFERING
static int attributeList[] = {
    GLX_RGBA,
    GLX_RED_SIZE,   8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE,  8,
    None };
#else // DOUBLE_BUFFERING
static int attributeList[] = {
	GLX_RGBA, 
	GLX_DOUBLEBUFFER,
	GLX_RED_SIZE,   8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE,  8,
	None };
#endif // DOUBLE_BUFFERING
	

// Get tick in microseconds
time_t getTickUs() {
    time_t          TickUs = 0;
    int             Res;
    struct timespec ts_now;

    // Get current time, using the MONOTONIC clock
    Res = clock_gettime(CLOCK_MONOTONIC, &ts_now);
    if (0 == Res) {
        TickUs = (ts_now.tv_sec * 1000000) + (ts_now.tv_nsec / 1000);
    }

    return TickUs;
}

// Calculate difference between consecutive ticks
time_t diff(const time_t aStartTime, const time_t aEndTime) {
    time_t deltaTime;
    if (aStartTime <= aEndTime) {
        // 0 --- t1 --- t2 --- max
        deltaTime = aEndTime - aStartTime;
    } else {
        // 0 --- t2 --- t1 --- max
        deltaTime = (time_t)((-1 - aStartTime) + 1 + aEndTime);
    }

    return deltaTime;
}


static Bool WaitForNotify(Display *d, XEvent *e, char *arg) {
    return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

int main(int argc, char **argv) {
    Display *dpy;
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    Window win;
    GLXContext cx;
    XEvent event;
    float r = 1.0f;
    float rd = 0.001f;
    float xoffset = -0.7f;
    float xd = 0.002f;
    time_t lastUs;
    time_t previousUs;
    time_t currentUs;
    time_t frameUs;
    float  coef;
    time_t diffUs;
    time_t worstUs;
    unsigned long nbFrames;

    int Policy = SCHED_FIFO;
    struct sched_param Param;
    // Passe le thread en priorité FIFO maximale
    Param.__sched_priority = 99;
    pthread_setschedparam (pthread_self(), Policy, &Param);


    /* get a connection */
    dpy = XOpenDisplay(0);

    /* get an appropriate visual */
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeList);

    /* create a GLX context */
    cx = glXCreateContext(dpy, vi, 0, GL_TRUE);

    /* create a color map */
    cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
			   vi->visual, AllocNone);

    /* create a window */
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 1920, 1080,
                        0, vi->depth, InputOutput, vi->visual,
                        CWBorderPixel|CWColormap|CWEventMask, &swa);
    XMapWindow(dpy, win);
    XIfEvent(dpy, &event, WaitForNotify, (char*)win);

    /* connect the context to the window */
    glXMakeCurrent(dpy, win, cx);

    lastUs = getTickUs();
    previousUs = lastUs;
    nbFrames = 0;
    worstUs = 0;
    do
    {
        // Calcul FPS
        nbFrames++;
        currentUs = getTickUs();
        frameUs = diff(previousUs, currentUs);
        if (worstUs < frameUs)
        {
            worstUs = frameUs;
        }
        diffUs = diff(lastUs, currentUs);
        if (1000000 <= diffUs)
        {
            if (worstUs > 22000) // Si > 22ms au lieu de 16.67ms
            {
                time_t t = time(0);
                char buffer[9] = {0};
                strftime(buffer, 9, "%H:%M:%S", localtime(&t));

                printf("%s: %lu frames (%ffps) worst: %fms\n", buffer, nbFrames, (1000000*nbFrames)/(float)diffUs, worstUs/1000.0f);
                currentUs = getTickUs(); // pour ne pas tenir compte du temps d'affichage de la trace
            }
            lastUs = currentUs;
            nbFrames = 0;
            worstUs = 0;
        }
        previousUs = currentUs;

        // Coefficient multiplicateur normalement à "1" si framerate exactement à 60fps.
        coef = frameUs / 16667.0;

        // Déplacements et changements de couleur
        r -= (rd * coef);
        if (r < 0.0f)
        {
            r = 0.0f;
            rd = -rd;
        }
        else if (r > 1.0f)
        {
            r = 1.0f;
            rd = -rd;
        }
        xoffset += (xd * coef);
        if (xoffset < -0.7f)
        {
            xoffset = -0.7f;
            xd = -xd;
        }
        else if (xoffset > 0.7f)
        {
            xoffset = 0.7f;
            xd = -xd;
        }

        /* clear the buffer */
        glClearColor(r,1,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        /* draw a rectangle polygon */
        glBegin(GL_POLYGON);
            glVertex2f(-0.3 + xoffset, -0.5);
            glVertex2f(-0.3 + xoffset, -0.1);
            glVertex2f(0.3 + xoffset, -0.1);
            glVertex2f(0.3 + xoffset, -0.5);
        glEnd();

        /* flush GL buffers */
        glFlush();

#ifdef DOUBLE_BUFFERING
        /* swap GL buffers */
        glXSwapBuffers(dpy,win);
#endif

    } while (1);

    return 0;
}

