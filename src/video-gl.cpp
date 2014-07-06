
#include "video-gl.h"

#include "audio.h"

GLWindow::GLWindow() {


//#ifdef Q_OS_WIN32
    //QString core_path = "../libretro-super/dist/win/bsnes_balanced_libretro.dll";
    //QString game_path = "../test_roms/Chrono Trigger (U) [!].smc";
//#endif

//#ifdef Q_OS_LINUX
    //QString core_path = "../libretro-super/dist/unix/bsnes_balanced_libretro.so";
    //QString game_path = "../test_roms/Chrono Trigger (U) [!].smc";
//#endif

    core = new Core();
    //if (!core->loadCore(core_path)) {
       // qDebug() << "Core was not loaded";
        //exit(EXIT_FAILURE);
    //}
    //if (!core->loadGame(game_path)) {
        //qDebug() << "Game was not loaded";
        //exit(EXIT_FAILURE);
    //}

    m_program = 0;
    m_texture = 0;
    m_libcore = "";
    m_game = "";


    gamepad.setCore(core);
    gamepad.setPort(0);
    if (gamepad.connect())
        qDebug() <<  "JoyStick connected";

    id = 0;
    device = RETRO_DEVICE_JOYPAD;
    port = 0;
    is_pressed = true;
    index = 0;

    audio = new Audio();
    Q_CHECK_PTR(audio);
    audio->start();
    core->aio = audio->aio();

    connect(this, SIGNAL(runChanged(bool)), audio, SLOT(runChanged(bool)));
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));

}

GLWindow::~GLWindow() {
    delete core;
    //delete gamepad;
    if (m_program)
        delete m_program;
    if (m_texture)
        delete m_texture;
}

void GLWindow::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        // Connect the beforeRendering signal to our paint function.
        // Since this call is executed on the rendering thread it must be
        // a Qt::DirectConnection

        connect(win, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
        connect(win, SIGNAL(widthChanged(int)), this, SLOT(handleGeometryChanged(int)));
        connect(win, SIGNAL(heightChanged(int)), this, SLOT(handleGeometryChanged(int)));
        connect(win, SIGNAL(sceneGraphInitialized()), this, SLOT(handleSceneGraphInitialized()));


        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);

    }
}

void GLWindow::handleSceneGraphInitialized() {
    refreshItemGeometry();
    // initialize m_texture with an empty 1x1 black image
    QImage emptyImage(1, 1, QImage::Format_RGB32);
    emptyImage.fill(Qt::black);
    m_texture = new QOpenGLTexture(emptyImage);
}

void GLWindow::setWindowVisibility(QString windowVisibility) {

    m_win_visibility = windowVisibility;
    emit windowVisibilityChanged( windowVisibility );

}

void GLWindow::setSystemDirectory(QString systemDirectory) {

    core->setSystemDirectory(systemDirectory);

}

void GLWindow::setCore( QString libcore ) {
    qDebug() << "Core: " << libcore;
    if ( !core->loadCore(libcore.toStdString().c_str() )) {
        qDebug() << "Core was not loaded";
        exit(EXIT_FAILURE);
    }
    emit libcoreChanged(libcore);
}

void GLWindow::setGame( QString game ) {
    qDebug() << "Game: " << game;
    if ( !core->loadGame(game.toStdString().c_str() )) {
        qDebug() << "Core was not loaded";
        exit(EXIT_FAILURE);
    }
    updateAudioFormat();
    emit gameChanged(game);
}


void GLWindow::setRun( bool run ) {
    m_run = run;
    emit runChanged(run);
}

void GLWindow::updateAudioFormat() {
    QAudioFormat format;
    format.setSampleSize(16);
    format.setSampleRate(core->getSampleRate());
    format.setChannelCount(2);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");
    // TODO test format
    audio->setFormat(format);
}

void GLWindow::keyReleaseEvent(QKeyEvent *event) {

    id = 16;
    device = RETRO_DEVICE_JOYPAD;
    port = 0;
    is_pressed = false;
    index = 0;

    switch(event->key()) {
        case Qt::Key_Return:
            id = RETRO_DEVICE_ID_JOYPAD_START;
            break;
        case Qt::Key_Space:
            break;
        case Qt::Key_Left:
            id = RETRO_DEVICE_ID_JOYPAD_LEFT;
            break;
        case Qt::Key_Right:
            id = RETRO_DEVICE_ID_JOYPAD_RIGHT;
            break;
        case Qt::Key_Down:
            id = RETRO_DEVICE_ID_JOYPAD_DOWN;
            break;
        case Qt::Key_Up:
            id = RETRO_DEVICE_ID_JOYPAD_UP ;
            break;
        case Qt::Key_A:
            id = RETRO_DEVICE_ID_JOYPAD_A;
            break;
        case Qt::Key_S:
            id = RETRO_DEVICE_ID_JOYPAD_B;
            break;
        case Qt::Key_W:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_X:
            id = RETRO_DEVICE_ID_JOYPAD_X;
            break;
        case Qt::Key_Z:
            id = RETRO_DEVICE_ID_JOYPAD_Y;
            break;
        default:
            break;
    }

    core->setInputStateCallBack(is_pressed, port, device, index, id);

}

