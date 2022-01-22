#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <windows.h>

#define PI 3.14159265357
//GLAD--------
#include <glad/glad.h>

//GLFW----------
#include <GLFW/glfw3.h>

//OpenCV---------open the file/texture
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//GLM-----------math operation
#include<glm/glm/glm.hpp>
#include<glm/glm/gtc/matrix_transform.hpp>
#include<glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/vec3.hpp> // glm::vec3
#include <glm/glm/vec4.hpp> // glm::vec4
#include <glm/glm/mat4x4.hpp> // glm::mat4

//ffmpeg-------------audio lib
#define __STDC_CONSTANT_MACROS		//declare ffmpeg

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
    #include "libavfilter/avfilter.h"
    #include "libavutil/avutil.h"
    #include "libpostproc/postprocess.h"
    #include "libswresample/swresample.h"
    #include "libavutil/imgutils.h"		
};

//own head program-----------
#include "shader.h"
#include "camera.h"

// video size setting
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

//---------------- functions ---------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


//----------- shaders: in program --------------
//---------------- camera view ---------------------
//----------- 3.0f --> outside to inside || 0.0f ---> inside to outside --------------
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat lastX = SCR_WIDTH / 2.0;
GLfloat lastY = SCR_HEIGHT / 2.0;
GLfloat aspect = 45.0f;

bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//YUV file
const int pixel_w = 1280, pixel_h = 720;
unsigned char buffer[pixel_w * pixel_h * 3 / 2];

//-----------------initialize---------------------

//-------------  screen ----------

