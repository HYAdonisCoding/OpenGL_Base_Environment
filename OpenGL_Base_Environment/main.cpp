#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

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

/** 地面批次类 */
GLBatch             floorBatch;
/** 天花板批次类 */
GLBatch             ceilingBatch;
/** 左侧墙批次类 */
GLBatch             leftWallBatch;
/** 右侧墙 */
GLBatch             rightWallBatch;
/** 深度初始值 -65 */
GLfloat             viewZ = -65.0f;

//纹理标识符号
/** 墙面 */
#define TEXTURE_BRICK       0
/** 底板 */
#define TEXTURE_FLOOR       1
/** 纹理天花板 */
#define TEXTURE_CEILING     2
/** 纹理个数 */
#define TEXTURE_COUNT       3
/** 纹理标记数组 */
GLuint textures[TEXTURE_COUNT];

/** 添加纹理 纹理标记数组 */
GLuint uiTextures[3];

/** TAG文件名字数组 */
const char *szTextureFiles[TEXTURE_COUNT] = {"brick.tga", "floor.tga", "ceiling.tga"};


/**
 菜单栏修改纹理的属性--缩小过滤器

 @param value 菜单排序
 */
void ProcessMenu(int value)
{
    GLint iLoop;
    
    //3个纹理
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        /** 绑定纹理
         1.使用纹理的类型
         2.纹理标记
         */
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        
        /** 配置纹理参数
         glTexParameteri(GLenum target, GLenum pname, GLint param)
         1.使用纹理的类型
         2.纹理参数(属性)
         3.对应参数的值
         */
        GLenum target = GL_TEXTURE_2D;
        GLenum pname = GL_TEXTURE_MIN_FILTER;
        GLint param = GL_NEAREST;
        switch (value) {
            case 0:
                
                break;
            case 1:
                param = GL_LINEAR;
                break;
            case 2:
                param = GL_NEAREST_MIPMAP_NEAREST;
                break;
            case 3:
                param = GL_NEAREST_MIPMAP_LINEAR;
                break;
            case 4:
                param = GL_LINEAR_MIPMAP_NEAREST;
                break;
            case 5:
                param = GL_LINEAR_MIPMAP_LINEAR;
                break;
            case 6:
                //设置各向异性过滤
                GLfloat fLargest;
                //设置g获取各向异性过滤的最大数量
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
                pname = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
                param = fLargest;
                break;
            case 7:
                //各向同性过滤,数量为1.0
                pname = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
                param = 1.0;
                break;
            default:
                break;
        }
        glTexParameteri(target, pname, param);
        
        
    }
    
    glutPostRedisplay();
    
}

// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    GLbyte *pBytes;
    GLint iWidth, iHeight, iComponents, iLoop;
    GLenum eFormat;
    
    shaderManager.InitializeStockShaders();
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    //1.加载纹理
    /** glGenTextures (GLsizei n, GLuint *textures);
     1.纹理数量
     2.纹理对象标识数组
     */
    glGenTextures(TEXTURE_COUNT, textures);
    
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        //2.绑定纹理
        /** glBindTexture(GLenum target, GLuint texture)
         1.使用的纹理类型
         2.纹理标记
         */
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        
        //3.加载tga文件
        /** gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat)
         1.纹理文件的名称
         2.获取纹理文件的宽度
         3.获取纹理文件的高度
         4.获取纹理文件的组件
         5.获取纹理文件的格式
         
         返回值:指向图像数据的指针
         */
        pBytes = gltReadTGABits(szTextureFiles[iLoop], &iWidth, &iHeight, &iComponents, &eFormat);
        
        //放大过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //缩小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //环绕方式s
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //环绕方式t
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        //载入纹理
        /** glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
         1.使用纹理的类型
         2.mip贴图的层次
         3.纹理单元的颜色组成方式(从读取像素图中获得)
         4\5\6\7.宽\高\边框\格式
         8.像素数据的数据类型
         9.指向纹理图像数据的指针
         */
        glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        
        //生成mip图层
        glGenerateMipmap(GL_TEXTURE_2D);
        
        free(pBytes);
        
    }
    
    //建立立体图像
    GLfloat z;
    /** Begin(GLenum primitive, GLuint nVerts GLuint nTextureUnits = 0);
     1.绘图模式
     2.顶点个数
     3.纹理,默认等于0
     */
    floorBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    
    for (z = 60.0f; z >= 0.0f; z -= 10.0f) {
        //指定左下角顶点
        floorBatch.Vertex3f(-10.0f, -10.0f, z);
        //指定顶点对应纹理的坐标
        floorBatch.MultiTexCoord2f(0, 0, 0);
        
        //指定右下角顶点以及纹理坐标
        floorBatch.Vertex3f(10.0f, -10.0f, z);
        floorBatch.MultiTexCoord2f(0, 1, 0);
        
        floorBatch.Vertex3f(-10.0f, -10.0f, z-10.0f);
        //指定顶点w对应的纹理坐标
        floorBatch.MultiTexCoord2f(0, 0, 1.0f);
        
        floorBatch.Vertex3f(10, -10.0f, z-10.0f);
        floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        
    }
    
    floorBatch.End();
    
    ceilingBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        ceilingBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z);
    }
    ceilingBatch.End();
    
    //参考PPT图6-12
    leftWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        leftWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z - 10.0f);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);
    }
    leftWallBatch.End();
    
    //参考PPT图6-13
    rightWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        rightWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z - 10.0f);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);
    }
    rightWallBatch.End();}


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



/**
 关闭渲染环境
 */
