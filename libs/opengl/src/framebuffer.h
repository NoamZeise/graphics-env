#ifndef OGL_RENDER_FRAMEBUFFER_H
#define OGL_RENDER_FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

struct InternalAttachment;

class GlFramebuffer {
 public:
    enum class AttachmentType {
	renderbuffer,
	texture2D,
    };
    struct Attachment {
	enum class Position {
	    color0 = GL_COLOR_ATTACHMENT0,
	    depth = GL_DEPTH_ATTACHMENT,
	    stencil = GL_STENCIL_ATTACHMENT,
	    depthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
	};
	Attachment(Position position, AttachmentType type, int format) {
	    this->position = position;
	    this->type = type;
	    this->format = format;
	}
	Position position;
	AttachmentType type;
	int format;
    };
    
    GlFramebuffer(GLsizei width, GLsizei height, int samples, std::vector<Attachment> attachments);
    ~GlFramebuffer();
    GLuint id();
    GLuint textureId(unsigned int attachmentIndex);
 private:
    GLuint framebuffer;
    InternalAttachment* attachments;
    int attachmentCount;
};


#endif
