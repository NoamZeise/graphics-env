#ifndef GRAPHICS_ENV_RENDER_PASS_H
#define GRAPHICS_ENV_RENDER_PASS_H

enum class AttachmentType {
  // For storing colour data
  Colour,
  // For storing depth data
  Depth,
  // A target for multisampled attachment colour/depth
  // to be downsampled to.
  Resolve,
};

enum class AttachmentUse {
  // A normal attachment
  Attachment,
  // This attachment will be stored in a texture that
  // can be read from in shaders from future render passes
  ShaderRead,
  // This attachment is the actual window
  // that is shown to the user at the end of the frame
  Screen,
};

enum class MsaaSample {
  Count1 = 1,
  Count2 = 2,
  Count4 = 4,
  Count8 = 8,
  Count16 = 16,
  Count32 = 32,
  Count64 = 64,
  CountMax = 0,
};

struct Attachment {
    AttachmentType type;
    AttachmentUse use;
    MsaaSample samples;
};

#endif
