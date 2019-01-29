#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#define NUM_SPHERE 50
GLFrame sphere[NUM_SPHERE];

/** 着色器管理器 */
GLShaderManager		shaderManager;
/** 模型视图矩阵 */
GLMatrixStack       modelViewMatrix;
/** 投影矩阵 */
GLMatrixStack       projectionMatrix;
/** 视景体-投影矩阵通过它来设置 */
GLFrustum		    viewFrustum;
/** 几何视图变换管道 */
GLGeometryTransform transformPipelint;

/** 三角形批次类 */
GLTriangleBatch     torusBatch;
/** 底板批次类 */
GLBatch             floorBatch;
/** 球批次类 */
GLTriangleBatch     sphereBatch;
/** 角色帧 照相机角色帧 */
GLFrame             cameraFrame;


// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    shaderManager.InitializeStockShaders();

    glEnable(GL_DEPTH_TEST);
    
    
    //线框渲染
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    //绘制甜甜圈
    gltMakeTorus(torusBatch, .4f, .15f, 30, 30);
    
    //绘制球体
    gltMakeSphere(sphereBatch, .1f, 26, 13);
    
    //绘制底板
    floorBatch.Begin(GL_LINES, 324);
    //底板多的宽度
    for (GLfloat x = -20.0f; x <= 20.0f; x += 0.5f) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
    }
    
    floorBatch.End();
    
    //随机放置球体
    for (int i = 0; i < NUM_SPHERE; i++) {
        //Y轴不变0,x,z随机
        GLfloat x = (GLfloat)(((rand() % 400)-200) * .1f);
        GLfloat y = (GLfloat)(((rand() % 5) - 5) * .1f);
        GLfloat z = (GLfloat)(((rand() % 400)-200) * .1f);
        
//        sphere[i].SetOrigin(x, 0.0f, z);
        sphere[i].SetOrigin(x, -y, z);
    }
}



void DrawWireFramedBatch(GLTriangleBatch* pBatch)
{
    
    
    //恢复设置
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    
}



// 召唤场景
void RenderScene(void)
{
    
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    static GLfloat vTrousColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat vSphereColor[] = { .0f, .0f, 1.0f, 1.0f };
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //建立一个基于世界变化的动画
    static CStopWatch rotTime;
    
    //当前时间 * 60s
    float yRot = rotTime.GetElapsedSeconds() * 60.0f;
    
//    modelViewMatrix.PushMatrix();
    
    //设置观察者矩阵
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
    //添加光源
    M3DVector4f vLightPos = {.0f, 10.0f, 5.0f, 1.0f};
    M3DVector4f vLightEyePos;
    
    //将照相机的mCamera 与光源矩阵vLightPos 相乘得到 vLightEyePos
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
    
    //绘制底板
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vFloorColor);
    floorBatch.Draw();
    
    //绘制悬浮随机小球体 sphereBatch
    for (int i = 0; i < NUM_SPHERE; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(sphere[i]);
        
        //shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vSphereColor);
        //默认光源着色器
        /** 参数1: GLT_SHADER_POINT_LIGHT_DIFF
            参数2: 模型视图矩阵
            参数3:投影矩阵
            参数4"光源位置
            参数5:漫反射颜色
         */
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipelint.GetModelViewMatrix(), transformPipelint.GetProjectionMatrix(), vLightEyePos, vSphereColor);
        sphereBatch.Draw();
        modelViewMatrix.PopMatrix();
    }
    
    //向屏幕的-Z方向移动2.5个单位
    modelViewMatrix.Translate(.0f, .0f, -2.5f);
    
    //将结果压栈
    modelViewMatrix.PushMatrix();
    
    //旋转
    modelViewMatrix.Rotate(yRot, .0f, 1.0f, .0f);
    
    //绘制🍩
    //shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vTrousColor);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipelint.GetModelViewMatrix(), transformPipelint.GetProjectionMatrix(), vLightEyePos, vTrousColor);
    torusBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    //绘制公转的球体
    modelViewMatrix.Rotate(yRot * -2.0f, .0f, 1.0f, .0f);
    modelViewMatrix.Translate(.8f, .0f, .0f);
    
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vSphereColor);
    sphereBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PopMatrix();
    
    
    glutSwapBuffers();
    glutPostRedisplay();
}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    float linar = .1f;
    float angular = float(m3dDegToRad(5.0f));
    
    if (key == GLUT_KEY_UP) {
        cameraFrame.MoveForward(linar);
    }
    
    if (key == GLUT_KEY_DOWN) {
        cameraFrame.MoveForward(-linar);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, .0f, 1.0f, .0f);
    }
    
    if (key == GLUT_KEY_LEFT) {
        cameraFrame.RotateWorld(angular, .0f, 1.0f, .0f);
    }
    

}




//根据空格次数。切换不同的“窗口名称”
//key:敲击的键位
//x,y: 光标的位置
void KeyPressFunc(unsigned char key, int x, int y)
{
   
}

//鼠标点击事件
void MouseKey(int button,int state, int x,int y)
{
    
}

// 窗口已更改大小，或刚刚创建。无论哪种情况，我们都需要
// 使用窗口维度设置视口和投影矩阵.
void ChangeSize(int w, int h)
{
    if (h == 0) {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    
    //透视投影
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
    
    //获取投影矩阵到
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //变换管道中来使用
    transformPipelint.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
}


int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    
    //申请一个颜色缓存区、深度缓存区、双缓存区、模板缓存区
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    
    //设置window 的尺寸
    glutInitWindowSize(800, 600);
    
    //创建window的名称
    glutCreateWindow("SphereWorld");
    
    //注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    
    //点击空格时，调用的函数
    //注册一个键盘ASC2 码的键位事件的回调函数，当有键盘按下时，则执行回调函数
    glutKeyboardFunc(KeyPressFunc);
    
    //特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    
    //注册鼠标点击事件：
    //glutMouseFunc(MouseKey);
    
    
    //显示函数
    glutDisplayFunc(RenderScene);
    
    //判断一下是否能初始化glew库，确保项目能正常使用OpenGL 框架
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    //绘制
    SetupRC();
    
    //runloop运行循环
    glutMainLoop();
    return 0;
}
