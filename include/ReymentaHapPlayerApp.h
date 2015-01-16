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
// Utils
#include "Batchass.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Reymenta;

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
	// parameters
	ParameterBagRef				mParameterBag;
	// utils
	BatchassRef					mBatchass;
	// -------- SPOUT -------------
	SpoutSender					spoutsender;            // Create a Spout sender object
	bool						bInitialized;           // true if a sender initializes OK
	bool						bMemoryMode;            // tells us if texture share compatible
	unsigned int				g_Width, g_Height;      // size of the texture being sent out
	char						SenderName[256];        // sender name 
	gl::TextureRef				spoutTexture;           // Local Cinder texture used for sharing
	bool						bDoneOnce;				// only try to initialize once
	int							nSenders;
	// ----------------------------
	bool						mLoopVideo;
};
