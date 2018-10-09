libbmpread
==========

libbmpread is a tiny, fast bitmap (.bmp) image file loader, written from
scratch in portable C (see below), with no dependencies.  Its default behavior
is compatible with OpenGL texture functions, making it ideal for use in simple
games.  It handles any valid bit depth (1, 4, 8, 16, 24, or 32), and can even
load the alpha channel from 16- and 32-bit bitmaps.  There is no support for
RLE or other compression schemes yet.

<https://github.com/chazomaticus/libbmpread>

Documentation
-------------

To use, simply copy `bmpread.c` and `bmpread.h` into your project and add them
to the build.

The API exposed by `bmpread.h` is described below.  The same description can be
found in the header itself.

### `bmpread()`

Loads the specified bitmap file from disk and fills out a `bmpread_t` struct
with data about it.

```c
int bmpread(const char * bmp_file,
            unsigned int flags,
            bmpread_t * p_bmp_out);
```

 * `bmp_file`: The filename of the bitmap file to load.

 * `flags`: Any `BMPREAD_*` flags, combined with bitwise OR.  Specify 0 (or
   `BMPREAD_ALPHA` if you want an alpha channel) for standard, OpenGL compliant
   behavior.

 * `p_bmp_out`: Pointer to a `bmpread_t` struct to fill with information.  Its
   contents on input are ignored.  Must be freed with `bmpread_free()` when no
   longer needed.

Returns 0 if there's an error (file doesn't exist or is invalid, i/o error,
etc.), or nonzero if the file loaded ok.

The file must be a Windows 3 (not NT) or higher format bitmap file with any
valid bit depth (1, 4, 8, 16, 24, or 32), and must not be compressed (no RLE).

Default behavior is for `bmpread()` to return `data` in a format directly
usable by OpenGL texture functions, e.g. `glTexImage2D`, format `GL_RGB` (or
`GL_RGBA` if `BMPREAD_ALPHA` is in `flags`), type `GL_UNSIGNED_BYTE`.  This
implies a few oddities:

 * Lines are ordered bottom-first.  To return data starting with the top line
   like you might otherwise expect, pass `BMPREAD_TOP_DOWN` in `flags`.
 * Lines are padded to span a multiple of four bytes.  To return data with no
   padding, pass `BMPREAD_BYTE_ALIGN` in `flags`.
 * Images with a width or height that isn't a power of 2 will fail to load.  To
   allow loading images of any size, pass `BMPREAD_ANY_SIZE` in `flags`.

Note that passing any of these flags may cause the output to be unusable as an
OpenGL texture, which may or may not matter to you.

Most bitmap files can't include an alpha channel, so the default behavior is to
ignore any alpha values present in the file.  Pass `BMPREAD_ALPHA` in `flags`
to capture alpha values from the file; in case of an absent alpha channel,
alpha values are output as 255 (this can be changed by redefining
`BMPREAD_DEFAULT_ALPHA` in `bmpread.c`).  This allows fully loading 16- and
32-bit bitmaps, which *can* include an alpha channel.

### `bmpread_free()`

Frees memory allocated during `bmpread()`.  Call `bmpread_free()` when you are
done using the `bmpread_t` struct (e.g. after you have passed the data on to
OpenGL).

```c
void bmpread_free(bmpread_t * p_bmp);
```

 * `p_bmp`: The pointer you previously passed to `bmpread()`.

### `bmpread_t`

The struct filled by `bmpread()`.  Holds information about the image's pixels.

```c
typedef struct bmpread_t
{
    int width;
    int height;

    unsigned int flags;

    unsigned char * data;

} bmpread_t;
```

 * `width`: Width in pixels.

 * `height`: Height in pixels.

 * `flags`: `BMPREAD_*` flags, combined with bitwise OR, that affect the format
   of `data`.  These are set to the flags passed to `bmpread()`.

 * `data`: A buffer holding the pixel data of the image.

   By default, each pixel spans three bytes: the red, green, and blue color
   components in that order.  However, with `BMPREAD_ALPHA` set in `flags`,
   each pixel spans four bytes: the red, green, blue, and alpha components in
   that order.

   Pixels are ordered left to right sequentially.  By default, the bottom line
   comes first, proceeding upward.  However, with `BMPREAD_TOP_DOWN` set in
   `flags`, the top line comes first, proceeding downward instead.

   Lines by default must span a multiple of four bytes.  If the image width and
   pixel span don't yield a multiple of four (a non-issue for `BMPREAD_ALPHA`
   with four bytes per pixel), the end of each line is padded with up to three
   unused bytes to meet the requirement.  For example, each line of an image
   three pixels wide, loaded without `BMPREAD_ALPHA`, will span 12 bytes (3
   pixels * 3 (RGB) channels per pixel = 9, padded with 3 bytes up to the next
   multiple of 4).  However, this behavior is disabled with
   `BMPREAD_BYTE_ALIGN` set in flags, in which case all lines span exactly
   `width * pixel_span` bytes.

