#include "ReymentaHapPlayerApp.h"

// -------- SPOUT -------------
void ReymentaHapPlayerApp::prepareSettings(Settings *settings)
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

	settings->setWindowSize(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX, mParameterBag->mRenderY));

	settings->setFullScreen(false);
	settings->setResizable(false); // keep the screen size constant for a sender
	settings->enableHighDensityDisplay();
}
// ----------------------------

void ReymentaHapPlayerApp::setup()
{
	setFrameRate(60);
	setFpsSampleInterval(0.25);
	mBatchass->setup();
	mLoopVideo = false;
	// -------- SPOUT -------------
	// Set up the texture we will use to send out
	// We grab the screen so it has to be the same size
	bInitialized = false;
	spoutTexture = gl::Texture::create(g_Width, g_Height);
	strcpy_s(SenderName, "Reymenta Hap Spout Sender"); // we have to set a sender name first
	// Optionally test for texture share compatibility
	// bMemoryMode informs us whether Spout initialized for texture share or memory share
	bMemoryMode = spoutsender.GetMemoryShareMode();
	// Initialize a sender
	bInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);
	// Arguments
	for (vector<string>::const_iterator argIt = getArgs().begin(); argIt != getArgs().end(); ++argIt) loadMovieFile(argIt[0]);
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
		mMovie->setLoop(mLoopVideo);
		mMovie->play();
	}
	catch (...) {
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
	gl::viewport(toPixels(getWindowSize()));

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
CINDER_APP_NATIVE(ReymentaHapPlayerApp, RendererGl)
