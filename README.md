# Langton's Ant Animator

This is a GUI program I made to create animations of Langton's Ant grids.  It does not create video files directly, but instead creates image files for the animation frames.  An external tool (such as [x264](http://www.videolan.org/developers/x264.html)) is required to combine the frames into a video.

In addition to typical Langton's Ant functionality, this program has two notable features.  First, it can create animation frames with temporal anti-aliasing using supersampling.  This allow for smoother, more natural motion in the resulting animation.  Second, it has a search function to rapidly create many image files from different rules.  Rules can either be searched exhaustively or randomly.

Here's a video I made using the frames from this program:
[https://www.youtube.com/watch?v=w7ESrgpQH4k](https://www.youtube.com/watch?v=w7ESrgpQH4k)

### Build

This program uses Qt 5, so the easiest way to build it is to download and install [the Qt kit for your OS](http://www.qt.io/download-open-source/).  Then load `Langtons_Ant.pro` in Qt Creator and build it.

### License

GNU General Public License, version 3
