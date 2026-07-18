# LibreSprite Core Adaptation

This directory contains a deliberately small, command-line-oriented adaptation
of LibreSprite's GPL image, palette, mask, and cleanup concepts. It is pinned
to LibreSprite commit `f06050a6227470cc6450a912a0748f6db7ae83a7` and excludes
LibreSprite's GUI, application, and document layers.

Source project: https://github.com/LibreSprite/LibreSprite

The implementation here provides the minimum embedded pixel-engine surface
needed by `spratforge`: RGBA image buffers, GPL palette loading, palette
normalization, flood-fill connected-component cleanup, and transparent-edge
cleanup. It is distributed under the GNU General Public License, version 2 or
later, consistent with LibreSprite.