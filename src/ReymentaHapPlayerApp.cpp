/*

Basic Spout app for Cinder

Search for "SPOUT" to see what is required
Uses the Spout dll

Based on the RotatingBox CINDER example without much modification
Nothing fancy about this, just the basics.

Search for "SPOUT" to see what is required

==========================================================================
Copyright (C) 2014 Lynn Jarvis.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
==========================================================================

*/

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/Timer.h"

#include "Resources.h"
#include "MovieHap.h"

// spout
#include "spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

template <typename T> string tostr(const T& t, int p) { ostringstream os; os << std::setprecision(p) << std::fixed << t; return os.str(); }

class ReymentaHapPlayerApp : public AppNative {
public:
	void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();
	void mouseDown(MouseEvent event);
	void keyDown(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void shutdown();
	void loadMovieFile(const fs::path &path);

	gl::TextureRef			mInfoTexture;
	qtime::MovieGlHapRef	mMovie;

private:
	// -------- SPOUT -------------
	SpoutSender         spoutsender;                    // Create a Spout sender object
	bool                bInitialized;             // true if a sender initializes OK
	bool                bMemoryMode;                    // tells us if texture share compatible
	unsigned int        g_Width, g_Height;              // size of the texture being sent out
	char                SenderName[256];                // sender name 
	gl::TextureRef      spoutTexture;             // Local Cinder texture used for sharing
	bool bDoneOnce;                                     // only try to initialize once
	int nSenders;
	// ----------------------------
};

// -------- SPOUT -------------
void ReymentaHapPlayerApp::prepareSettings(Settings *settings)
{
	g_Width = 640;
	g_Height = 512;
	settings->setWindowSize(g_Width, g_Height);
	settings->setFullScreen(false);
	settings->setResizable(false); // keep the screen size constant for a sender
	settings->enableHighDensityDisplay();
}
// ----------------------------

void ReymentaHapPlayerApp::setup()
{
	setFrameRate(60);
	setFpsSampleInterval(0.25);
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
	if (event.getChar() == 'f') {
		setFullScreen(!isFullScreen());
	}
	else if (event.getChar() == 'o') {
		fs::path moviePath = getOpenFilePath();
		if (!moviePath.empty())
			loadMovieFile(moviePath);
	}
	else if (event.getChar() == 'r') {
		mMovie.reset();
	}
}
void ReymentaHapPlayerApp::loadMovieFile(const fs::path &moviePath)
{
	try {
		mMovie.reset();
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGlHap::create(moviePath);
		mMovie->setLoop();
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


}

void ReymentaHapPlayerApp::draw()
{
	gl::clear(Color::black());
	gl::enableAlphaBlending();
	gl::viewport(toPixels(getWindowSize()));

	if (mMovie) {
		mMovie->draw();
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
	TextLayout infoFps;
	infoFps.clear(ColorA(0.2f, 0.2f, 0.2f, 0.5f));
	infoFps.setColor(Color::white());
	infoFps.addLine("Movie Framerate: " + tostr(mMovie->getPlaybackFramerate(), 1));
	infoFps.addLine("App Framerate: " + tostr(this->getAverageFps(), 1));
	infoFps.setBorder(4, 2);
	gl::draw(gl::Texture::create(infoFps.render(true)), ivec2(20, 20));

}
void ReymentaHapPlayerApp::mouseDown(MouseEvent event)
{

}
// -------- SPOUT -------------
void ReymentaHapPlayerApp::shutdown()
{
	spoutsender.ReleaseSender();
}
// This line tells Cinder to actually create the application
CINDER_APP_NATIVE(ReymentaHapPlayerApp, RendererGl)
