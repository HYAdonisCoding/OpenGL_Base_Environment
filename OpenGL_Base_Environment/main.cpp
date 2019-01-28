
#include "GLTools.h"	
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

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
/** 视景体-投影矩阵通过它来设置 */
GLFrustum		    viewFrustum;

/** 三角形批次类 */
GLTriangleBatch     torusBatch;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };



// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);
    
    shaderManager.InitializeStockShaders();
    
    gltMakeSphere(torusBatch, .4f, 10, 20);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //建立一个基于世界变化的动画
    static CStopWatch rotTimer;
    
    //当前时间 * 60s
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
    
    //矩阵变量
    M3DMatrix44f mTranlate, mRotate, mModelView, mMdoelViewProjection;
    
    //将圆球向Z周副方向移动2.5个单位长度
    m3dTranslationMatrix44(mTranlate, .0f, .0f, -2.5f);
    
    //旋转
    m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), .0f, 1.0f, .0f);
    
    //将平移和旋转矩阵进行叉乘,产生一个新的矩阵
    m3dMatrixMultiply44(mModelView, mTranlate, mRotate);
    
    //模型视图矩阵 和 投影矩阵
    //将投影矩阵 与 模型视图矩阵进行叉乘 ,将变化最终结果通过矩阵叉乘的方式应用到mModelViewProjection中来
    m3dMatrixMultiply44(mMdoelViewProjection, viewFrustum.GetProjectionMatrix(), mModelView);
    
    GLfloat vBlack[] = {.0f, .0f, .0f, 1.0f};
    
    //平面着色器来渲染图像
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mMdoelViewProjection, vBlack);
    
    //开始绘图
    torusBatch.Draw();
    
    
    glutSwapBuffers();
    glutPostRedisplay();
}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    
    glutPostRedisplay();

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