int main(int argc, char* argv[])
{   

    //----------------GLFW init-----------------
    glfwInit();//init glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//glfw's main version is 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//glfw's subversion is 3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//

    //--------------- window -------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);    // generating a window
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();// clean memory
        return -1;
    }
    glfwMakeContextCurrent(window);// Set current window as main context
    //-----------GLAD init---------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //glad init glad(glad is the pointer for managing OpenGL)
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //--------------the callback function--------------------
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //--------------viewport（SDL rect）----------------
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);        // get buffer size -- height and width
    glViewport(0, 0, width, height);                        // Coordinate mapping (full screen here)
    // Build and compile our shader program
    Shader ourShader("shader.vs", "shader.frag");
    
    // calculating points of sphere
    //------------------------------------vertexs-------------------------------
    //---------------calc sphere model's vertexs----------------
    //--------radius--------
    GLfloat r = 0.6f;

    // parts size
    GLuint longitude = 20;
    GLuint latitude = 50;

    // offset of each loop
    GLfloat Voffset = 180.0f / longitude;
    GLfloat Hoffset = 360.0f / latitude;
    GLfloat TVoffset = 1.0f / longitude;
    GLfloat THoffset = 1.0f / latitude;

    // intersaction angle for x axis and y axis
    GLfloat a, b, x, y, z, t_x, t_y;

    //索引
    GLuint vert_index = 0;
    GLuint num = (longitude - 1) * latitude;
    GLuint k = 1;

    //vertex array p
    GLfloat vertices[10000];

    // first vertex
    //----- vertex -----

    //改，1和最后颠倒
    glm::vec3 point = glm::vec3(0.0f, -r, 0.0f);
    memcpy(vertices, glm::value_ptr(point), 3 * sizeof(GLfloat));
    //----- texture -----
    glm::vec2 tex = glm::vec2(0.5f, 1.0f);
    memcpy(vertices + 3, glm::value_ptr(tex), 2 * sizeof(GLfloat));

    vert_index++;
    
    for (GLuint i = 1; i < longitude; i++)
    {
        a = glm::radians(i * Voffset);
        y = - r * cos(a);
        for (GLuint j = 1; j <= latitude; j++)
        {
            b = glm::radians(j * Hoffset);
            z = r * sin(a) * cos(b);
            x = r * sin(a) * sin(b);
            point = glm::vec3(x, y, z);
            memcpy(vertices + 5 * vert_index, glm::value_ptr(point), 3 * sizeof(GLfloat));

            t_x = (j - 1)* THoffset;
            t_y = 1 - i * TVoffset;
            tex = glm::vec2(t_x, t_y);
            memcpy(vertices + 5 * vert_index + 3, glm::value_ptr(tex), 2 * sizeof(GLfloat));

            vert_index++;
        }
    }

    // last vertex
    point = glm::vec3(0.0f, r, 0.0f);
    memcpy(vertices + 5 * vert_index, glm::value_ptr(point), 3 * sizeof(GLfloat));
    tex = glm::vec2(0.5f, 0.0f);
    memcpy(vertices + 5 * vert_index + 3, glm::value_ptr(tex), 2 * sizeof(GLfloat));
    
    //-------------------- end vertex calculation------------------------
    //-------------------- define index -------------------------
    GLuint indices[8000];

    //first layer index
    for (GLuint j = 0; j < latitude; j++)
    {
        indices[j * 3] = j + 1;
        indices[j * 3 + 1] = 0;
        indices[j * 3 + 2] = (j + 2)%50;
    }
    // latitude*3 numbers

    GLuint indi_index = latitude * 3;

    //middle index -- draw quadrangles
    for (GLuint i = 0; i < (longitude - 2); i++)
    {
        for (GLuint j = 0; j < latitude; j++)
        {
            indices[indi_index] = (i + 1) * latitude + j % 50 + 1;             indi_index++;
            indices[indi_index] = (i ) * latitude + j % 50 + 1;       indi_index++;
            indices[indi_index] = (i + 1) *latitude + (j + 1) % 50 + 1;         indi_index++;
            indices[indi_index] = (i) * latitude + j % 50 + 1;       indi_index++;
            indices[indi_index] = (i) * latitude + (j + 1) % 50 + 1;   indi_index++;
            indices[indi_index] = (i + 1) * latitude + (j + 1) % 50 + 1;         indi_index++;
        }   
    }

    for (GLuint j = 0; j  < latitude; j++)
    {
        indices[indi_index] = (longitude - 1) * latitude + 1;   indi_index++;
        indices[indi_index] = (longitude - 2) * latitude + 1 + j;   indi_index++;
        indices[indi_index] = (longitude - 2) * latitude + 1 + (1 + j) % 50;   indi_index++;
    }
    //---------------------end index--------------------------

   
    //---------------position vector (center of window) --------------------
    glm::vec3 spherePositions = glm::vec3(0.0f,  0.0f,  0.0f);
    // buffer objects
    GLuint VBO1, VAO1, EBO1;             //
    glGenBuffers(1, &VBO1);          // VBO Vertex Buffer Objects generating
    // aim: store vertex datas & settings
    glGenVertexArrays(1, &VAO1);    // generating VAO vertex objects
    glGenBuffers(1, &EBO1);
    // index buffer object

    glBindVertexArray(VAO1);         // bound VAO1
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);    // bound the VBO buffer object to vertex buffer object（GL_ARRAY_BUFFER）
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //finishing storing vertex data
    
    // bound index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // define the pointer of vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);               // linking vertexes properties

    // texture data of vertex
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // set layout of color property as 1
    glBindVertexArray(0);           // unbound VAO

    // texture operating--------------------------------------------
    //IplImage* image;// = cvLoadImage("3.png", 1);		// input image
    // for videos, should read frames 

    // generating different texture for Y, U, and V stream
    GLuint texture, texture2, texture3;                         //生成纹理
    glGenTextures(1, &texture);
    glGenTextures(1, &texture2);
    glGenTextures(1, &texture3);

    glBindTexture(GL_TEXTURE_2D, texture);  //bound texture 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, texture2);  //bound textrure 2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, texture3);  //bound textrure 3
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);

     //--------------------------------------------------------------------
    //-----------------------------video processing------------------------------
    //------------------------------------------------------------------

    AVFormatContext* pFormatCtx;	// format contex
    int				i, videoindex;	// index
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);		//coding context
    AVCodec* pCodec;				// encoder selecting
    AVFrame* pFrame, * pFrameYUV;	// fram data stream
    uint8_t* out_buffer;			// out buffer
    AVPacket* packet;				// packet
    int y_size;
    int ret, got_picture;
    struct SwsContext* img_convert_ctx;
    // input path
    char filepath[] = "14.mp4";
    //FILE* fp;
    int frame_cnt;
    pFormatCtx = avformat_alloc_context();		

    /*       -------------- read a video steam------------------       */
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < (pFormatCtx->nb_streams); i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    /*       --------------decoding video------------------       */

    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);		//read coding context
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);		// matching encoder
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {		// open encoder
        printf("Could not open codec.\n");
        return -1;
    }
    pFrame = av_frame_alloc();								 
    pFrameYUV = av_frame_alloc();							 

    // data size of a frame
    out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    packet = (AVPacket*)av_malloc(sizeof(AVPacket));

    //Output Info
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("-------------------------------------------------\n");
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, 4, NULL, NULL, NULL);//4->SWS_BICUBIC

    //FILE* fp_YUV;

    //fopen_s(&fp_YUV, "YUVversion.yuv", "wb+");

    frame_cnt = 0;

    //--------------------------------------------------------------------
    //--------------------------------------------------------------------
    //--------------------------   LOOP   --------------------------------
    
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        
        if (glfwWindowShouldClose(window))   break;

        // ------prepare for each refreshing of window-------
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //check
        glfwPollEvents();
        do_movement();     
        //clear
        glClearColor(0.5f, 0.1f, 0.8f, 1.0);    // settings(color of clearing the window buffer)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // clear buffer

        if (packet->stream_index == videoindex) {
            ret = avcodec_send_packet(pCodecCtx, packet);
            got_picture = avcodec_receive_frame(pCodecCtx, pFrame);

            if (ret < 0) {
                printf("Decode Error.\n");
                return -1;
            }

            //pframe  raw YUV file. pframeYUV, file scaled by sws_scale
            if (!got_picture) {		//  got_picture == 0 --> success
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                    pFrameYUV->data, pFrameYUV->linesize);
                printf("Decoded frame index: %d\n", frame_cnt);

                //Y
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w, pixel_h, 0, GL_RED, GL_UNSIGNED_BYTE, pFrameYUV->data[0]);
                glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture"), 0);//boudn uniform value
                glGenerateMipmap(GL_TEXTURE_2D);
                //U
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture2);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w / 2, pixel_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, pFrameYUV->data[1]);
                glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);//boudn uniform value
                glGenerateMipmap(GL_TEXTURE_2D);
                //V
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texture3);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w / 2, pixel_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, pFrameYUV->data[2]);
                glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture3"), 2);//boudn uniform value
                glGenerateMipmap(GL_TEXTURE_2D);

                frame_cnt++;

            }
        }
        av_packet_unref(packet);

        ourShader.Use();               //shadering

        // observing matrix
        glm::mat4 view;
        // moving to opposite direction as human feeling
        view = camera.GetViewMatrix();

        // projection matrix
        glm::mat4 projection;
        projection = glm::perspective(camera.Zoom, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
        //fov = 45 degree//
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        // Pass them to the shaders
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAO1);

        //-------------------MVP change--------------------
        glm::mat4 model;
        model = glm::translate(model, spherePositions);
        GLfloat angle = glm::radians(20.0f);/* 2 * (GLfloat)glfwGetTime();*/

        //改： --------model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
        model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //-------------------- draw ---------------------
        glDrawElements(GL_TRIANGLES, (indi_index - 1), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glfwSwapBuffers(window);//swap color buffer for texture drawing
        //av_free_packet(packet);
    }

    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &EBO1);

    glfwTerminate();

    return 0;
}


// interaction functions of windows
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

}       //normalize         cross 

void do_movement()
{
    // camera control
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)             //mouse call back 
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

//zooming
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
