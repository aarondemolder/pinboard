//adapted from libfreenect's kinect OpenGL demo

#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>

#include <libfreenect.hpp>

#include "arduino-serial-lib.c"
#include "arduino-serial-lib.h"

#include <pinboard.h>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//set our serial connection to the arduino, with com port and baud rate
int serialPort = serialport_init("/dev/ttyACM0", 9600);

//depth positions for the servo connected to the arduino is setup in cases
//each case value sent to the arduino sets a varying LED brightness and Servo Angle.
//we could also instead send the actual depth value to the arduino for a more precise position of the servo.
void sendDepth(int depthVal)
{
    const char* data;

    if (depthVal < 800)
    {
        data = "1";
    }
    else if ((depthVal > 800) && (depthVal < 1200))
    {
        data = "2";
    }
    else if ((depthVal > 1200) && (depthVal < 1600))
    {
        data = "3";
    }
    else if ((depthVal > 1600) && (depthVal < 2000))
    {
        data = "4";
    }
    else if (depthVal > 2000)
    {
        data = "5";
    }
    else
    {
        data = "0";
    }

    //send data to serial
    serialport_write(serialPort, data);
}

//initialise the serial connection to wait for the arduino to reset before sending any data
void initSerial()
{
    const char* mes = "1";

    sleep(2);

    serialport_write(serialPort, mes);

}

//draw our 3D preview from the kinect view
void DrawGLScene()
{
    static std::vector<uint8_t> rgb(640*480*3);
    static std::vector<uint16_t> depth(640*480);

    device->getRGB(rgb);
    device->getDepth(depth);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPointSize(1.0f);

    glBegin(GL_POINTS);

    if (!color) glColor3ub(255, 255, 255);
    for (int i = 0; i < 480*640; ++i)
    {
        if (color)
            glColor3ub( rgb[3*i+0],    // R
                        rgb[3*i+1],    // G
                        rgb[3*i+2] );  // B

        float f = 595.f;
        if (depth[i] != 0)
        {
            // Convert from image plane coordinates to world coordinates
            glVertex3f( (i%640 - (640-1)/2.f) * depth[i] / f,  // X = (x - cx) * d / fx
                        (i/640 - (480-1)/2.f) * depth[i] / f,  // Y = (y - cy) * d / fy
                        depth[i] );                            // Z = d
        }

    }

    glEnd();

    //if our depth sending is toggled
    if (printDepth)
    {
        //wait for new frame
        if (device->m_new_depth_frame == true)
        {
            //print and send data from the bottom middle pixel
            std::cout<< "Pixel Depth: "<< (depth[(640*480)-320]) << "\n";
            sendDepth(depth[(640*480)-320]);
        }
    }

    // Place the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(zoom, zoom, 1);
    gluLookAt( -7*anglex, -7*angley, -1000.0,
                     0.0,       0.0,  2000.0,
                     0.0,      -1.0,     0.0 );

    glutSwapBuffers();

}


void keyPressed(unsigned char key, int x, int y)
{
    switch (key)
    {
        case  'C':
        case  'c':
            color = !color; //toggle colour in OpenGL view
        break;

    case  'D':
    case  'd':
        printDepth = !printDepth; //toggle depth print
    break;


        case  'Q':
        case  'q':
        case 0x1B:  // ESC
            glutDestroyWindow(window);
            device->stopDepth();
            device->stopVideo();
            exit(0);

    }
}


void mouseMoved(int x, int y)
{
    if (mx >= 0 && my >= 0)
    {
        anglex += x - mx;
        angley += y - my;
    }

    mx = x;
    my = y;
}


void mouseButtonPressed(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        switch (button)
        {
            case GLUT_LEFT_BUTTON:
                mx = x;
                my = y;
                break;

            case 3:
                zoom *= 1.2f;
                break;

            case 4:
                zoom /= 1.2f;
                break;
        }
    }
    else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
    {
        mx = -1;
        my = -1;
    }
}


void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)width / height, 900.0, 11000.0);

    glMatrixMode(GL_MODELVIEW);
}


void idleGLScene()
{
    glutPostRedisplay();
}

void printInfo()
{
    std::cout << "\nAvailable Controls:"              << std::endl;
    std::cout << "==================="                << std::endl;
    std::cout << "Rotate                :   Mouse Left Button" << std::endl;
    std::cout << "Zoom                  :   Mouse Wheel"       << std::endl;
    std::cout << "Toggle Color          :   C"                 << std::endl;
    std::cout << "Toggle Arduino Depth  :   D"                 << std::endl;
    std::cout << "Quit                  :   Q or Esc\n"        << std::endl;
}


int main(int argc, char **argv)
{
    device = &freenect.createDevice<MyFreenectDevice>(0);
    device->startVideo();
    device->startDepth();

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(0, 0);

    window = glutCreateWindow("deepnect");
    glClearColor(0.45f, 0.45f, 0.45f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0, 1.0, 900.0, 11000.0);

    glutDisplayFunc(&DrawGLScene);
    glutIdleFunc(&idleGLScene);
    glutReshapeFunc(&resizeGLScene);
    glutKeyboardFunc(&keyPressed);
    glutMotionFunc(&mouseMoved);
    glutMouseFunc(&mouseButtonPressed);

    printInfo();

    initSerial();

    glutMainLoop();

    return 0;
}
