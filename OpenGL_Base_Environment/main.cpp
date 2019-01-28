
#include "GLTools.h"	
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/*
 GLMatrixStack 变化管线使用矩阵堆栈
 
 GLMatrixStack 构造函数允许指定堆栈的最大深度、默认的堆栈深度为64.这个矩阵堆在初始化时已经在堆栈中包含了单位矩阵。
 GLMatrixStack::GLMatrixStack(int iStackDepth = 64);
 
 //通过调用顶部载入这个单位矩阵
 void GLMatrixStack::LoadIndentiy(void);
 
 //在堆栈顶部载入任何矩阵
 void GLMatrixStack::LoadMatrix(const M3DMatrix44f m);
 */
// 各种需要的类
GLShaderManager		shaderManager;
/** 模型视图矩阵堆栈 */
GLMatrixStack		modelViewMatrix;
/** 投影视图矩阵堆栈 */
GLMatrixStack		projectionMatrix;
/** 观察者位置 */
GLFrame				cameraFrame;
/** 世界坐标位置 */
GLFrame             objectFrame;
//投影矩阵
GLFrustum			viewFrustum;

/** 三角形批次类 */
GLTriangleBatch     CC_Triangle;

//容器类（7种不同的图元对应7种容器对象）
/** 球 */
GLTriangleBatch     sphereBatch;
/** 环 */
GLTriangleBatch     torusBatch;
/** 圆柱 */
GLTriangleBatch     cylinderBatch;
/** 锥 */
GLTriangleBatch     coneBatch;
/** 磁盘 */
GLTriangleBatch     diskBatch;


/** 几何变换的管道，用来管理投影视图矩阵堆栈和模型视图矩阵堆栈的 */
GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };


/** 跟踪效果步骤 */
int nStep = 0;



// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    
    shaderManager.InitializeStockShaders();
    
    //开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
    //将观察者的坐标Z值放屏幕里移动15个单位
    cameraFrame.MoveForward(-15.0f);
    
    //球
    /**
        gltMakeSphere(GLTriangleBatch &sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
        参数一:三角形批次类对象
        参数二:球体半径
         参数三:从球体底部到顶部的三角形带的数量,其实球体是一圈一圈的三角形带组成的.
         参数四:围绕球体一圈的三角形对数
     */
    gltMakeSphere(sphereBatch, 3.0f, 10, 20);
    
    //甜甜圈🍩
    /** gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor)
     参数一:三角形批次类
     参数二:甜甜圈中心到外圈的半径
     参数三:甜甜圈中心到内圈的半径
     参数四:沿着主半径的三角形的对数
     参数五:颜色内部较小半径的三角形对数.!!!尽量!!!numMajor = 2*numMinor
        */
    gltMakeTorus(torusBatch, 3.0f, 0.75f, 24, 12);
    
    //圆柱
    /** gltMakeCylinder(GLTriangleBatch &cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks)
     参数一:三角形批次类
     参数二:底部半径
     参数三:顶部半径
     参数四:圆柱的长度
     参数五:三角形对的数量
     参数六:底部堆叠到顶部圆弧的三角形对数
     */
    gltMakeCylinder(cylinderBatch, 2.0f, 2.0f, 3.0f, 15.0f, 2);
    
    //圆锥
    gltMakeCylinder(coneBatch, 2.0f, 0.0f, 3.0f, 13.0f, 2);
    
    //磁盘
    /** gltMakeDisk(GLTriangleBatch &diskBatch, GLfloat innerRadius, GLfloat outerRadius, GLint nSlices, GLint nStacks)
     参数一:三角形批次类
     参数二:内圆半径
     参数三:外圆半径
     参数四:圆围绕Z轴的三角形对数
     参数五:Y圆盘外圈到内圈的三角形对数
     */
    gltMakeDisk(diskBatch, 1.5f, 3.0f, 13, 3);
    
}



void DrawWireFramedBatch(GLTriangleBatch* pBatch)
{
    //1.平面主色去绘制图
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    //2.黑色轮廓
    glPolygonOffset(-1.0f, -1.0f);
    
    //3.开启混合抗锯齿
    glEnable(GL_LINE_SMOOTH);
    
    //4.开启混合
    glEnable(GL_BLEND);
    
    //5.颜色混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //6.
    glEnable(GL_POLYGON_OFFSET_LINE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glLineWidth(2.5f);
    
    //使用平面着色器绘制黑色边框线条
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();
    
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //模型视图压栈
    modelViewMatrix.PushMatrix();
    
    //获取观察者矩阵
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    //模式视图堆栈的顶部矩阵 与 mCamera 矩阵相乘,存储到modelViewMatrix
    modelViewMatrix.MultMatrix(mCamera);
    
    //
    M3DMatrix44f mObjectFrame;
    objectFrame.GetMatrix(mObjectFrame);
    modelViewMatrix.MultMatrix(objectFrame);
    
    //判断
    switch (nStep) {
        case 0:
            DrawWireFramedBatch(&sphereBatch);
            break;
        case 1:
            DrawWireFramedBatch(&torusBatch);
            break;
        case 2:
            DrawWireFramedBatch(&cylinderBatch);
            break;
        case 3:
            DrawWireFramedBatch(&coneBatch);
            break;
        case 4:
            DrawWireFramedBatch(&diskBatch);
            break;
        default:
            break;
    }
    
    modelViewMatrix.PopMatrix();
    glutSwapBuffers();

}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        //移动世界坐标系,而不是移动物体
        //将坐标系在X方向移动-5.0
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, .0f, .0f);
    }

    if (key == GLUT_KEY_DOWN) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, .0f, .0f);
    }
    
    if (key == GLUT_KEY_LEFT) {
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), .0f, 1.0f, .0f);
    }

    if (key == GLUT_KEY_RIGHT) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), .0f, 1.0f, .0f);
    }
    
    glutPostRedisplay();

}




//根据空格次数。切换不同的“窗口名称”
//key:敲击的键位
//x,y: 光标的位置
void KeyPressFunc(unsigned char key, int x, int y)
{
    //key == 32 32的ASC2码，是空格。比如A：65; B = 66
    if(key == 32)
    {
        nStep++;
        
        if(nStep > 4)
            nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("Sphere 球");
            break;
        case 1:
            glutSetWindowTitle("Torus 环");
            break;
        case 2:
            glutSetWindowTitle("Cylinder 圆柱");
            break;
        case 3:
            glutSetWindowTitle("Cone 圆锥");
            break;
        case 4:
            glutSetWindowTitle("Disk 光盘");
            break;
    }
    
    glutPostRedisplay();
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
    
    //o透视投影
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 500.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //对模型视图加载一个单元矩阵
    modelViewMatrix.LoadIdentity();
    

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
    glutCreateWindow("Sphere 球");
    
    //注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    
    //点击空格时，调用的函数
    //注册一个键盘ASC2 码的键位事件的回调函数，当有键盘按下时，则执行回调函数
    glutKeyboardFunc(KeyPressFunc);
    
    //特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    
    //注册鼠标点击事件：
    glutMouseFunc(MouseKey);
    
    
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
