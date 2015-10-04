#include "ReymentaHapPlayerApp.h"

void ReymentaHapPlayerApp::setup()
{
	g_Width = 640;
	g_Height = 480;
	// parameters
	mParameterBag = ParameterBag::create();
	// utils
	mBatchass = Batchass::create(mParameterBag);
	// if AutoLayout, try to position the window on the 2nd screen
	if (mParameterBag->mAutoLayout)
	{
		mBatchass->getWindowsResolution();
	}

	setWindowSize(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	setWindowPos(ivec2(mParameterBag->mRenderX, mParameterBag->mRenderY));

	setFullScreen(false);
	//enableHighDensityDisplay();	
	setFrameRate(60);
	mBatchass->setup();
	mLoopVideo = false;
	// -------- SPOUT -------------
	// Set up the texture we will use to send out
	// We grab the screen so it has to be the same size
	bInitialized = false;

}
void ReymentaHapPlayerApp::keyDown(KeyEvent event)
{
	fs::path moviePath;
	switch (event.getCode())
	{
	case ci::app::KeyEvent::KEY_o:
		moviePath = getOpenFilePath();
		if (!moviePath.empty())
			loadMovieFile(moviePath);
		break;
	case ci::app::KeyEvent::KEY_r:
		mMovie.reset();
		break;
	case ci::app::KeyEvent::KEY_p:
		if (mMovie) mMovie->play();
		break;
	case ci::app::KeyEvent::KEY_s:
		if (mMovie) mMovie->stop();
		break;
	case ci::app::KeyEvent::KEY_SPACE:
		if (mMovie->isPlaying()) mMovie->stop(); else mMovie->play();
		break;
	case ci::app::KeyEvent::KEY_ESCAPE:
		shutdown();
		break;
	case ci::app::KeyEvent::KEY_l:
		mLoopVideo = !mLoopVideo;
		if (mMovie) mMovie->setLoop(mLoopVideo);
		break;

	default:
		break;
	}
}
void ReymentaHapPlayerApp::loadMovieFile(const fs::path &moviePath)
{
	try {
		mMovie.reset();
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGlHap::create(moviePath);
		mLoopVideo = (mMovie->getDuration() < 30.0f);
		mMovie->setLoop(mLoopVideo);
		mMovie->play();
		g_Width = mMovie->getWidth();
		g_Height = mMovie->getHeight();
		if (!bInitialized) {
			strcpy_s(SenderName, "Reymenta Hap Spout Sender"); // we have to set a sender name first
			// Optionally test for texture share compatibility
			// bMemoryMode informs us whether Spout initialized for texture share or memory share
			//bMemoryMode = spoutsender.GetMemoryShareMode();
		}
		spoutTexture = gl::Texture::create(g_Width, g_Height);
		setWindowSize(g_Width, g_Height);
			// Initialize a sender
			bInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);

	}
	catch (ci::Exception &e)
	{
		console() <<  string(e.what()) << std::endl;
		console() << "Unable to load the movie." << std::endl;
		mInfoTexture.reset();
	}

}

void ReymentaHapPlayerApp::fileDrop(FileDropEvent event)
{
	loadMovieFile(event.getFile(0));
}
void ReymentaHapPlayerApp::update()
{
	getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Reymenta Hap" + toString(mLoopVideo));
}

void ReymentaHapPlayerApp::draw()
{
	gl::clear(Color::black());
	
	gl::enableAlphaBlending();
	//gl::viewport(toPixels(getWindowSize()));

	if (mMovie) {
		if (mMovie->isPlaying()) mMovie->draw();
	}
	if (bInitialized)
	{
		// Grab the screen (current read buffer) into the local spout texture
		spoutTexture->bind();
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		spoutTexture->unbind();

		// Send the texture for all receivers to use
		// NOTE : if SendTexture is called with a framebuffer object bound, that binding will be lost
		// and has to be restored afterwards because Spout uses an fbo for intermediate rendering
		spoutsender.SendTexture(spoutTexture->getId(), spoutTexture->getTarget(), g_Width, g_Height);
	}
}
void ReymentaHapPlayerApp::mouseDown(MouseEvent event)
{

}
// -------- SPOUT -------------
void ReymentaHapPlayerApp::shutdown()
{
	spoutsender.ReleaseSender();
	quit();
}
// This line tells Cinder to actually create the application
CINDER_APP(ReymentaHapPlayerApp, RendererGl)