void ShutdownRC(void)
{
    //删除纹理
    glDeleteTextures(TEXTURE_COUNT, textures);
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


//void DrawSongAndDance(GLfloat yRot)
//{
//    static GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
//    static GLfloat vLightPos[] = {.0f, 3.0f, .0f, 1.0f};
//
//    //添加光源 光源位置的全局坐标存储在vLightPos变量中，其中包含了光源位置x坐标、y坐标、z坐标和w坐标。我们必须保留w坐标为1.0。因为无法用一个3分量去乘以4*4矩阵
//    M3DVector4f vLightTransformed;
//
//    //相机矩阵
//    M3DMatrix44f mCamera;
//    //从modelViewMatrix 获取矩阵堆栈顶部的值
//    modelViewMatrix.GetMatrix(mCamera);
//    //将相机矩阵mCamera与光源矩阵vLightPos相乘获得vLightTransformed矩阵
//    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
//
//    //结果压栈
//    modelViewMatrix.PushMatrix();
//    //仿射变换, 平移
//    modelViewMatrix.Translatev(vLightPos);
//
//    /** 绘制(平面着色器) 参数
//     1.GLT_SHADER_FLAT
//     2.模型视图投影矩阵
//     3.颜色数组
//     */
//    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipelint.GetModelViewProjectionMatrix(), vWhite);
//    sphereBatch.Draw();
//
//    //恢复矩阵
//    modelViewMatrix.PopMatrix();
//
//    //绘制悬浮球体 绑定纹理
//    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
//
//    //绘制50个蓝色悬浮球体,绘制一个压栈一个,绘制完成出栈一个
//    for (int i = 0; i < NUM_SPHERE; i++) {
//        modelViewMatrix.PushMatrix();
//        modelViewMatrix.MultMatrix(sphere[i]);
//
//        /** 绘制光源,修改着色器管理器 参数
//         1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
//         2.模型视图矩阵
//         3.投影矩阵
//         4.视点坐标系中的光源位置
//         5.基本漫反射颜色
//         6.颜色(使用纹理则不用颜色)
//         */
//        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
//                                     modelViewMatrix.GetMatrix(),
//                                     transformPipelint.GetProjectionMatrix(),
//                                     vLightTransformed,
//                                     vWhite,
//                                     0);
//        sphereBatch.Draw();
//
//        modelViewMatrix.PopMatrix();
//    }
//
//    //绘制旋转甜甜圈
//    //modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
//    modelViewMatrix.Translate(.0f, .2f, -2.5f);
//
//    //保存平移 (公转自转)
//    modelViewMatrix.PushMatrix();
//
//    //顶部矩阵旋转
//    modelViewMatrix.Rotate(yRot, .0f, 1.0f, .0f);
//
//    //绑定纹理
//    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
//    /** 绘制光源,修改着色器管理器 参数
//     1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
//     2.模型视图矩阵
//     3.投影矩阵
//     4.视点坐标系中的光源位置
//     5.基本漫反射d颜色
//     6.颜色(使用纹理则为0)
//     */
//    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipelint.GetProjectionMatrix(), vLightTransformed, vWhite, 0);
//
//    //绘制
//    torusBatch.Draw();
//
//    //恢复矩阵
//    modelViewMatrix.PopMatrix();
//
//    //绘制公转球体
//    modelViewMatrix.Rotate(yRot * -2.0f, .0f, 1.0f, .0f);
//    modelViewMatrix.Translate(.8f, .0f, .0f);
//
//    //绑定纹理
//    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
//
//    //绘制光源
//    /** 绘制光源,修改着色器管理器 参数
//     1.GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
//     2.模型视图矩阵
//     3.投影矩阵
//     4.视点坐标系中的光源位置
//     5.基本漫反射d颜色
//     6.颜色(使用纹理则为0)
//     */
//    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipelint.GetProjectionMatrix(), vLightTransformed, vWhite, 0);
//    sphereBatch.Draw();
//
//
//}


// 召唤场景
void RenderScene(void)
{
    
    glClear(GL_COLOR_BUFFER_BIT);
    //将当前的模型视图矩阵压入矩阵堆栈（单位矩阵）
    /*因为我们先绘制地面，而地面是不需要有任何变换的。所以在开始渲染时保证矩阵状态，
     然后在结束时使用相应的PopMatrix恢复它。这样就不必在每一次渲染时重载单位矩阵了。
     */
    modelViewMatrix.PushMatrix();
    
    modelViewMatrix.Translate(.0f, .0f, viewZ);
    
    /** 纹理替换矩阵着色器
     1.GLT_SHADER_TEXTURE_REPLACE
     2.模型视图k矩阵 & 投影矩阵
     3.纹理层次
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipelint.GetModelViewProjectionMatrix(), 0);
    
    //底板
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_FLOOR]);
    floorBatch.Draw();
    
    //天花板
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_CEILING]);
    ceilingBatch.Draw();
    
    //左右墙壁
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BRICK]);
    leftWallBatch.Draw();
    rightWallBatch.Draw();
    
    //绘制完,恢复矩阵
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    
    if (key == GLUT_KEY_UP) {
        //移动的是深度值
        viewZ += .5f;
    }
    
    if (key == GLUT_KEY_DOWN) {
        viewZ -= .5f;
    }
    
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
    viewFrustum.SetPerspective(80.0f, float(w)/float(h), 1.0f, 120.0f);
    
    //获取投影矩阵到
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //变换管道中来使用
    transformPipelint.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
//    modelViewMatrix.LoadIdentity();
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
    glutCreateWindow("Tunnel 隧道");
    
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
    
    //添加菜单入口
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("GL_NEAREST", 0);
    glutAddMenuEntry("GL_LINEAR", 1);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", 3);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 4);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", 5);
    glutAddMenuEntry("Anisotropic Filter", 6);
    glutAddMenuEntry("Anisotropic Off", 7);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    
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
