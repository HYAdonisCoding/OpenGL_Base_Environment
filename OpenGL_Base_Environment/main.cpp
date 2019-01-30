#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"

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
/** 角色帧 照相机角色帧 */
GLFrame             cameraFrame;
/** 角色帧 */
GLFrame             objectFrame;
/** 视景体-投影矩阵通过它来设置 */
GLFrustum            viewFrustum;
/** 几何视图变换管道 */
GLGeometryTransform transformPipelint;

/** 批次类 */
GLBatch             pyramidBatch;
/** 纹理变量，一般使用无符号整型 */
GLuint              textureID;
M3DMatrix44f        shadowMatrix;


#pragma mark - 绘制金字塔
void MakePyramid(GLBatch &pyramidBatch)
{
    //1.pyramidBatch
    /** 通过三角形批次类pyramidBatch 来组件图形
        //参数1:类型
        //参数2:顶点数 6 * 3 = 18
        //参数3:需要使用到纹理,默认为0,可以不写这个参数
     */
    pyramidBatch.Begin(GL_TRIANGLES, 18, 1);
    
    //2.金字塔底部
    /** 底部四边形 = 三角形X + 三角形Y     */
    //----------vBlackLeft------------
    //设置法线坐标-光照
    pyramidBatch.Normal3f(.0f, -1.0f, .0f);
    
    //设置顶点对应的纹理坐标
    /** pyramidBatch.MultiTexCoord2f(GLuint texture, GLclampf s, GLclampf t)
     参数1:纹理的层次
     参数2:S坐标
     参数3:T坐标*/
    pyramidBatch.MultiTexCoord2f(0, .0f, .0f);
    
    //设置顶点坐标
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
    //------vBlackRight------------
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, -1.0f);
    
    //vFrontRight点
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    
    //三角形Y
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, 1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    //塔顶
    M3DVector3f vApex = {0.0f,1.0f,0.0f};
    M3DVector3f vFrontLeft = {-1.0f,-1.0f,1.0f};
    M3DVector3f vFrontRight = {1.0f,-1.0f,1.0f};
    M3DVector3f vBackLeft = {-1.0f,-1.0f,-1.0f};
    M3DVector3f vBackRight = {1.0f,-1.0f,-1.0f};
    
    //目的:为了临时存储法线向量
    M3DVector3f n;
    
    //金字塔的前面
    //三角形:(Apex,vFrontLeft,vFrontRight)
    
    //找法线
    //m3dFindNormal(M3DVector3f result, const M3DVector3f point1, const M3DVector3f point2,const M3DVector3f point3);
    //参数1:结果
    //参数2-4:3个顶点
    m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
    
    //vApex
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    //vFrontLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);
    
    //vFrongRight
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    
    //金字塔的左边
    //三角形:(Apex,vBackLeft,vFrontLeft)
    m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
    
    //vApex
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    //vBackLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    //vFrontLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);
    
    //金字塔右边
    //三角形：（vApex, vFrontRight, vBackRight）
    m3dFindNormal(n, vApex, vFrontRight, vBackRight);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    //金字塔后边
    //三角形：（vApex, vBackRight, vBackLeft）
    m3dFindNormal(n, vApex, vBackRight, vBackLeft);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    //结束批次设置
    pyramidBatch.End();
}


