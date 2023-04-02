// Copyright 2017-2023, Nicholas Sharp and the Polyscope contributors. https://polyscope.run

#include "polyscope/screenshot.h"

#include "polyscope/polyscope.h"

#include "stb_image_write.h"

#include <algorithm>
#include <string>

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#undef max
#undef min

#include <Commdlg.h>
#endif

namespace polyscope {

namespace state {

// Storage for the screenshot index
size_t screenshotInd = 0;

} // namespace state

// Helper functions
namespace {

bool hasExtension(std::string str, std::string ext) {

  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (str.length() >= ext.length()) {
    return (0 == str.compare(str.length() - ext.length(), ext.length(), ext));
  } else {
    return false;
  }
}

} // namespace


void saveImage(std::string name, unsigned char* buffer, int w, int h, int channels) {

  // our buffers are from openGL, so they are flipped
  stbi_flip_vertically_on_write(1);
  stbi_write_png_compression_level = 0;

  // Auto-detect filename
  if (hasExtension(name, ".png")) {
    stbi_write_png(name.c_str(), w, h, channels, buffer, channels * w);
  } else if (hasExtension(name, ".jpg") || hasExtension(name, "jpeg")) {
    stbi_write_jpg(name.c_str(), w, h, channels, buffer, 100);

    // TGA seems to display different on different machines: our fault or theirs?
    // Both BMP and TGA need alpha channel stripped? bmp doesn't seem to work even with this
    /*
    } else if (hasExtension(name, ".tga")) {
     stbi_write_tga(name.c_str(), w, h, channels, buffer);
    } else if (hasExtension(name, ".bmp")) {
     stbi_write_bmp(name.c_str(), w, h, channels, buffer);
    */

  } else {
    // Fall back on png
    stbi_write_png(name.c_str(), w, h, channels, buffer, channels * w);
  }
}

void screenshot(std::string filename, bool transparentBG) {

  render::engine->useAltDisplayBuffer = true;
  if (transparentBG) render::engine->lightCopy = true; // copy directly in to buffer without blending

  // == Make sure we render first
  processLazyProperties();

  // save the redraw requested bit and restore it below
  bool requestedAlready = redrawRequested();
  requestRedraw();

  draw(false, false);

  if (requestedAlready) {
    requestRedraw();
  }

  // these _should_ always be accurate
  int w = view::bufferWidth;
  int h = view::bufferHeight;
  std::vector<unsigned char> buff = render::engine->displayBufferAlt->readBuffer();

  // Set alpha to 1
  if (!transparentBG) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        int ind = i + j * w;
        buff[4 * ind + 3] = std::numeric_limits<unsigned char>::max();
      }
    }
  }

  // Save to file
  saveImage(filename, &(buff.front()), w, h, 4);

  render::engine->useAltDisplayBuffer = false;
  if (transparentBG) render::engine->lightCopy = false;
}


std::string file_dialog_save()
{
	const int FILE_DIALOG_MAX_BUFFER = 1024;
	char buffer[FILE_DIALOG_MAX_BUFFER];
	buffer[0] = '\0';
	buffer[FILE_DIALOG_MAX_BUFFER - 1] = 'x'; // Initialize last character with a char != '\0'

#ifdef __APPLE__
  // For apple use applescript hack
  // There is currently a bug in Applescript that strips extensions off
  // of chosen existing files in the "choose file name" dialog
  // I'm assuming that will be fixed soon
	FILE * output = popen(
		"osascript -e \""
		"   tell application \\\"System Events\\\"\n"
		"           activate\n"
		"           set existing_file to choose file name\n"
		"   end tell\n"
		"   set existing_file_path to (POSIX path of (existing_file))\n"
		"\" 2>/dev/null | tr -d '\n' ", "r");
	if (output)
	{
		auto ret = fgets(buffer, FILE_DIALOG_MAX_BUFFER, output);
		if (ret == NULL || ferror(output))
		{
			// I/O error
			buffer[0] = '\0';
		}
		if (buffer[FILE_DIALOG_MAX_BUFFER - 1] == '\0')
		{
			// File name too long, buffer has been filled, so we return empty string instead
			buffer[0] = '\0';
}
}
#elif defined _WIN32

  // Use native windows file dialog box
  // (code contributed by Tino Weinkauf)

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;//hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	int pos = 0;
	if (GetSaveFileName(&ofn) == TRUE)
	{
		while (ofn.lpstrFile[pos] != '\0')
		{
			buffer[pos] = (char)ofn.lpstrFile[pos];
			pos++;
		}
		buffer[pos] = 0;
	}

#else
  // For every other machine type use zenity
	FILE * output = popen("/usr/bin/zenity --file-selection --save", "r");
	if (output)
	{
		auto ret = fgets(buffer, FILE_DIALOG_MAX_BUFFER, output);
		if (ret == NULL || ferror(output))
		{
			// I/O error
			buffer[0] = '\0';
		}
		if (buffer[FILE_DIALOG_MAX_BUFFER - 1] == '\0')
		{
			// File name too long, buffer has been filled, so we return empty string instead
			buffer[0] = '\0';
		}
	}

	// Replace last '\n' by '\0'
	if (strlen(buffer) > 0)
	{
		buffer[strlen(buffer) - 1] = '\0';
	}

#endif
	return std::string(buffer);
}

void screenshot(bool transparentBG) {

  //char buff[50];
  //snprintf(buff, 50, "screenshot_%06zu%s", state::screenshotInd, options::screenshotExtension.c_str());
  //std::string defaultName(buff);
  std::string defaultName = file_dialog_save();
  // only pngs can be written with transparency
  if (!hasExtension(options::screenshotExtension, ".png")) {
    transparentBG = false;
  }

  screenshot(defaultName + options::screenshotExtension.c_str(), transparentBG);

  state::screenshotInd++;
}

void resetScreenshotIndex() { state::screenshotInd = 0; }

} // namespace polyscope
