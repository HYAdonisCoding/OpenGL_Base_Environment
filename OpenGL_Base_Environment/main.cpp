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

/** 随机球体 */
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

/** 花托批次类 */
GLTriangleBatch     torusBatch;
/** 底板批次类 */
GLBatch             floorBatch;
/** 球批次类 */
GLTriangleBatch     sphereBatch;
/** 角色帧 照相机角色帧 */
GLFrame             cameraFrame;

/** 添加纹理 纹理标记数组 */
GLuint uiTextures[3];

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int iWidth, iHeight, iComponents;
    GLenum eFormat;
    
    //读取纹理数据
    pBits = gltReadTGABits(szFileName, &iWidth, &iHeight, &iComponents, &eFormat);
    if (pBits == NULL) {
        printf("读取纹理数据失败!\n");
        return false;
    }
    
    //2.设置纹理参数
    /**参数
     1.纹理纬度
     2.为S\T坐标设置模式
     3.环绕模式*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    /**参数
     1.纹理纬度
     2.线性过滤
     3.环绕模式*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    //3.精密包装像素数据
    /** 参数
     1.GL_UNPACK_ALIGNMENT, 指定OpenGL如何从数据缓存区中解包s图像数据
     2.针对GL_UNPACK_ALIGNMENT设置的值*/
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /** 载入纹理 参数
     1.纹理纬度
     2.mip贴图层次
     3.纹理单元存储的颜色成分(从读取像素图是获得) - 将内部参数iComponents改为了通用压缩纹理格式GL_COMPRESSED_RGB
     4.加载纹理的宽
     5.加载纹理的高
     6.加载纹理的深度
     7.像素数据的数据类型(每个颜色分量s都是一个8位无符号整数)
     8.指向纹理图像数据的指针
     */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
    
    //使用完毕释放pBits
    free(pBits);
    
    /** 只有minFilter 为以下四种模式,才能生成mip贴图
     GL_LINEAR_MIPMAP_LINEAR 和GL_NEAREST_MIPMAP_LINEAR 过滤器在Mip层之间执行了一些额外的插值，以消除他们之间的过滤痕迹。
     GL_LINEAR_MIPMAP_NEAREST常常用于对游戏进行加速，它使用了高质量的线性过滤器
     GL_NEAREST_MIPMAP_NEAREST具有非常好的性能，并且闪烁现象非常弱
     GL_LINEAR_MIPMAP_LINEAR 三线性Mip贴图。纹理过滤的黄金准则，具有最高的精度。
     */
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST) {
        //加载mip,纹理生成所有的mip层
        //参数:GL_TEXTURE_2D
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    
    return true;
}

// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    shaderManager.InitializeStockShaders();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    //线框渲染
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    //绘制甜甜圈
    gltMakeTorus(torusBatch, .4f, .15f, 30, 30);
    
    //绘制球体
    gltMakeSphere(sphereBatch, .1f, 26, 13);
    
    //往底板批处理中添加顶点数据
    GLfloat textSize = 10.0f;
    //绘制底板
    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
    floorBatch.MultiTexCoord2f(0, .0f, .0f);
    floorBatch.Vertex3f(-20.0f, -.41f, 20.0f);
    
    floorBatch.MultiTexCoord2f(0, textSize, .0f);
    floorBatch.Vertex3f(20.0f, -.41f, -20.0f);
    
    floorBatch.MultiTexCoord2f(0, textSize, textSize);
    floorBatch.Vertex3f(20.0f, -.41f, -20.0f);
    
    floorBatch.MultiTexCoord2f(0, .0f, textSize);
    floorBatch.Vertex3f(-20.0f, -.41f, -20.0f);
    floorBatch.End();
    
    //绑定3个纹理
    glGenTextures(3, uiTextures);
    
    /** 将TGA文件加载为2D纹理
     1.纹理文件名称
     2\3.需要缩小\放大的过滤器
     4.纹理坐标环绕模式
     */
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    //随机放置球体
    for (int i = 0; i < NUM_SPHERE; i++) {
        //Y轴不变0,x,z随机
        GLfloat x = (GLfloat)(((rand() % 400)-200) * .1f);
        GLfloat y = (GLfloat)(((rand() % 5) - 5) * .1f);
        GLfloat z = (GLfloat)(((rand() % 400)-200) * .1f);
        
        sphere[i].SetOrigin(x, 0.0f, z);
//        sphere[i].SetOrigin(x, -y, z);
    }
}

