//
// Created by wong on 2024/7/21.
//

#include "XVideoWidget.hpp"

//自动加双引号
#define GET_STR(args) #args
static inline constexpr int A_VER{3};
static inline constexpr int T_VER{4};

//顶点shader
static inline constexpr auto vString{
GET_STR(
    in vec4 vertexIn; //顶点坐标
    in vec2 textureIn; //材质坐标
    varying vec2 textureOut; //片元和顶点共享坐标
    void main(void){
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)};

static inline constexpr auto tString{
GET_STR(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void){
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;
        rgb = mat3(1.0, 1.0, 1.0,
                   0.0, -0.39465, 2.03211,
                   1.13983, -0.58060, 0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);
    }
)};

XVideoWidget::XVideoWidget(QWidget *parent):QOpenGLWidget(parent){
#if defined(__APPLE__) && defined(__MACH__)
//    QSurfaceFormat format;
//    format.setDepthBufferSize(24);
//    format.setStencilBufferSize(8);
//    format.setVersion(1, 2);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    QSurfaceFormat::setDefaultFormat(format);
#endif
}

XVideoWidget::~XVideoWidget() {
    for (auto & i:m_datas) {
        delete [] i;
        i = nullptr;
    }
    std::fill_n(m_datas,std::size(m_datas), nullptr);
}

//初始化opengl
void XVideoWidget::initializeGL() {
    qDebug() << "begin " << __FUNCTION__ ;
    //初始化opengl
    initializeOpenGLFunctions();

    qDebug() << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "OpenGL GLSL version: " <<  reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    //program加载shader(顶点和片元)脚本

    //片元(像素)
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,tString);
    qDebug() << m_program.log();

    //顶点shader
    qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,vString);
    qDebug() << m_program.log();

    //设置顶点坐标的变量
    m_program.bindAttributeLocation(GET_STR(vertexIn),A_VER);

    //设置材质坐标
    m_program.bindAttributeLocation(GET_STR(textureIn),T_VER);

    //编译shader
    qDebug() << "program.link() = " << m_program.link();
    qDebug() << m_program.log();
    //绑定
    qDebug() << "program.bind() = " << m_program.bind();
    qDebug() << m_program.log();

    //传递顶点和材质坐标
    const auto vertexIn_num {m_program.attributeLocation(GET_STR(vertexIn))};
    qDebug() << "vertexIn_num = " << vertexIn_num;
    //顶点
    glVertexAttribPointer(vertexIn_num, 3, GL_FLOAT, GL_FALSE, 0, ver);
    glEnableVertexAttribArray(vertexIn_num);

    const auto textureIn_num {m_program.attributeLocation(GET_STR(textureIn))};
    qDebug() << "textureIn_num = " << textureIn_num;
    //材质
    glVertexAttribPointer(textureIn_num, 2, GL_FLOAT, GL_FALSE, 0, tex);
    glEnableVertexAttribArray(textureIn_num);

    //从shader获取材质
    m_unis[0] = m_program.uniformLocation(GET_STR(tex_y));
    m_unis[1] = m_program.uniformLocation(GET_STR(tex_u));
    m_unis[2] = m_program.uniformLocation(GET_STR(tex_v));

    //创建材质(YUV的材质)
    glGenTextures(static_cast<GLsizei>(std::size(m_texs)),m_texs);

    /*************************************Y********************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[0]);
    //放大过滤,线性插值,GL_NEAREST(效率高,但马赛克严重)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w,m_h,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*************************************Y********************************************/

    /*******************************************U***************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[1]);
    //放大过滤,线性插值
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w / 2,m_h / 2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*******************************************U***************************************/

    /*****************************************V******************************************/
    glBindTexture(GL_TEXTURE_2D,m_texs[2]);
    //放大过滤,线性插值
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,m_w / 2,m_h / 2,
                 0,GL_RED,GL_UNSIGNED_BYTE, nullptr);
    /*****************************************V******************************************/

    //分配材质内存空间
    m_datas[0] = new uint8_t[m_w * m_h]{};      //Y
    m_datas[1] = new uint8_t[m_w * m_h / 4]{}; //U
    m_datas[2] = new uint8_t[m_w * m_h / 4]{}; //V

    m_file.setFileName(GET_STR(out240x128.yuv));
    if (!m_file.open(QFile::ReadOnly)){
        throw std::runtime_error(GET_STR(out240x128.yuv file open failed!));
    }

    void (XVideoWidget::*f)(){&XVideoWidget::update};
    connect(&timer,&QTimer::timeout,this,f);

    timer.start(40);

    qDebug() << "end " << __FUNCTION__ ;
}

void XVideoWidget::paintGL() {

    qDebug() << "begin: " << __FUNCTION__;

    if (m_file.atEnd()){
        m_file.seek(0);
    }

    m_file.read(reinterpret_cast<char *>(m_datas[0]),m_w * m_h);
    m_file.read(reinterpret_cast<char *>(m_datas[1]),m_w * m_h / 4);
    m_file.read(reinterpret_cast<char *>(m_datas[2]),m_w * m_h / 4);

    /****************************************y****************************************/
    glActiveTexture(GL_TEXTURE0);//激活了0层材质
    glBindTexture(GL_TEXTURE_2D,m_texs[0]); //0层绑定到Y材质
    //修改材质内容(复印内存中内容)
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w,m_h,
                    GL_RED,GL_UNSIGNED_BYTE,m_datas[0]);
    //与shader uni变量关联
    glUniform1i(m_unis[0], 0);
    /****************************************y****************************************/

    /****************************************u****************************************/
    glActiveTexture(GL_TEXTURE0 + 1);//激活了1层材质
    glBindTexture(GL_TEXTURE_2D,m_texs[1]); //1层绑定到U材质
    //修改材质内容(复印内存中内容)
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w / 2,m_h / 2,
                    GL_RED,GL_UNSIGNED_BYTE,m_datas[1]);
    //与shader uni变量关联
    glUniform1i(m_unis[1], 1);
    /****************************************u****************************************/

    /****************************************v****************************************/
    glActiveTexture(GL_TEXTURE0 + 2);//激活了2层材质
    glBindTexture(GL_TEXTURE_2D,m_texs[2]); //2层绑定到V材质
    //修改材质内容(复印内存中内容)
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_w / 2,m_h / 2,
                    GL_RED,GL_UNSIGNED_BYTE,m_datas[2]);
    //与shader uni变量关联
    glUniform1i(m_unis[2], 2);
    /****************************************v****************************************/

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    qDebug() << "end: " << __FUNCTION__;
}

void XVideoWidget::resizeGL(int w, int h) {
    //qDebug() << __FUNCTION__ << " w:" << w << " h:" << h;
}