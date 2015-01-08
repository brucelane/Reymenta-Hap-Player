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
	case ci::app::KeyEvent::KEY_ESCAPE:
		shutdown();
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
		mMovie->setLoop(false);
		mMovie->play();

		// create a texture for showing some info about the movie
		TextLayout infoText;
		infoText.clear(ColorA(0.2f, 0.2f, 0.2f, 0.5f));
		infoText.setColor(Color::white());
		infoText.addCenteredLine(moviePath.filename().string());
		infoText.addLine(toString(mMovie->getWidth()) + " x " + toString(mMovie->getHeight()) + " pixels");
		infoText.addLine(toString(mMovie->getDuration()) + " seconds");
		infoText.addLine(toString(mMovie->getNumFrames()) + " frames");
		infoText.addLine(toString(mMovie->getFramerate()) + " fps");
		infoText.setBorder(4, 2);
		mInfoTexture = gl::Texture::create(infoText.render(true));
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
	getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Reymenta Hap player");
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
	// draw fps
	/*TextLayout infoFps;
	infoFps.clear(ColorA(0.2f, 0.2f, 0.2f, 0.5f));
	infoFps.setColor(Color::white());
	infoFps.addLine("Movie Framerate: " + tostr(mMovie->getPlaybackFramerate(), 1));
	infoFps.addLine("App Framerate: " + tostr(this->getAverageFps(), 1));
	infoFps.setBorder(4, 2);
	gl::draw(gl::Texture::create(infoFps.render(true)), ivec2(20, 20));*/

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