void GLWindow::keyPressEvent(QKeyEvent *event) {

    id = 16;
    device = RETRO_DEVICE_JOYPAD;
    port = 0;
    is_pressed = true;
    index = 0;

    switch(event->key()) {
        case Qt::Key_Escape:
            emit windowVisibilityChanged("Windowed");
            break;
        case Qt::Key_Space:
            if (m_run)
                setRun(false);
            else
                setRun(true);
            break;
        case Qt::Key_Return:
            id = RETRO_DEVICE_ID_JOYPAD_START;
            break;
        case Qt::Key_Left:
            id = RETRO_DEVICE_ID_JOYPAD_LEFT;
            break;
        case Qt::Key_Right:
            id = RETRO_DEVICE_ID_JOYPAD_RIGHT;
            break;
        case Qt::Key_Down:
            id = RETRO_DEVICE_ID_JOYPAD_DOWN;
            break;
        case Qt::Key_Up:
            id = RETRO_DEVICE_ID_JOYPAD_UP ;
            break;
        case Qt::Key_A:
            id = RETRO_DEVICE_ID_JOYPAD_A;
            break;
        case Qt::Key_S:
            id = RETRO_DEVICE_ID_JOYPAD_B;
            break;
        case Qt::Key_W:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_X:
            id = RETRO_DEVICE_ID_JOYPAD_X;
            break;
        case Qt::Key_Z:
            id = RETRO_DEVICE_ID_JOYPAD_Y;
            break;
        default:
            qDebug() << "Key not handled";
            break;
    }

    core->setInputStateCallBack(is_pressed, port, device, index, id);

}

//int16_t Core::inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id ) {

void GLWindow::refreshItemGeometry() {
    qreal pixel_ratio = window()->devicePixelRatio();
    item_w = int(pixel_ratio * width());
    item_h = int(pixel_ratio * height());
    item_aspect = (qreal)item_w / item_h;

    viewportXY = mapToScene(QPointF(x(), height()+y())).toPoint();
    viewportXY.setY(window()->height() - viewportXY.y());
}

void GLWindow::initGL() {

    qreal desired_aspect = core->getAspectRatio();
    ulong core_w = item_h * desired_aspect;
    ulong core_h = item_w / desired_aspect;
    QRect viewportRect;

    if(fabsf(item_aspect - desired_aspect) < 0.0001f) {
        viewportRect.setRect(viewportXY.x(), viewportXY.y(), core_w, core_h);
    }
    else if(item_aspect > desired_aspect) {
        viewportRect.setRect(viewportXY.x() + ((item_w - core_w) / 2),
                             viewportXY.y(),
                             core_w, item_h);
    }
    else {
        viewportRect.setRect(viewportXY.x(),
                             viewportXY.y() + ((item_h - core_h) / 2),
                             item_w, core_h );
    }

    glViewport(viewportRect.x(), viewportRect.y(),
               viewportRect.width(), viewportRect.height());



    glDisable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

}

void GLWindow::initShader() {
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                       "attribute highp vec4 vertices;"
                                       "attribute mediump vec4 texCoord;"
                                       "varying mediump vec4 texc;"
                                       "void main() {"
                                       "    gl_Position = vertices;"
                                       "    texc = texCoord;"
                                       "}");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "uniform sampler2D texture;"
                                       "varying mediump vec4 texc;"
                                       "void main() {"
                                       "    gl_FragColor = texture2D(texture, texc.st);"
                                       "}");

    // Binds location 0 to variable vertices
    m_program->bindAttributeLocation("vertices", 0);
    m_program->bindAttributeLocation("texCoord", 1);


    // Links vertex and frag shader
    m_program->link();

    connect(window()->openglContext(), SIGNAL(aboutToBeDestroyed()),
            this, SLOT(cleanup()), Qt::DirectConnection);

}

void GLWindow::setTexture( QOpenGLTexture::Filter min_scale, QOpenGLTexture::Filter max_scale ) {


    QImage::Format frame_format = retroToQImageFormat(core->getPixelFormat());

    m_texture->destroy();
    m_texture->setData( QImage( ( const uchar * )core->getImageData(),
                        core->getBaseWidth(),
                        core->getBaseHeight(),
                        core->getPitch(),
                        frame_format ).mirrored() );

    m_texture->setMinMagFilters(min_scale, max_scale);

    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);

}

void GLWindow::paint() {
    // Produces 1 frame of data


    if (m_run) {

        core->doFrame();

        // Sets texture from core->getImageData();
        setTexture( QOpenGLTexture::Linear, QOpenGLTexture::LinearMipMapNearest );
    }

    // Sets viewport size, and enables / disables opengl functionality.
    initGL();

    // Binds texture to opengl context
    m_texture->bind();

    if (!m_program) {
        // constructs vertex & frag shaders and links them.
        initShader();
    }


    // Makes this shader program the current shader affecting the window.
    m_program->bind();

    // Allows 0 to be used as variable id
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    // Holds vertice values for triangle strip.
    // GLfloat is a cross platform macro for standard float
    // Triangle strip coords
    GLfloat values[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };

    // Texture coords
    GLfloat texture[] = {
        0, 0,
        1, 0,
        0, 1,
        1, 1,
    };

    // Sets location 0 equal to vertices in values array
    m_program->setAttributeArray(0, QOpenGLTexture::Float32, values, 2);

    m_program->setAttributeArray(1, QOpenGLTexture::Float32, texture, 2);

    // Draws processed triangle stip onto the screen.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->disableAttributeArray(0);
    m_program->disableAttributeArray(1);
    m_program->release();
    m_texture->release();

    // Loop forever;
    gamepad.sync(); // Only here temporarily
    window()->update();
}

void GLWindow::cleanup()
{
    // resets shader program
    if (m_program) {
        delete m_program;
        m_program = 0;
    }


}