void ShutdownRC(void)
{
    glDeleteTextures(3, uiTextures);
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


void DrawSongAndDance(GLfloat yRot)
{
    static GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat vLightPos[] = {.0f, 3.0f, .0f, 1.0f};
    
    //添加光源 光源位置的全局坐标存储在vLightPos变量中，其中包含了光源位置x坐标、y坐标、z坐标和w坐标。我们必须保留w坐标为1.0。因为无法用一个3分量去乘以4*4矩阵
    M3DVector4f vLightTransformed;
    
    //相机矩阵
    M3DMatrix44f mCamera;
    //从modelViewMatrix 获取矩阵堆栈顶部的值
    modelViewMatrix.GetMatrix(mCamera);
    //将相机矩阵mCamera与光源矩阵vLightPos相乘获得vLightTransformed矩阵
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    
    //结果压栈
    modelViewMatrix.PushMatrix();
    //仿射变换, 平移
    modelViewMatrix.Translatev(vLightPos);
    
    /** 绘制(平面着色器) 参数
     1.GLT_SHADER_FLAT
     2.模型视图投影矩阵
     3.颜色数组
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vWhite);
    sphereBatch.Draw();
    
    //恢复矩阵
    modelViewMatrix.PopMatrix();
    
    //绘制悬浮球体 绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    
    //绘制50个蓝色悬浮球体,绘制一个压栈一个,绘制完成出栈一个
    for (int i = 0; i < NUM_SPHERE; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(sphere[i]);
        
        /** 绘制光源,修改着色器管理器 参数
         1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
         2.模型视图矩阵
         3.投影矩阵
         4.视点坐标系中的光源位置
         5.基本漫反射颜色
         6.颜色(使用纹理则不用颜色)
         */
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatrix.GetMatrix(),
                                     transformPipelint.GetProjectionMatrix(),
                                     vLightTransformed,
                                     vWhite,
                                     0);
        sphereBatch.Draw();
        
        modelViewMatrix.PopMatrix();
    }
    
    //绘制旋转甜甜圈
    //modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
    modelViewMatrix.Translate(.0f, .2f, -2.5f);
    
    //保存平移 (公转自转)
    modelViewMatrix.PushMatrix();
    
    //顶部矩阵旋转
    modelViewMatrix.Rotate(yRot, .0f, 1.0f, .0f);
    
    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    /** 绘制光源,修改着色器管理器 参数
     1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
     2.模型视图矩阵
     3.投影矩阵
     4.视点坐标系中的光源位置
     5.基本漫反射d颜色
     6.颜色(使用纹理则为0)
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipelint.GetProjectionMatrix(), vLightTransformed, vWhite, 0);
    
    //绘制
    torusBatch.Draw();
    
    //恢复矩阵
    modelViewMatrix.PopMatrix();
    
    //绘制公转球体
    modelViewMatrix.Rotate(yRot * -2.0f, .0f, 1.0f, .0f);
    modelViewMatrix.Translate(.8f, .0f, .0f);
    
    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    
    //绘制光源
    /** 绘制光源,修改着色器管理器 参数
     1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
     2.模型视图矩阵
     3.投影矩阵
     4.视点坐标系中的光源位置
     5.基本漫反射d颜色
     6.颜色(使用纹理则为0)
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipelint.GetProjectionMatrix(), vLightTransformed, vWhite, 0);
    sphereBatch.Draw();
    
    
}


// 召唤场景
void RenderScene(void)
{
    
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, .75f };
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //建立一个基于世界变化的动画
    static CStopWatch rotTime;
    
    //当前时间 * 60s
    float yRot = rotTime.GetElapsedSeconds() * 60.0f;
    
    //将当前的模型视图矩阵压入矩阵堆栈（单位矩阵）
    /*因为我们先绘制地面，而地面是不需要有任何变换的。所以在开始渲染时保证矩阵状态，
     然后在结束时使用相应的PopMatrix恢复它。这样就不必在每一次渲染时重载单位矩阵了。
     */
    modelViewMatrix.PushMatrix();
    
    //设置观察者矩阵
    M3DMatrix44f mCamera;
    //从camraFrame中获取一个4*4的矩阵；
    cameraFrame.GetCameraMatrix(mCamera);
    //将照相机矩阵压入模型视图堆栈中
    modelViewMatrix.MultMatrix(mCamera);
    
    //压栈
    modelViewMatrix.PushMatrix();
    
    //添加反光效果
    modelViewMatrix.Scale(1.0f, -1.0f, 1.0f);//翻转y轴
    modelViewMatrix.Translate(.0f, .8f, .0f);//缩小
    
    /** 定义前向和背向的多边形
     GL_CCW:表示传入的mode会选择逆时针为前向
     GL_CW:表示顺时针为前向
     */
    glFrontFace(GL_CW);
    //绘制地面意外其他部分
    DrawSongAndDance(yRot);
    glFrontFace(GL_CCW);
    //绘制完毕,恢复矩阵
    modelViewMatrix.PopMatrix();
    
    //开启混合功能
    glEnable(GL_BLEND);
    
    //绑定地面纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    
    /** 颜色混合方程式
     1.目标颜色
     2.源颜色
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /** 纹理调整着色器(将一个基本色乘以一个取自纹理的单元 的纹理)
     1.GLT_SHADER_TEXTURE_MODULATE
     2.模型视图矩阵
     3.颜色
     4.纹理单元(第0层的纹理单元)
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipelint.GetModelViewProjectionMatrix(), vFloorColor, 0);
    
    floorBatch.Draw();
    
    //取消混合
    glDisable(GL_BLEND);
    
    //绘制地面以外的其他部分
    DrawSongAndDance(yRot);
    
    //绘制完,恢复矩阵
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
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    //获取投影矩阵到
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //变换管道中来使用
    transformPipelint.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
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
    glutCreateWindow("Open GL SphereWorld");
    
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
    
    ShutdownRC();
    return 0;
}