/**
 将TGA文件加载为2D纹理。

 @param szFileName 纹理文件名称
 @param minFilter 缩小时过滤器使用的哪一种
 @param magFilter 放大小时过滤器使用的哪一种
 @param wrapMode 纹理坐标环绕方式
 @return a加载纹理成功否
 */
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    //1.读取纹理的像素
    /** gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat, GLbyte *pData = NULL)
     参数1:纹理文件的名称
     参数2:纹理文件多的宽度地址
     参数3:文件的高度地址
     参数4:文件的组件地址
     参数5:文件格式地址
     返回值:指向图像数据的指针*/
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    
    if (pBits == NULL) {
        printf("读取纹理像素失败!\n");
        return false;
    }
    
    //2.设置纹理参数
    /** 设置s,t的环绕方式 glTexParameteri(GLenum target, GLenum pname, GLint param)
     参数1:纹理纬度
     参数2:为S/T坐标设置环绕方式
     参数3:环绕模式
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    //设置放大\缩小多的过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    //3.精密包装像素格式
    /** glPixelStorei(GLenum pname, GLint param)
     参数1:GL_UNPACK_ALIGNMENT, OpenGL 如何从数据缓存区中解包图像数据
     参数2:设置GL_UNPACK_ALIGNMENT的值*/
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    //4.载入纹理
    /** glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
     参数1:纹理的纬度
     参数2:mip贴图层次
     参数3:加载纹理颜色成分(从读取的像素图获取的)
     参数4:加载纹理的宽
     参数5:加载纹理的高
     参数6:纹理的边框
     参数7:线束数据的类型
     参数8:指向纹理数据的指针
     */
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
    
    
    //使用完释放
    free(pBits);
    
    /** 只有minFilter 等于以下四种模式，才可以生成Mip贴图
        GL_NEAREST_MIPMAP_NEAREST具有非常好的性能，并且闪烁现象非常弱
        GL_LINEAR_MIPMAP_NEAREST常常用于对游戏进行加速，它使用了高质量的线性过滤器
        GL_LINEAR_MIPMAP_LINEAR 和GL_NEAREST_MIPMAP_LINEAR 过滤器在Mip层之间执行了一些额外的插值，以消除他们之间的过滤痕迹。
        GL_LINEAR_MIPMAP_LINEAR 三线性Mip贴图。纹理过滤的黄金准则，具有最高的精度。
     */
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST) {
        //加载MIP, 纹理生成所有的Mip层
        //参数:GL_TEXTURE_1D\GL_TEXTURE_2D\GL_TEXTURE_3D
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    return true;
}
// 此函数在呈现上下文中进行任何必要的初始化。.
// 这是第一次做任何与opengl相关的任务。
void SetupRC()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    shaderManager.InitializeStockShaders();

    glEnable(GL_DEPTH_TEST);
    
    //纹理对象的处理
    //1.分配纹理对象
    /** glGenTextures (GLsizei n, GLuint *textures);
     参数1:纹理对象的个数
     参数2:纹理对象的指针*/
    glGenTextures(1, &textureID);
    
    //2.绑定纹理状态
    /** glBindTexture(GLenum target, GLuint texture)
     参数1:纹理的状态, GL_TEXTURE_1D\GL_TEXTURE_2D\GL_TEXTURE_3D
     参数2:纹理对象*/
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    //3.将TGA文件加载成2D纹理
    /** 开发者自己设计
     参数1:纹理文件名称
     参数2:缩小时过滤器使用哪一种
     参数3:放大时过滤器使用的哪一种
     参数4:纹理坐标环绕方式
     */
    LoadTGATexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    //4.设置渲染图像的顶点--金字塔
    MakePyramid(pyramidBatch);
    
    cameraFrame.MoveForward(-10.0f);
    
}

#pragma mark - 清理...例如删除纹理对象
void ShutDownRC(void)
{
    glDeleteTextures(1, &textureID);
}


// 召唤场景
void RenderScene(void)
{
    //z光照位置
    static GLfloat vLightPos[] = {1.0f,1.0f,0.0f};
//    static GLfloat vLightPos[] = { .5f, .5f, 2.0f};
    
    //光照颜色
    static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    modelViewMatrix.PushMatrix();
    
    //观察者
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    //
    M3DMatrix44f mObjectFrame;
    objectFrame.GetMatrix(mObjectFrame);
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    //点光源着色器
    /** 参数1:GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
     参数2:模型视图矩阵
     参数3:投影矩阵
     参数4:光源位置
     参数5:光源颜色
     参数6:图形颜色(如使用了纹理,则设置为0)*/
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, transformPipelint.GetModelViewMatrix(), transformPipelint.GetProjectionMatrix(), vLightPos , vWhite, 0);
    
    pyramidBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    //上下键位时, 围绕X轴旋转
    if (key == GLUT_KEY_UP) {
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, .0f, .0f);
    }
    
    if (key == GLUT_KEY_DOWN) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, .0f, .0f);
    }
    
    //左右键位时, 围绕Y轴旋转
    if (key == GLUT_KEY_RIGHT) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), .0f, 1.0f, .0f);
    }
    
    if (key == GLUT_KEY_LEFT) {
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), .0f, 1.0f, .0f);
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
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 500.0f);
    
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
    glutCreateWindow("Pyramid 金字塔");
    
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
    
    ShutDownRC();
    
    return 0;
}