### Flags

Flags for `bmpread()` and `bmpread_t`.  Combine with bitwise OR.

 * `BMPREAD_TOP_DOWN`: Output data as top line first (default is bottom line
   first).

   ```c
   #define BMPREAD_TOP_DOWN 1u
   ```

 * `BMPREAD_BYTE_ALIGN`: Don't pad lines to span a multiple of four bytes
   (default does pad).

   ```c
   #define BMPREAD_BYTE_ALIGN 2u
   ```

 * `BMPREAD_ANY_SIZE`: Allow loading of any size bitmap (default is bitmaps
   must be 2^n x 2^m).

   ```c
   #define BMPREAD_ANY_SIZE 4u
   ```

 * `BMPREAD_ALPHA`: Load and output an alpha channel (default is just color
   channels).

   ```c
   #define BMPREAD_ALPHA 8u
   ```

Example
-------

Here's a code snippet showing how libbmpread might be used to create an OpenGL
texture from a bitmap file on disk:

```c
#include <stdio.h>
#include <GL/gl.h>
#include "bmpread.h"

/* Load the specified bitmap file from disk and copy it into an OpenGL texture.
 * Return the GLuint representing the texture.
 */
GLuint LoadTexture(const char * bitmap_file)
{
    GLuint texture = 0;
    bmpread_t bitmap;

    if(!bmpread(bitmap_file, 0, &bitmap))
    {
        fprintf(stderr, "%s: error loading bitmap file\n", bitmap_file);
        exit(1);
    }

    /* At this point, bitmap.width and .height hold the pixel dimensions of the
     * file, and bitmap.data holds the raw pixel data in RGB triplets.
     */

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, bitmap.width, bitmap.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, bitmap.data);

    bmpread_free(&bitmap);

    return texture;
}

void SomeInitFunction(void)
{
    GLuint tex1 = LoadTexture("texture1.bmp");
    // ...
}
```

See also the `example` directory for a full, compiling example that displays a
bitmap named on the command line on an OpenGL quad using GLUT.  To build and
run:

    cd example
    make
    ./bmpread-example <bmpfile>

Some example bitmap files are included there as well.

Portability/Security Notes
--------------------------

libbmpread is written to be maximally ANSI C (C89/C90) portable, minimizing
undefined and implementation-defined behavior.  It's also written to be able to
be compiled as C++, so it can be used in C++ projects with a minimum of fuss.
It should work in most environments--I believe there are only two assumptions
the code makes that aren't guaranteed by the C standard: 1) `CHAR_BIT == 8`
(note: if this is violated, compilation will fail), and 2) two's complement
integer storage; deviations from these assumptions are extremely rare in the
wild.  I've also made use of `<stdint.h>` and `ptrdiff_t`, which I believe are
technically C99 features, but are common in practice even for non-compliant
compilers.

I've taken every precaution to prevent common bugs that can have security
impact, such as integer overflows that might lead to buffer overruns.  I
believe it's impossible to cause libbmpread to do anything besides properly
load a file or fail with error, even on maliciously crafted files.  I haven't
done a thorough audit, nor am I an expert at writing hardened "C/C++" code, so
take my belief with a grain of salt.

Tests
-----

To run the test suite:

    cd test
    make

The last line will read `All tests passed!` if everything was ok.  Be patient:
I test an unoptimized function over its entire 32-bit numeric range, so it can
take a minute or two to finish.

There are some basic unit tests, but more work is needed to test the behavior
of the public API.  The tests are compiled as both portable C and portable C++
code to cover compilation in each, and each resulting test binary is run to
check its runtime behavior.

Authors
-------

 * [Charles Lindsay](https://github.com/chazomaticus) (maintainer)
 * [xFrednet](https://github.com/xFrednet/) (added support for 16- and 32-bit
   files)


Enjoy!
