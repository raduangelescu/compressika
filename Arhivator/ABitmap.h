

// **********
// class CRaster
//   - Generic class for BMP raster images.
class CBitmapRaster {
private:
	int Width,Height;		// Dimensions

public:
	int BPP;				// Bits Per Pixel.
	char * Raster;			// Bits of the Image.
	RGBQUAD * Palette;		// RGB Palette for the image.
	int BytesPerRow;		// Row Width (in bytes).
	BITMAPINFO * pbmi;		// BITMAPINFO structure
	
	int getWidth(){return Width;}
	int getHeight(){return Height;}
	// Member functions (defined later):
	int LoadBMP (char * szFile);
};
